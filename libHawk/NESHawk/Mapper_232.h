#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_232 : public Mappers
	{
	public:

		void Reset()
		{
			PRG_Mask = (*Core_ROM_Length >> 14) - 1;

			PRG_Bank = 0;

			PRG_Block = 0;

			Remap_ROM();
		}

		void Remap_ROM()
		{
			Core_ROM[0] = Core_ROM_Base + 0x4000 * (((PRG_Block << 2) | PRG_Bank) & PRG_Mask);
			Core_ROM[1] = Core_ROM_Base + 0x4000 * (((PRG_Block << 2) | 3) & PRG_Mask);
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 14) & 1][addr & 0x3FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			if ((addr & 0x4000) != 0)
			{
				PRG_Bank = value & 0x3;
			}
			else
			{
				PRG_Block = (value >> 3) & 3;
			}

			Remap_ROM();
		}
	};
}