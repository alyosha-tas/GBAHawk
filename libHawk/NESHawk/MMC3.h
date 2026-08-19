#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_MMC3 : public Mappers
	{
	public:

		bool IRQ_Pending;
		bool IRQ_Enable;
		bool IRQ_Reload_Flag;
		bool PRG_Mode;
		bool CHR_Mode;
		bool Just_Cleared;
		bool Just_Cleared_Pending;
		bool WRAM_Enable;
		bool WRAM_Write_Protect;

		uint8_t Command;
		uint8_t IRQ_Reload;
		uint8_t IRQ_Counter;

		uint32_t CHR_Bank;
		uint32_t CHR_Mask;
		uint32_t PRG_Bank;
		uint32_t PRG_Mask;
		uint32_t Mirror_Mode;
		uint32_t Reg_Addr;
		uint32_t Separator_Counter;
		uint32_t IRQ_Countdown;
		uint32_t A12_Old;

		uint8_t MMC3_Regs[8] = { };
		uint8_t MMC3_CHR_Regs_1K[8] = { };

		void Reset()
		{
			PRG_Mode = false;
			CHR_Mode = false;
			IRQ_Pending = false;
			IRQ_Enable = false;
			IRQ_Reload_Flag = false;
			Just_Cleared = false;
			Just_Cleared_Pending = false;
			WRAM_Enable = false;
			WRAM_Write_Protect = false;
			Old_IRQ_Type = false;
			Alt_Mirroring = false;

			Command = 0;
			IRQ_Reload = 0;
			IRQ_Counter = 0;

			Reg_Addr = 0;
			Separator_Counter = 0;
			IRQ_Countdown = 0;
			A12_Old = 0;
			
			//initial values seem necessary
			MMC3_Regs[0] = 0;
			MMC3_Regs[1] = 2;
			MMC3_Regs[2] = 4;
			MMC3_Regs[3] = 5;
			MMC3_Regs[4] = 6;
			MMC3_Regs[5] = 7;
			MMC3_Regs[6] = 0;
			MMC3_Regs[7] = 1;

			CHR_Bank = 0;

			CHR_Mask = (*Core_CHR_ROM_Length >> 10) - 1;

			PRG_Mask = (*Core_ROM_Length >> 13) - 1;

			PRG_Bank = 0;

			Mirror_Mode = 0;

			Remap_ROM();
		}

		void Remap_ROM()
		{			
			if (PRG_Mode)
			{
				Core_ROM[0] = Core_ROM_Base + *Core_ROM_Length - 0x4000;
				Core_ROM[1] = Core_ROM_Base + (MMC3_Regs[7] & PRG_Mask) * 0x2000;
				Core_ROM[2] = Core_ROM_Base + (MMC3_Regs[6] & PRG_Mask) * 0x2000;
				Core_ROM[3] = Core_ROM_Base + *Core_ROM_Length - 0x2000;
			}
			else
			{
				Core_ROM[0] = Core_ROM_Base + (MMC3_Regs[6] & PRG_Mask) * 0x2000;
				Core_ROM[1] = Core_ROM_Base + (MMC3_Regs[7] & PRG_Mask) * 0x2000;
				Core_ROM[2] = Core_ROM_Base + *Core_ROM_Length - 0x4000;
				Core_ROM[3] = Core_ROM_Base + *Core_ROM_Length - 0x2000;
			}

			uint8_t r0_0 = (uint8_t)(MMC3_Regs[0] & ~1);
			uint8_t r0_1 = (uint8_t)(MMC3_Regs[0] | 1);
			uint8_t r1_0 = (uint8_t)(MMC3_Regs[1] & ~1);
			uint8_t r1_1 = (uint8_t)(MMC3_Regs[1] | 1);

			if (CHR_Mode)
			{
				MMC3_CHR_Regs_1K[0] = MMC3_Regs[2];
				MMC3_CHR_Regs_1K[1] = MMC3_Regs[3];
				MMC3_CHR_Regs_1K[2] = MMC3_Regs[4];
				MMC3_CHR_Regs_1K[3] = MMC3_Regs[5];
				MMC3_CHR_Regs_1K[4] = r0_0;
				MMC3_CHR_Regs_1K[5] = r0_1;
				MMC3_CHR_Regs_1K[6] = r1_0;
				MMC3_CHR_Regs_1K[7] = r1_1;
			}
			else
			{
				MMC3_CHR_Regs_1K[0] = r0_0;
				MMC3_CHR_Regs_1K[1] = r0_1;
				MMC3_CHR_Regs_1K[2] = r1_0;
				MMC3_CHR_Regs_1K[3] = r1_1;
				MMC3_CHR_Regs_1K[4] = MMC3_Regs[2];
				MMC3_CHR_Regs_1K[5] = MMC3_Regs[3];
				MMC3_CHR_Regs_1K[6] = MMC3_Regs[4];
				MMC3_CHR_Regs_1K[7] = MMC3_Regs[5];
			}

			if (!Alt_Mirroring)
			{
				if (Mirror_Mode == 0)
				{
					Core_CIRAM[0] = Core_CIRAM_Base;
					Core_CIRAM[1] = Core_CIRAM_Base + 0x400;
					Core_CIRAM[2] = Core_CIRAM_Base;
					Core_CIRAM[3] = Core_CIRAM_Base + 0x400;
				}
				else
				{
					Core_CIRAM[0] = Core_CIRAM_Base;
					Core_CIRAM[1] = Core_CIRAM_Base;
					Core_CIRAM[2] = Core_CIRAM_Base + 0x400;
					Core_CIRAM[3] = Core_CIRAM_Base + 0x400;
				}
			}
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			switch (addr & 0x6001)
			{
				case 0x0000: //$8000
					Command = value;
					CHR_Mode = (value & 0x80) == 0x80;
					PRG_Mode = (value & 0x40) == 0x40;
					Reg_Addr = (value & 7);
					break;
				case 0x0001: //$8001
					MMC3_Regs[Reg_Addr] = value;
					break;
				case 0x2000: //$A000
					//mirroring
					Mirror_Mode = (value & 1);
					break;
				case 0x2001: //$A001
					//wram enable/protect
					WRAM_Write_Protect = (value & 0x40) == 0x40;
					WRAM_Enable = (value & 0x80) == 0x80;
					break;
				case 0x4000: //$C000 - IRQ Reload value
					IRQ_Reload = value;
					break;
				case 0x4001: //$C001 - IRQ Clear
					// does not take immediate effect (fixes Klax)
					Just_Cleared_Pending = true;
					break;
				case 0x6000: //$E000 - IRQ Acknowledge / Disable
					IRQ_Enable = false;
					IRQ_Pending = false;
					break;
				case 0x6001: //$E001 - IRQ Enable
					IRQ_Enable = true;
					break;
			}

			Remap_ROM();
		}

		uint8_t ReadWRAM(uint32_t addr)
		{
			if ((*Core_Cart_RAM_Length > 0) && WRAM_Enable)
			{
				return Core_Cart_RAM[addr];
			}

			// not entirely accurate and hardware dependent
			// works for Battletoads and Castlevania III
			// also effect Low G Man, preventing cobsole verification

			if ((addr & 4) == 0)
			{
				return (*Core_DB & 0x7F);
			}
			else
			{
				return (*Core_DB | 0x80);
			}

			//return *Core_DB;
		}

		void WriteWRAM(uint32_t addr, uint8_t value)
		{
			if ((*Core_Cart_RAM_Length > 0) && WRAM_Enable && !WRAM_Write_Protect)
			{
				Core_Cart_RAM[addr] = value;
			}
		}

		void IRQ_EQ_Pass()
		{
			if (IRQ_Enable)
			{
				IRQ_Pending = true;
			}
		}

		void ClockIRQ()
		{
			uint8_t last_irq_counter = IRQ_Counter;
			if (IRQ_Reload_Flag || IRQ_Counter == 0)
			{
				IRQ_Counter = IRQ_Reload;
			}
			else
			{
				IRQ_Counter--;
			}
			if (IRQ_Counter == 0)
			{
				if (Old_IRQ_Type)
				{				
					if (last_irq_counter != 0 || IRQ_Reload_Flag)
						IRQ_EQ_Pass();
				}
				else
					IRQ_EQ_Pass();
			}

			IRQ_Reload_Flag = false;
		}

		void ClockPPU()
		{
			if (Separator_Counter > 0)
				Separator_Counter--;

			if (IRQ_Countdown > 0)
			{				
				IRQ_Countdown--;
				if (IRQ_Countdown == 0)
				{
					ClockIRQ();
				}
			}

			if (Just_Cleared)
			{
				IRQ_Counter = 0;
				if (Old_IRQ_Type)
					IRQ_Reload_Flag = true;
			}

			Just_Cleared = Just_Cleared_Pending;
			Just_Cleared_Pending = false;
		}

		void AddressPPU(uint32_t addr)
		{
			uint32_t a12 = (addr >> 12) & 1;
			bool rising_edge = (a12 == 1 && A12_Old == 0);
			if (rising_edge)
			{
				if (Separator_Counter > 0)
				{
					Separator_Counter = 15;
				}
				else
				{
					Separator_Counter = 15;
					IRQ_Countdown = 5;
				}
			}

			A12_Old = a12;
		}

		uint32_t Get_CHRBank_1K(uint32_t addr)
		{
			uint32_t bank_1k = addr >> 10;
			bank_1k = MMC3_CHR_Regs_1K[bank_1k];
			return bank_1k;
		}

		uint32_t MapCHR(uint32_t addr)
		{
			uint32_t bank_1k = Get_CHRBank_1K(addr);
			// allow NPOT chr sizes
			bank_1k %= CHR_Mask + 1;
			addr = (bank_1k << 10) | (addr & 0x3FF);
			return addr;
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				if (*Core_CHR_ROM_Length > 0)
				{
					addr = MapCHR(addr);
					
					return Core_CHR_ROM[addr];
				}
				else
				{
					return VRAM[addr];
				}
			}

			uint32_t ofs = addr & 0x3FF;
			uint32_t block = (addr >> 10) & 3;

			return Core_CIRAM[block][ofs];
		}

		bool IrqSignal() { return IRQ_Pending; }

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(IRQ_Pending, saver);
			saver = bool_saver(IRQ_Enable, saver);
			saver = bool_saver(IRQ_Reload_Flag, saver);
			saver = bool_saver(PRG_Mode, saver);
			saver = bool_saver(CHR_Mode, saver);
			saver = bool_saver(Just_Cleared, saver);
			saver = bool_saver(Just_Cleared_Pending, saver);
			saver = bool_saver(WRAM_Enable, saver);
			saver = bool_saver(WRAM_Write_Protect, saver);

			saver = byte_saver(Command, saver);
			saver = byte_saver(IRQ_Reload, saver);
			saver = byte_saver(IRQ_Counter, saver);

			saver = int_saver(CHR_Bank, saver);
			saver = int_saver(CHR_Mask, saver);
			saver = int_saver(PRG_Bank, saver);
			saver = int_saver(PRG_Mask, saver);
			saver = int_saver(Mirror_Mode, saver);
			saver = int_saver(Reg_Addr, saver);
			saver = int_saver(Separator_Counter, saver);
			saver = int_saver(IRQ_Countdown, saver);
			saver = int_saver(A12_Old, saver);

			saver = byte_array_saver(MMC3_Regs, saver, 8);
			saver = byte_array_saver(MMC3_CHR_Regs_1K, saver, 8);

			// common to all mappers
			saver = bool_saver(Mirroring, saver);
			saver = bool_saver(Bus_Conflicts, saver);
			saver = bool_saver(Old_IRQ_Type, saver);
			saver = bool_saver(Alt_Mirroring, saver);

			saver = int_saver(Size_Mask, saver);

			saver = byte_array_saver(Ex_RAM, saver, 0x400);
			saver = byte_array_saver(VRAM, saver, 0x8000);
			saver = byte_array_saver(EXT_CIRAM, saver, 0x2000);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&IRQ_Pending, loader);
			loader = bool_loader(&IRQ_Enable, loader);
			loader = bool_loader(&IRQ_Reload_Flag, loader);
			loader = bool_loader(&PRG_Mode, loader);
			loader = bool_loader(&CHR_Mode, loader);
			loader = bool_loader(&Just_Cleared, loader);
			loader = bool_loader(&Just_Cleared_Pending, loader);
			loader = bool_loader(&WRAM_Enable, loader);
			loader = bool_loader(&WRAM_Write_Protect, loader);

			loader = byte_loader(&Command, loader);
			loader = byte_loader(&IRQ_Reload, loader);
			loader = byte_loader(&IRQ_Counter, loader);

			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Mask, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Mask, loader);
			loader = int_loader(&Mirror_Mode, loader);
			loader = int_loader(&Reg_Addr, loader);
			loader = int_loader(&Separator_Counter, loader);
			loader = int_loader(&IRQ_Countdown, loader);
			loader = int_loader(&A12_Old, loader);

			loader = byte_array_loader(MMC3_Regs, loader, 8);
			loader = byte_array_loader(MMC3_CHR_Regs_1K, loader, 8);

			// common to all mappers
			loader = bool_loader(&Mirroring, loader);
			loader = bool_loader(&Bus_Conflicts, loader);
			loader = bool_loader(&Old_IRQ_Type, loader);
			loader = bool_loader(&Alt_Mirroring, loader);

			loader = int_loader(&Size_Mask, loader);

			loader = byte_array_loader(Ex_RAM, loader, 0x400);
			loader = byte_array_loader(VRAM, loader, 0x8000);
			loader = byte_array_loader(EXT_CIRAM, loader, 0x2000);

			Remap_ROM();

			return loader;
		}
	};
}