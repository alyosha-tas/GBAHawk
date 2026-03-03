#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_AxROM : public Mappers
	{
	public:

		void Reset()
		{
			CHR_Bank = 0;

			CHR_Mask = (*Core_CHR_ROM_Length >> 13) - 1;

			PRG_Mask = (*Core_ROM_Length >> 15) - 1;

			PRG_Bank = 0;

			Mirror_Mode = 0;

			Remap_ROM();
		}

		void Remap_ROM()
		{
			Core_ROM[0] = Core_ROM_Base + 0x8000 * PRG_Bank;
			Core_ROM[1] = Core_ROM_Base + 0x8000 * PRG_Bank + 0x2000;
			Core_ROM[2] = Core_ROM_Base + 0x8000 * PRG_Bank + 0x4000;
			Core_ROM[3] = Core_ROM_Base + 0x8000 * PRG_Bank + 0x6000;

			if (Mirror_Mode == 0)
			{
				Core_CIRAM[0] = Core_CIRAM_Base;
				Core_CIRAM[1] = Core_CIRAM_Base;
				Core_CIRAM[2] = Core_CIRAM_Base;
				Core_CIRAM[3] = Core_CIRAM_Base;
			}
			else
			{
				Core_CIRAM[0] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[1] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[2] = Core_CIRAM_Base + 0x400;
				Core_CIRAM[3] = Core_CIRAM_Base + 0x400;
			}
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			if (Bus_Conflicts)
			{
				value &= ReadPRG(addr);
			}

			PRG_Bank = value & 7 & PRG_Mask;

			Mirror_Mode = (value >> 4) & 1;

			Remap_ROM();
		}
	};
}