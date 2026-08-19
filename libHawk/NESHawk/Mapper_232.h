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
			loader = int_loader(&PRG_Block, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Mask, loader);

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