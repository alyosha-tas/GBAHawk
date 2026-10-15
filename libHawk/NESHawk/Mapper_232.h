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

		uint32_t PRG_Bank;
		uint32_t PRG_Block;
		uint32_t PRG_Mask;
		
		void Reset()
		{
			PRG_Mask = (*Core_ROM_Length >> 14) - 1;

			PRG_Bank = 0;

			PRG_Block = 0;

			WRAM_Enable = true;

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

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(PRG_Block, saver);
			saver = int_saver(PRG_Bank, saver);
			saver = int_saver(PRG_Mask, saver);

			// common to all mappers
			saver = Base_Saver(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&PRG_Block, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Mask, loader);

			// common to all mappers
			loader = Base_Loader(loader);

			Remap_ROM();

			return loader;
		}
	};
}