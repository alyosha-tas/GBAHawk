#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_MMC2 : public Mappers
	{
	public:

		void Reset()
		{
			CHR_Mask = (*Core_CHR_ROM_Length >> 12) - 1;

			PRG_Mask = (*Core_ROM_Length >> 13) - 1;

			PRG_Bank = 0;

			CHR_Bank = 0;
			CHR_Bank_2 = 0;
			CHR_Bank_3 = 0;
			CHR_Bank_4 = 0;

			MMC2_Latch_0 = 0xFD;
			MMC2_Latch_1 = 0xFD;

			Mirror_Mode = 0;

			Remap_ROM();
		}

		void Remap_ROM()
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

			uint32_t temp_bank = 0;

			// last 3 banks are fixed
			Core_ROM[0] = Core_ROM_Base + PRG_Bank * 0x2000;
			Core_ROM[1] = Core_ROM_Base + *Core_CHR_ROM_Length - 0x6000;
			Core_ROM[2] = Core_ROM_Base + *Core_CHR_ROM_Length - 0x4000;
			Core_ROM[3] = Core_ROM_Base + *Core_CHR_ROM_Length - 0x2000;
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			if (addr < 0x2000)
			{

			}
			else if (addr < 0x3000)
			{
				PRG_Bank = value & PRG_Mask & 0xF;
			}
			else if (addr < 0x4000)
			{
				CHR_Bank = value & CHR_Mask & 0x1F;
			}
			else if (addr < 0x5000)
			{
				CHR_Bank_2 = value & CHR_Mask & 0x1F;
			}
			else if (addr < 0x6000)
			{
				CHR_Bank_3 = value & CHR_Mask & 0x1F;
			}
			else if (addr < 0x7000)
			{
				CHR_Bank_4 = value & CHR_Mask & 0x1F;
			}
			else
			{
				Mirror_Mode = value & 1;
			}

			Remap_ROM();
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x1000)
			{
				if (MMC2_Latch_0 == 0xFD)
				{
					addr += CHR_Bank * 0x1000;
				}
				else
				{
					addr += CHR_Bank_2 * 0x1000;
				}

				if ((addr & 0xFFF) == 0xFD8)
				{
					MMC2_Latch_0 = 0xFD;
				}

				if ((addr & 0xFFF) == 0xFE8)
				{
					MMC2_Latch_0 = 0xFE;
				}

				return Core_CHR_ROM[addr];
			}
			else if (addr < 0x2000)
			{
				addr &= 0xFFF;
				
				if (MMC2_Latch_1 == 0xFD)
				{
					addr += CHR_Bank_3 * 0x1000;
				}
				else
				{
					addr += CHR_Bank_4 * 0x1000;
				}

				if ((addr & 0xFF8) == 0xFD8)
				{
					MMC2_Latch_1 = 0xFD;
				}

				if ((addr & 0xFF8) == 0xFE8)
				{
					MMC2_Latch_1 = 0xFE;
				}

				return Core_CHR_ROM[addr];
			}
			else
			{
				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				return Core_CIRAM[block][ofs];
			}
		}
	};
}