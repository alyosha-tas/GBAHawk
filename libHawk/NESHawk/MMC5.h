#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_MMC5 : public Mappers
	{
	public:

		void Reset()
		{
			//set all prg regs to use ROM
			MMC5_Regs_PRG[0] = 0x80;
			MMC5_Regs_PRG[1] = 0x80;
			MMC5_Regs_PRG[2] = 0x80;
			MMC5_Regs_PRG[3] = 0xFF;

			MMC5_PRG_Mode = 3;
			MMC5_CHR_Mode = 0;
			MMC5_ExRAM_Mode = 0;
			MMC5_AB_Mode = 0;

			Multiplicand = 0;
			Multiplier = 0;
			Product_Low = 0;
			Product_High = 0;
			NT_Fill_Tile = 0;
			NT_Fill_Attrib = 0;

			IRQ_Pending = false;
			IRQ_Enable = false;
			IRQ_Audio = false;
			In_Frame = false;

			IRQ_Counter = 0;
			IRQ_Target = 0;

			MMC5_CHR_Reg_High = 0;
			Last_NT_Read = 0;

			PRG_Mask = (*Core_ROM_Length >> 13) - 1;

			if (*Core_CHR_ROM_Length > 0)
				CHR_Mask = (*Core_CHR_ROM_Length >> 10) - 1;
			else
				CHR_Mask = 7; // might need to change

			// Audio
			MMC5_Audio_PCM_Read = false;
			MMC5_Audio_PCM_Enable_IRQ = false;
			MMC5_Audio_PCM_IRQ_Triggered = false;

			MMC5_Audio_PCM_Val = 0;
			MMC5_Audio_PCM_NextVal = 0;

			MMC5_Audio_Frame = 0;



			Remap_ROM();
		}

		void Remap_ROM()
		{
			SyncPRGBanks();
			SyncCHRBanks();
			SyncMultiplier();
		}

		uint32_t PRGGetBank(uint32_t addr)
		{
			int bank_8k = addr >> 13;
			bank_8k = MMC5_PRG_Banks_8K[bank_8k];
			bool ram = (bank_8k & 0x80) == 0;
			if (!ram)
				bank_8k &= PRG_Mask;
			return bank_8k;
		}

		bool PRGIsRAM(uint32_t addr)
		{
			int bank_8k = addr >> 13;
			bank_8k = MMC5_PRG_Banks_8K[bank_8k];
			return (bank_8k & 0x80) == 0;
		}

		uint32_t MapCHR(uint32_t addr)
		{
			uint32_t bank_1k = addr >> 10;
			uint32_t ofs = addr & ((1 << 10) - 1);

			if ((MMC5_ExRAM_Mode == 1) && (*Core_PPU_Phase == 1)) // BG Phase
			{
				uint32_t exram_addr = Last_NT_Read;
				uint32_t bank_4k = Ex_RAM[exram_addr] & 0x3F;

				bank_1k = bank_4k * 4;
				// low 12 bits of address come from PPU
				// next 6 bits of address come from exram table
				// top 2 bits of address come from chr_reg_high
				bank_1k += MMC5_CHR_Reg_High << 8;
				ofs = addr & (4 * 1024 - 1);

				bank_1k &= CHR_Mask;
				addr = (bank_1k << 10) | ofs;
				return addr;
			}

			if (*Core_ppu_OBJ_Size_16)
			{
				bool isPattern = *Core_show_bg_new || *Core_show_obj_new;
				
				if ((*Core_PPU_Phase == 2) && isPattern) // OBJ Phase
				{
					bank_1k = MMC5_A_Banks_1K[bank_1k];
				}
				else if ((*Core_PPU_Phase == 1) && isPattern) // BG Phase
				{
					bank_1k = MMC5_B_Banks_1K[bank_1k];
				}
				else
				{
					bank_1k = MMC5_AB_Mode == 0
						? MMC5_A_Banks_1K[bank_1k]
						: MMC5_B_Banks_1K[bank_1k];
				}
			}
			else
			{
				bank_1k = MMC5_A_Banks_1K[bank_1k];
			}

			bank_1k &= CHR_Mask;
			addr = (bank_1k << 10) | ofs;
			return addr;
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				addr = MapCHR(addr);
				
				if (*Core_CHR_ROM_Length > 0)
				{
					return Core_CHR_ROM[addr];
				}
				else
				{
					return VRAM[addr];
				}
			}

			addr -= 0x2000;
			uint32_t nt_entry = addr & 0x3FF;
			if (nt_entry < 0x3C0)
			{
				//track the last nametable entry read so that subsequent pattern and attribute reads will know which exram address to use
				Last_NT_Read = nt_entry;
			}
			else
			{
				//attribute table
				if (MMC5_ExRAM_Mode == 1)
				{
					//attribute will be in the top 2 bits of the exram byte
					uint32_t exram_addr = Last_NT_Read;
					uint32_t attribute = Ex_RAM[exram_addr] >> 6;
					//calculate tile address by getting x/y from last nametable
					uint32_t tx = Last_NT_Read & 0x1F;
					uint32_t ty = Last_NT_Read / 32;
					//attribute table address is just these coords shifted
					uint32_t atx = tx >> 1;
					uint32_t aty = ty >> 1;
					//figure out how we need to shift the attribute to fake out the ppu
					uint32_t at_shift = ((aty & 1) << 1) + (atx & 1);
					at_shift <<= 1;
					attribute <<= at_shift;
					return (uint8_t)attribute;
				}
			}
			uint32_t nt = (addr >> 10) & 3; // &3 to read from the NT mirrors at 3xxx
			uint32_t offset = addr & ((1 << 10) - 1);
			nt = MMC5_NT_Modes[nt];
			switch (nt)
			{
				case 0: //NES internal NTA
					return Core_CIRAM_Base[offset];
				
				case 1: //NES internal NTB
					return Core_CIRAM_Base[0x400 | offset];
				
				case 2: //use ExRAM as NT
					//TODO - additional r/w security
					if (MMC5_ExRAM_Mode >= 2)
						return 0;
					else
						return Ex_RAM[offset];
				
				case 3: // Fill Mode
					if (offset >= 0x3c0)
						return NT_Fill_Attrib;
					else
						return NT_Fill_Tile;
				
				default: throw exception();
			}
		}

		uint8_t PeekPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				addr = MapCHR(addr);

				if (*Core_CHR_ROM_Length > 0)
				{
					return Core_CHR_ROM[addr];
				}
				else
				{
					return VRAM[addr];
				}			
			}

			addr -= 0x2000;
			uint32_t nt_entry = addr & 0x3FF;
			if (nt_entry < 0x3C0)
			{
				//track the last nametable entry read so that subsequent pattern and attribute reads will know which exram address to use
				//last_nt_read = nt_entry;
			}
			else
			{
				//attribute table
				if (MMC5_ExRAM_Mode == 1)
				{
					//attribute will be in the top 2 bits of the exram byte
					uint32_t exram_addr = Last_NT_Read;
					uint32_t attribute = Ex_RAM[exram_addr] >> 6;
					//calculate tile address by getting x/y from last nametable
					uint32_t tx = Last_NT_Read & 0x1F;
					uint32_t ty = Last_NT_Read / 32;
					//attribute table address is just these coords shifted
					uint32_t atx = tx >> 1;
					uint32_t aty = ty >> 1;
					//figure out how we need to shift the attribute to fake out the ppu
					uint32_t at_shift = ((aty & 1) << 1) + (atx & 1);
					at_shift <<= 1;
					attribute <<= at_shift;
					return (uint8_t)attribute;
				}
			}
			uint32_t nt = (addr >> 10) & 3; // &3 to read from the NT mirrors at 3xxx
			uint32_t offset = addr & ((1 << 10) - 1);
			nt = MMC5_NT_Modes[nt];
			switch (nt)
			{
				case 0: //NES internal NTA
					return Core_CIRAM_Base[offset];
				
				case 1: //NES internal NTB
					return Core_CIRAM_Base[0x400 | offset];
				
				case 2: //use ExRAM as NT
					//TODO - additional r/w security
					if (MMC5_ExRAM_Mode >= 2)
						return 0;
					else
						return Ex_RAM[offset];
				
				case 3: // Fill Mode
					if (offset >= 0x3c0)
						return NT_Fill_Attrib;
					else
						return NT_Fill_Tile;
				
				default: throw exception();
			}
		}

		void WritePPU(uint32_t addr, uint8_t value)
		{
			if (addr < 0x2000)
			{
				if (*Core_CHR_ROM_Length == 0)
					VRAM[MapCHR(addr)] = value;
			}
			else
			{
				addr -= 0x2000;
				uint32_t nt = (addr >> 10) & 3; // &3 to read from the NT mirrors at 3xxx
				uint32_t offset = addr & ((1 << 10) - 1);
				nt = MMC5_NT_Modes[nt];
				switch (nt)
				{
					case 0: //NES internal NTA
						Core_CIRAM_Base[offset] = value;
						break;
					
					case 1: //NES internal NTB
						Core_CIRAM_Base[0x400 | offset] = value;
						break;
					
					case 2: //use ExRAM as NT
						//TODO - additional r/w security
						Ex_RAM[offset] = value;
						break;
					
					case 3: //Fill Mode
						//what to do?
						break;
					
					default: throw exception();
				}
			}
		}

		uint8_t ReadPRG(uint32_t addr)
		{			
			uint8_t ret;
			uint32_t offs = addr & 0x1fff;
			uint32_t bank = PRGGetBank(addr);

			if (PRGIsRAM(addr))
				ret = ReadWRAMActual(bank, offs);
			else
				ret = Core_ROM_Base[bank << 13 | offs];
			if (addr < 0x4000)
				Audio_ReadROMTrigger(ret);
			return ret;
		}

		uint8_t PeekCart(uint32_t addr)
		{
			if (addr >= 0x8000)
				return PeekPRG(addr - 0x8000);
			if (addr >= 0x6000)
				return ReadWRAM(addr - 0x6000);
			return PeekEXP(addr - 0x4000);
		}

		uint8_t PeekPRG(uint32_t addr)
		{
			uint8_t ret;
			uint32_t offs = addr & 0x1fff;
			uint32_t bank = PRGGetBank(addr);

			if (PRGIsRAM(addr))
				ret = ReadWRAMActual(bank, offs);
			else
				ret = Core_ROM_Base[bank << 13 | offs];
			//if (addr < 0x4000)
			//	audio.ReadROMTrigger(ret);
			return ret;
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			uint32_t bank = PRGGetBank(addr);
			if (PRGIsRAM(addr))
				WriteWRAMActual(bank, addr & 0x1fff, value);
		}

		void WriteWRAM(uint32_t addr, uint8_t value)
		{
			WriteWRAMActual(WRAM_Bank, addr & 0x1fff, value);
		}

		void WriteWRAMActual(uint32_t bank, uint32_t offs, uint8_t value)
		{
			if (IsWRAM(bank))
				Core_Cart_RAM[MaskWRAM(bank) << 13 | offs] = value;
		}

		uint8_t ReadWRAM(uint32_t addr)
		{
			return ReadWRAMActual(WRAM_Bank, addr & 0x1fff);
		}

		uint8_t ReadWRAMActual(uint32_t bank, uint32_t offs)
		{
			if (IsWRAM(bank))
			{
				return Core_Cart_RAM[MaskWRAM(bank) << 13 | offs];
			}
			else
			{
				// not entirely accurate and hardware dependent
				if ((offs & 4) == 0)
				{
					return (uint8_t)(*Core_DB & 0x7F);
				}
				else
				{
					return (uint8_t)(*Core_DB | 0x80);
				}
			}
		}

		bool IsWRAM(uint32_t bank)
		{
			switch (*Core_Cart_RAM_Length >> 10)
			{
				case 0:
					return false;
				case 8:
					if (bank >= 4)
						return false;
					else
						return true;
				case 16:
					return true;
				case 32:
					if (bank >= 4)
						return false;
					else
						return true;
				case 64:
				case 128:
					return true;
				default:
					return false;
			}
		}

		uint32_t MaskWRAM(uint32_t bank)
		{
			switch (*Core_Cart_RAM_Length >> 10)
			{
				case 8:
					return 0;
				case 16:
					return bank >> 2;
				case 32:
					return bank & 3;
				case 64:
				case 128:
					return bank;
				default:
					return 0;
			}
		}


		void WriteExp(uint32_t addr, uint8_t value)
		{
			if (addr >= 0x1000 && addr <= 0x1015)
			{
				Audio_WriteExp(addr + 0x4000, value);
				return;
			}
			switch (addr)
			{
				case 0x1100: //$5100:  [.... ..PP]    PRG Mode Select:
					MMC5_PRG_Mode = value & 3;
					SyncPRGBanks();
					break;

				case 0x1101: //$5101:  [.... ..CC]
					MMC5_CHR_Mode = value & 3;
					SyncCHRBanks();
					break;

				case 0x1102: //$5102:  [.... ..AA]    PRG-RAM Protect A
				case 0x1103: //$5103:  [.... ..BB]    PRG-RAM Protect B
					break;

				case 0x1104: //$5104:  [.... ..XX]    ExRAM mode
					MMC5_ExRAM_Mode = value & 3;
					break;

				case 0x1105: //$5105:  [DDCC BBAA] (nametable config)
					MMC5_NT_Modes[0] = (value >> 0) & 3;
					MMC5_NT_Modes[1] = (value >> 2) & 3;
					MMC5_NT_Modes[2] = (value >> 4) & 3;
					MMC5_NT_Modes[3] = (value >> 6) & 3;
					//NES.LogLine("nt_modes set to {0},{1},{2},{3}", nt_modes[0], nt_modes[1], nt_modes[2], nt_modes[3]);
					break;
				case 0x1106: //$5106:  [TTTT TTTT]     Fill Tile
					NT_Fill_Tile = value;
					break;
				case 0x1107: //$5107:  [.... ..AA]     Fill Attribute bits
					NT_Fill_Attrib = (uint8_t)(value & 3);
					// extend out to fill all 4 positions
					NT_Fill_Attrib |= (uint8_t)(NT_Fill_Attrib << 2);
					NT_Fill_Attrib |= (uint8_t)(NT_Fill_Attrib << 4);
					break;


				case 0x1113: //$5113:  [.... .PPP]        (simplified, but technically inaccurate -- see below)
					WRAM_Bank = value & WRAM_Mask;
					break;

					//$5114-5117:  [RPPP PPPP] PRG select
				case 0x1114: case 0x1115: case 0x1116: case 0x1117:
					if (addr == 0x1117) value |= 0x80;
					MMC5_Regs_PRG[addr - 0x1114] = value;
					SyncPRGBanks();
					break;

					//$5120 - $5127 'A' Regs:
				case 0x1120: case 0x1121: case 0x1122: case 0x1123:
				case 0x1124: case 0x1125: case 0x1126: case 0x1127:
					MMC5_AB_Mode = 0;
					MMC5_Regs_A[addr - 0x1120] = value | (MMC5_CHR_Reg_High << 8);
					//NES.LogLine("set bank A {0:x4} to {1:x2}", addr+0x4000, value);
					SyncCHRBanks();
					break;

					//$5128 - $512B 'B' Regs:
				case 0x1128: case 0x1129: case 0x112A: case 0x112B:
					MMC5_AB_Mode = 1;
					MMC5_Regs_B[addr - 0x1128] = value | (MMC5_CHR_Reg_High << 8);
					//NES.LogLine("set bank B {0:x4} to {1:x2}", addr + 0x4000, value);
					SyncCHRBanks();
					break;

				case 0x1130: //$5130  [.... ..HH]  'High' CHR Reg:
					MMC5_CHR_Reg_High = value & 3;
					break;

				case 0x1203: //$5203:  [IIII IIII]    IRQ Target
					IRQ_Target = value;
					break;

				case 0x1204: //$5204:  [E... ....]    IRQ Enable (0=disabled, 1=enabled)
					IRQ_Enable = (value & 0x80) != 0;
					break;

				case 0x1205: //$5205:  multiplicand
					Multiplicand = value;
					SyncMultiplier();
					break;
				case 0x1206: //$5206:  multiplier
					Multiplier = value;
					SyncMultiplier();
					break;
			}

			//TODO - additional r/w timing security
			if (addr >= 0x1C00)
			{
				if (MMC5_ExRAM_Mode != 3)
					Ex_RAM[addr - 0x1C00] = value;
			}
		}

		bool IrqSignal() { return (IRQ_Pending && IRQ_Enable) || IRQ_Audio; }

		void ClockPPU()
		{
			if (*Core_status_cycle != 8)
				return;

			int sl = *Core_status_sl;

			if (!(*Core_show_bg_new || *Core_show_obj_new) || (sl >= 240))
			{
				// whenever rendering is off for any reason (vblank or forced disable
				// the irq counter resets, as well as the inframe flag (easily verifiable from software)
				In_Frame = false;
				IRQ_Counter = 0;
				IRQ_Pending = false;
				return;
			}

			if (!In_Frame)
			{
				In_Frame = true;
				IRQ_Counter = 0;
				IRQ_Pending = false;
			}
			else
			{
				IRQ_Counter++;
				if (IRQ_Counter == (IRQ_Target))
				{
					IRQ_Pending = true;
				}
			}
		}

		void ClockCPU()
		{
			Audio_Clock();
		}

		void SyncMultiplier()
		{
			uint32_t result = (uint32_t)Multiplicand * (uint32_t)Multiplier;
			Product_Low = (uint8_t)(result & 0xFF);
			Product_High = (uint8_t)((result >> 8) & 0xFF);
		}

		void SetBank(uint32_t* target, int offset, int size, int value)
		{
			value &= ~(size - 1);
			for (int i = 0; i < size; i++)
			{
				int index = i + offset;
				target[index] = value;
				value++;
			}
		}

		void SyncPRGBanks()
		{
			switch (MMC5_PRG_Mode)
			{
				case 0:
					SetBank(&MMC5_PRG_Banks_8K[0], 0, 4, MMC5_Regs_PRG[3] & ~3);
					break;
				case 1:
					SetBank(&MMC5_PRG_Banks_8K[0], 0, 2, MMC5_Regs_PRG[1] & ~1);
					SetBank(&MMC5_PRG_Banks_8K[0], 2, 2, MMC5_Regs_PRG[3] & ~1);
					break;
				case 2:
					SetBank(&MMC5_PRG_Banks_8K[0], 0, 2, MMC5_Regs_PRG[1] & ~1);
					SetBank(&MMC5_PRG_Banks_8K[0], 2, 1, MMC5_Regs_PRG[2]);
					SetBank(&MMC5_PRG_Banks_8K[0], 3, 1, MMC5_Regs_PRG[3]);
					break;
				case 3:
					SetBank(&MMC5_PRG_Banks_8K[0], 0, 1, MMC5_Regs_PRG[0]);
					SetBank(&MMC5_PRG_Banks_8K[0], 1, 1, MMC5_Regs_PRG[1]);
					SetBank(&MMC5_PRG_Banks_8K[0], 2, 1, MMC5_Regs_PRG[2]);
					SetBank(&MMC5_PRG_Banks_8K[0], 3, 1, MMC5_Regs_PRG[3]);
					break;
			}
		}

		void SyncCHRBanks()
		{
			//MASTER LOGIC: something like this this might be enough to work, but i'll play with it later
			//bank_1k >> (3 - chr_mode) << chr_mode | bank_1k & ( etc.etc.

			//TODO - do these need to have the last arguments multiplied by 8,4,2 to map to the right banks?
			switch (MMC5_CHR_Mode)
			{
				case 0:
					SetBank(&MMC5_A_Banks_1K[0], 0, 8, MMC5_Regs_A[7] * 8);
					SetBank(&MMC5_B_Banks_1K[0], 0, 8, MMC5_Regs_B[3] * 8);
					break;
				case 1:
					SetBank(&MMC5_A_Banks_1K[0], 0, 4, MMC5_Regs_A[3] * 4);
					SetBank(&MMC5_A_Banks_1K[0], 4, 4, MMC5_Regs_A[7] * 4);
					SetBank(&MMC5_B_Banks_1K[0], 0, 4, MMC5_Regs_B[3] * 4);
					SetBank(&MMC5_B_Banks_1K[0], 4, 4, MMC5_Regs_B[3] * 4);
					break;
				case 2:
					SetBank(&MMC5_A_Banks_1K[0], 0, 2, MMC5_Regs_A[1] * 2);
					SetBank(&MMC5_A_Banks_1K[0], 2, 2, MMC5_Regs_A[3] * 2);
					SetBank(&MMC5_A_Banks_1K[0], 4, 2, MMC5_Regs_A[5] * 2);
					SetBank(&MMC5_A_Banks_1K[0], 6, 2, MMC5_Regs_A[7] * 2);
					SetBank(&MMC5_B_Banks_1K[0], 0, 2, MMC5_Regs_B[1] * 2);
					SetBank(&MMC5_B_Banks_1K[0], 2, 2, MMC5_Regs_B[3] * 2);
					SetBank(&MMC5_B_Banks_1K[0], 4, 2, MMC5_Regs_B[1] * 2);
					SetBank(&MMC5_B_Banks_1K[0], 6, 2, MMC5_Regs_B[3] * 2);
					break;
				case 3:
					SetBank(&MMC5_A_Banks_1K[0], 0, 1, MMC5_Regs_A[0]);
					SetBank(&MMC5_A_Banks_1K[0], 1, 1, MMC5_Regs_A[1]);
					SetBank(&MMC5_A_Banks_1K[0], 2, 1, MMC5_Regs_A[2]);
					SetBank(&MMC5_A_Banks_1K[0], 3, 1, MMC5_Regs_A[3]);
					SetBank(&MMC5_A_Banks_1K[0], 4, 1, MMC5_Regs_A[4]);
					SetBank(&MMC5_A_Banks_1K[0], 5, 1, MMC5_Regs_A[5]);
					SetBank(&MMC5_A_Banks_1K[0], 6, 1, MMC5_Regs_A[6]);
					SetBank(&MMC5_A_Banks_1K[0], 7, 1, MMC5_Regs_A[7]);
					SetBank(&MMC5_B_Banks_1K[0], 0, 1, MMC5_Regs_B[0]);
					SetBank(&MMC5_B_Banks_1K[0], 1, 1, MMC5_Regs_B[1]);
					SetBank(&MMC5_B_Banks_1K[0], 2, 1, MMC5_Regs_B[2]);
					SetBank(&MMC5_B_Banks_1K[0], 3, 1, MMC5_Regs_B[3]);
					SetBank(&MMC5_B_Banks_1K[0], 4, 1, MMC5_Regs_B[0]);
					SetBank(&MMC5_B_Banks_1K[0], 5, 1, MMC5_Regs_B[1]);
					SetBank(&MMC5_B_Banks_1K[0], 6, 1, MMC5_Regs_B[2]);
					SetBank(&MMC5_B_Banks_1K[0], 7, 1, MMC5_Regs_B[3]);
					break;
			}
		}

		uint8_t ReadExp(uint32_t addr)
		{
			uint8_t ret = 0xFF;
			switch (addr)
			{
			case 0x1204: //$5204:  [E... ....]    IRQ Enable (0=disabled, 1=enabled)
				ret = (uint8_t)((IRQ_Pending ? 0x80 : 0) | (In_Frame ? 0x40 : 0));
				IRQ_Pending = false;
				break;

			case 0x1205: //$5205:  low 8 bits of product
				ret = Product_Low;
				break;
			case 0x1206: //$5206:  high 8 bits of product
				ret = Product_High;
				break;

			case 0x1015: // $5015: apu status
				ret = Audio_Read5015();
				break;

			case 0x1010: // $5010: apu PCM
				ret = Audio_Read5010();
				break;
			}

			//TODO - additional r/w timing security
			if (addr >= 0x1C00)
			{
				if (MMC5_ExRAM_Mode < 2)
					ret = 0xFF;
				else ret = Ex_RAM[addr - 0x1C00];
			}

			return ret;
		}

		uint8_t PeekEXP(uint32_t addr)
		{
			uint8_t ret = 0xFF;
			switch (addr)
			{
				case 0x1204: //$5204:  [E... ....]    IRQ Enable (0=disabled, 1=enabled)
					ret = (uint8_t)((IRQ_Pending ? 0x80 : 0) | (In_Frame ? 0x40 : 0));
					//irq_pending = false;
					//SyncIRQ();
					break;

				case 0x1205: //$5205:  low 8 bits of product
					ret = Product_Low;
					break;
				case 0x1206: //$5206:  high 8 bits of product
					ret = Product_High;
					break;

				case 0x1015: // $5015: apu status
					ret = Audio_Read5015();
					break;

				case 0x1010: // $5010: apu PCM
					ret = Audio_Peek5010();
					break;
			}

			//TODO - additional r/w timing security
			if (addr >= 0x1C00)
			{
				if (MMC5_ExRAM_Mode < 2)
					ret = 0xFF;
				else ret = Ex_RAM[addr - 0x1C00];
			}

			return ret;
		}

	#pragma region MMC5_Audio

		uint32_t Len_Lookup[32] =
		{
			10, 254, 20,  2, 40,  4, 80, 6, 160,  8, 60, 10, 14, 12, 26, 14,
			12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
		};

		uint32_t Sequence_Lookup[4][8] =
		{
			{0,0,0,0,0,0,0,1},
			{0,0,0,0,0,0,1,1},
			{0,0,0,0,1,1,1,1},
			{1,1,1,1,1,1,0,0}
		};


		/// <param name="addr">0x5000..0x5015</param>
		void Audio_WriteExp(uint32_t addr, uint8_t val)
		{		
			switch (addr)
			{
				case 0x5000: Pulse_0_Write0(val); break;
				case 0x5002: Pulse_0_Write2(val); break;
				case 0x5003: Pulse_0_Write3(val); break;
				case 0x5004: Pulse_1_Write0(val); break;
				case 0x5006: Pulse_1_Write2(val); break;
				case 0x5007: Pulse_1_Write3(val); break;
				case 0x5010: // pcm mode/irq
					MMC5_Audio_PCM_Read = (val & 1) == 1;
					MMC5_Audio_PCM_Enable_IRQ = (val & 0x80) == 0x80;
					break;
				case 0x5011: // PCM value
					if (!MMC5_Audio_PCM_Read)
						Audio_WritePCM(val);
					break;
				case 0x5015:
					Pulse_0_SetEnable((val & 1) == 1);
					Pulse_1_SetEnable((val & 2) == 2);
					break;
			}
		}

		uint8_t Audio_Read5015()
		{
			uint8_t ret = 0;
			
			if (Pulse_0_ReadLength())
				ret |= 1;
			if (Pulse_1_ReadLength())
				ret |= 2;
			
			return ret;
		}

		uint8_t Audio_Read5010()
		{
			uint8_t ret = 0;
			
			if (MMC5_Audio_PCM_Enable_IRQ && MMC5_Audio_PCM_IRQ_Triggered)
			{
				ret |= 0x80;
			}
			MMC5_Audio_PCM_IRQ_Triggered = false; // ack
			IRQ_Audio = false;
			
			return ret;
		}

		uint8_t Audio_Peek5010()
		{
			uint8_t ret = 0;
			
			if (MMC5_Audio_PCM_Enable_IRQ && MMC5_Audio_PCM_IRQ_Triggered)
			{
				ret |= 0x80;
			}
			
			return ret;
		}

		/// <summary>
		/// call for 8000:bfff reads
		/// </summary>
		void Audio_ReadROMTrigger(uint8_t val)
		{
			if (MMC5_Audio_PCM_Read)
				Audio_WritePCM(val);
		}

		void Audio_WritePCM(uint8_t val)
		{			
			if (val == 0)
			{
				MMC5_Audio_PCM_IRQ_Triggered = true;
			}
			else
			{
				MMC5_Audio_PCM_IRQ_Triggered = false;
				// can't set diff here, because APU cycle clock might be wrong
				MMC5_Audio_PCM_NextVal = val;
			}
			IRQ_Audio = (MMC5_Audio_PCM_Enable_IRQ && MMC5_Audio_PCM_IRQ_Triggered);
		}

		void Audio_Clock()
		{
			Pulse_0_Clock();
			Pulse_1_Clock();
			MMC5_Audio_Frame++;
			if (MMC5_Audio_Frame == MMC5_Audio_Frame_Reload)
			{
				MMC5_Audio_Frame = 0;
				Pulse_0_ClockFrame();
				Pulse_1_ClockFrame();
			}

			// send the output
			Cart_Audio_Output = 20 * MMC5_Audio_PCM_Val;
			Cart_Audio_Output += 370 * MMC5_Pulse_0_output;
			Cart_Audio_Output += 370 * MMC5_Pulse_1_output;
		}

		void Pulse_0_Write0(uint8_t val)
		{
			MMC5_Pulse_0_V = val & 15;
			MMC5_Pulse_0_ConstantVolume = ((val & 0x10) == 0x10);
			MMC5_Pulse_0_LenCntDisable = ((val & 0x20) == 0x20);
			MMC5_Pulse_0_D = val >> 6;
		}
		void Pulse_0_Write2(uint8_t val)
		{
			MMC5_Pulse_0_T &= 0x700;
			MMC5_Pulse_0_T |= val;
		}
		void Pulse_0_Write3(uint8_t val)
		{
			MMC5_Pulse_0_T &= 0xff;
			MMC5_Pulse_0_T |= val << 8 & 0x700;
			MMC5_Pulse_0_L = val >> 3;
			MMC5_Pulse_0_estart = true;
			if (MMC5_Pulse_0_Enable)
				MMC5_Pulse_0_length = Len_Lookup[MMC5_Pulse_0_L];
			MMC5_Pulse_0_sequence = 0;
		}
		void Pulse_0_SetEnable(bool val)
		{
			MMC5_Pulse_0_Enable = val;
			if (!MMC5_Pulse_0_Enable)
				MMC5_Pulse_0_length = 0;
		}
		bool Pulse_0_ReadLength()
		{
			return MMC5_Pulse_0_length > 0;
		}

		void Pulse_0_ClockFrame()
		{
			// envelope
			if (MMC5_Pulse_0_estart)
			{
				MMC5_Pulse_0_estart = false;
				MMC5_Pulse_0_ecount = 15;
				MMC5_Pulse_0_etime = MMC5_Pulse_0_V;
			}
			else
			{
				MMC5_Pulse_0_etime--;
				if (MMC5_Pulse_0_etime < 0)
				{
					MMC5_Pulse_0_etime = MMC5_Pulse_0_V;
					if (MMC5_Pulse_0_ecount > 0)
					{
						MMC5_Pulse_0_ecount--;
					}
					else if (MMC5_Pulse_0_LenCntDisable)
					{
						MMC5_Pulse_0_ecount = 15;
					}
				}
			}
			// length
			if (MMC5_Pulse_0_Enable && !MMC5_Pulse_0_LenCntDisable && MMC5_Pulse_0_length > 0)
			{
				MMC5_Pulse_0_length--;
			}
		}

		void Pulse_0_Clock()
		{
			MMC5_Pulse_0_clock--;
			if (MMC5_Pulse_0_clock < 0)
			{
				MMC5_Pulse_0_clock = MMC5_Pulse_0_T * 2 + 1;
				MMC5_Pulse_0_sequence--;
				if (MMC5_Pulse_0_sequence < 0)
					MMC5_Pulse_0_sequence += 8;

				int sequenceval = Sequence_Lookup[MMC5_Pulse_0_D][MMC5_Pulse_0_sequence];

				int newvol = 0;

				if (sequenceval > 0 && MMC5_Pulse_0_length > 0)
				{
					if (MMC5_Pulse_0_ConstantVolume)
						newvol = MMC5_Pulse_0_V;
					else
						newvol = MMC5_Pulse_0_ecount;
				}
			
				MMC5_Pulse_0_output = newvol;
			}
		}

		void Pulse_1_Write0(uint8_t val)
		{
			MMC5_Pulse_1_V = val & 15;
			MMC5_Pulse_1_ConstantVolume = ((val & 0x10) == 0x10);
			MMC5_Pulse_1_LenCntDisable = ((val & 0x20) == 0x20);
			MMC5_Pulse_1_D = val >> 6;
		}
		void Pulse_1_Write2(uint8_t val)
		{
			MMC5_Pulse_1_T &= 0x700;
			MMC5_Pulse_1_T |= val;
		}
		void Pulse_1_Write3(uint8_t val)
		{
			MMC5_Pulse_1_T &= 0xff;
			MMC5_Pulse_1_T |= val << 8 & 0x700;
			MMC5_Pulse_1_L = val >> 3;
			MMC5_Pulse_1_estart = true;
			if (MMC5_Pulse_1_Enable)
				MMC5_Pulse_1_length = Len_Lookup[MMC5_Pulse_1_L];
			MMC5_Pulse_1_sequence = 0;
		}
		void Pulse_1_SetEnable(bool val)
		{
			MMC5_Pulse_1_Enable = val;
			if (!MMC5_Pulse_1_Enable)
				MMC5_Pulse_1_length = 0;
		}
		bool Pulse_1_ReadLength()
		{
			return MMC5_Pulse_1_length > 0;
		}

		void Pulse_1_ClockFrame()
		{
			// envelope
			if (MMC5_Pulse_1_estart)
			{
				MMC5_Pulse_1_estart = false;
				MMC5_Pulse_1_ecount = 15;
				MMC5_Pulse_1_etime = MMC5_Pulse_1_V;
			}
			else
			{
				MMC5_Pulse_1_etime--;
				if (MMC5_Pulse_1_etime < 0)
				{
					MMC5_Pulse_1_etime = MMC5_Pulse_1_V;
					if (MMC5_Pulse_1_ecount > 0)
					{
						MMC5_Pulse_1_ecount--;
					}
					else if (MMC5_Pulse_1_LenCntDisable)
					{
						MMC5_Pulse_1_ecount = 15;
					}
				}
			}
			// length
			if (MMC5_Pulse_1_Enable && !MMC5_Pulse_1_LenCntDisable && MMC5_Pulse_1_length > 0)
			{
				MMC5_Pulse_1_length--;
			}
		}

		void Pulse_1_Clock()
		{
			MMC5_Pulse_1_clock--;
			if (MMC5_Pulse_1_clock < 0)
			{
				MMC5_Pulse_1_clock = MMC5_Pulse_1_T * 2 + 1;
				MMC5_Pulse_1_sequence--;
				if (MMC5_Pulse_1_sequence < 0)
					MMC5_Pulse_1_sequence += 8;

				int sequenceval = Sequence_Lookup[MMC5_Pulse_1_D][MMC5_Pulse_1_sequence];

				int newvol = 0;

				if (sequenceval > 0 && MMC5_Pulse_1_length > 0)
				{
					if (MMC5_Pulse_1_ConstantVolume)
						newvol = MMC5_Pulse_1_V;
					else
						newvol = MMC5_Pulse_1_ecount;
				}

				MMC5_Pulse_1_output = newvol;
			}
		}

	#pragma endregion

	};
}
/*
namespace BizHawk.Emulation.Cores.Nintendo.NES
{
	internal sealed class ExROM : NesBoardBase
	{

		class Pulse
		{


		}



		private void PulseAddDiff(int value)
		{
			enqueuer(value * 370);
			//Console.WriteLine(value);
		}

		public MMC5Audio(Action<int> enqueuer, Action<bool> RaiseIRQ)
		{
			this.enqueuer = enqueuer;
			this.RaiseIRQ = RaiseIRQ;
			for (int i = 0; i < pulse.Length; i++)
				pulse[i] = new Pulse(PulseAddDiff);
		}


	}
}
*/