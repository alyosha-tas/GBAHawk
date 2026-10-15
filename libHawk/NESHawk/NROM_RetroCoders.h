#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class NROM_RetroCoders : public Mappers
	{
	public:

		bool Chip_ID_Active;
		
		void Reset()
		{
			Chip_ID_Active = false;

			WRAM_Enable = true;
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				if (*Core_CHR_ROM_Length > 0)
				{
					if (Chip_ID_Active)
					{
						Chip_ID_Active = false;
						return 0xAA;
					}
					
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

		void WritePPU(uint32_t addr, uint8_t value)
		{
			if (addr >= 0x2000)
			{
				uint32_t ofs = addr & 0x3FF;
				uint32_t block = (addr >> 10) & 3;

				Core_CIRAM[block][ofs] = value;
			}
			else if (*Core_CHR_ROM_Length > 0)
			{
				// keep track of writes that activate ppu chip ID
				if ((addr == 1) && (value == 0xAA))
				{
					Chip_ID_Active = true;
				}
			}
			else
			{
				VRAM[addr] = value;
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Chip_ID_Active, saver);

			// common to all mappers
			saver = Base_Saver(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Chip_ID_Active, loader);

			// common to all mappers
			loader = Base_Loader(loader);

			Remap_ROM();

			return loader;
		}
	};
}