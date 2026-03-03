#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_MMC1 : public Mappers
	{
	public:

		const uint32_t PPU_Clock_Timeout = 4; // i don't know if this is right, but anything lower will not boot Bill & Ted

		void Reset()
		{
			Serial_Shift_Count = Serial_Shift_Value = 0;

			CHR_Mask = (*Core_CHR_ROM_Length >> 12) - 1;

			PRG_Mask = (*Core_ROM_Length >> 14) - 1;

			PRG_Bank = 0;

			MMC1_CHR_0 = 0;
			MMC1_CHR_1 = 0;

			MMC1_PPU_Clock = PPU_Clock_Timeout;

			Command = 0xC;

			Mirror_Mode = Command & 3;
			PRG_Slot = ((Command >> 2) & 1) == 1;
			PRG_Mode = ((Command >> 3) & 1) == 1;
			CHR_Mode = ((Command >> 4) & 1) == 1;

			Remap_ROM();
		}

		void SerialWriteRegister(int addr, int value)
		{
			switch (addr)
			{
				case 0: //8000-9FFF
					Command = value;
					
					Mirror_Mode = Command & 3;
					PRG_Slot = ((Command >> 2) & 1) == 1;
					PRG_Mode = ((Command >> 3) & 1) == 1;
					CHR_Mode = ((Command >> 4) & 1) == 1;
					break;
				case 1: //A000-BFFF
					MMC1_CHR_0 = value & 0x1F;
					break;
				case 2: //C000-DFFF
					MMC1_CHR_1 = value & 0x1F;
					break;
				case 3: //E000-FFFF
					PRG_Bank = value & 0xF;
					WRAM_Enable = ((value >> 4) & 1) == 0;
					break;
			}
		}

		void MMC1_Write(int addr, uint8_t value)
		{
			uint8_t data = value & 1;
			uint8_t reset = (value >> 7) & 1;
			if (reset == 1)
			{
				Serial_Shift_Count = Serial_Shift_Value = 0;

				SerialWriteRegister(0, Command | 0xC);
			}
			else
			{
				Serial_Shift_Value >>= 1;
				Serial_Shift_Value |= (data << 4);
				Serial_Shift_Count++;
				if (Serial_Shift_Count == 5)
				{
					SerialWriteRegister(addr >> 13, Serial_Shift_Value);
					Serial_Shift_Count = 0;
					Serial_Shift_Value = 0;
				}
			}

			Remap_ROM();
		}

		void Remap_ROM()
		{
			
			
			if (!CHR_Mode)
			{
				MMC1_CHR_Regs_4K[0] = MMC1_CHR_0 & ~1;
				MMC1_CHR_Regs_4K[1] = (MMC1_CHR_0 & ~1) + 1;
			}
			else
			{
				MMC1_CHR_Regs_4K[0] = MMC1_CHR_0;
				MMC1_CHR_Regs_4K[1] = MMC1_CHR_1;
			}

			if (Mirror_Mode == 0)
			{
				Core_CIRAM[0] = Core_CIRAM_Base;
				Core_CIRAM[1] = Core_CIRAM_Base;
				Core_CIRAM[2] = Core_CIRAM_Base;
				Core_CIRAM[3] = Core_CIRAM_Base;
			}
			else if (Mirror_Mode == 1)
			{
				Core_CIRAM[0] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[1] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[2] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[3] = Core_CIRAM_Base + 0x400;
			}
			else if (Mirror_Mode == 2)
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

			uint32_t temp_bank = 0;

			if (!PRG_Mode)
			{
				//switch 32kb
				temp_bank = (PRG_Bank & ~1) & PRG_Mask;
				Core_ROM[0] = Core_ROM_Base + 0x4000 * temp_bank;
				Core_ROM[1] = Core_ROM_Base + 0x4000 * temp_bank + 0x2000;
				Core_ROM[2] = Core_ROM_Base + 0x4000 * temp_bank + 0x4000;
				Core_ROM[3] = Core_ROM_Base + 0x4000 * temp_bank + 0x6000;
			}
			else
			{
				//switch 16KB at...
				if (!PRG_Slot)
				{
					//...$C000:
					temp_bank = PRG_Bank & PRG_Mask;
					Core_ROM[0] = Core_ROM_Base;
					Core_ROM[1] = Core_ROM_Base + 0x2000;
					Core_ROM[2] = Core_ROM_Base + 0x4000 * temp_bank;
					Core_ROM[3] = Core_ROM_Base + 0x4000 * temp_bank + 0x2000;
				}
				else
				{
					//...$8000:
					temp_bank = PRG_Bank & PRG_Mask;
					Core_ROM[0] = Core_ROM_Base + 0x4000 * temp_bank;
					Core_ROM[1] = Core_ROM_Base + 0x4000 * temp_bank + 0x2000;
					Core_ROM[2] = Core_ROM_Base + 0x4000 * (0xF & PRG_Mask);
					Core_ROM[3] = Core_ROM_Base + 0x4000 * (0xF & PRG_Mask) + 0x2000;
				}
			}
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			// mmc1 ignores subsequent writes that are very close together
			if (MMC1_PPU_Clock >= PPU_Clock_Timeout)
			{
				MMC1_PPU_Clock = 0;
				MMC1_Write(addr, value);
			}
		}

		void ClockPPU()
		{
			if (MMC1_PPU_Clock < PPU_Clock_Timeout)
				MMC1_PPU_Clock++;
		}

		uint32_t Gen_CHR_Address(uint32_t addr)
		{
			uint32_t bank = MMC1_CHR_Regs_4K[addr >> 12];
			addr = ((bank & CHR_Mask) << 12) | (addr & 0x0FFF);
			return addr;
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				addr = Gen_CHR_Address(addr);
				
				if (*Core_CHR_ROM_Length > 0)
				{
					return Core_CHR_ROM[addr];
				}
				else
				{
					return VRAM[addr & 0x1FFF];
				}
			}
			else
			{
				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				return Core_CIRAM[block][ofs];
			}
		}

		void WritePPU(uint32_t addr, uint8_t value)
		{
			if (addr < 0x2000)
			{
				addr = Gen_CHR_Address(addr);
				
				if (*Core_CHR_ROM_Length == 0)
				{
					VRAM[addr & 0x1FFF] = value;
				}
					
			}
			else
			{
				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				Core_CIRAM[block][ofs] = value;
			}
		}
	};
}