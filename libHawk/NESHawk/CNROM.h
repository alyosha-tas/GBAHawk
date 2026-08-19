#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_CNROM : public Mappers
	{
	public:

		uint32_t CHR_Bank;
		uint32_t CHR_Mask;
		
		void Reset()
		{
			CHR_Bank = 0;

			CHR_Mask = (*Core_CHR_ROM_Length >> 13) - 1;
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			if (Bus_Conflicts)
			{
				value &= ReadPRG(addr);
			}

			CHR_Bank = value & CHR_Mask;
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				return Core_CHR_ROM[addr + (CHR_Bank << 13)];
			}

			uint32_t ofs = addr & 0x3FF;
			uint32_t block = (addr >> 10) & 3;

			return Core_CIRAM[block][ofs];
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(CHR_Bank, saver);
			saver = int_saver(CHR_Mask, saver);

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
			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Mask, loader);

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