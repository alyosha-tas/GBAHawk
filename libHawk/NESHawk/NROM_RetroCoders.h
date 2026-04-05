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

		void Reset()
		{
			Chip_ID_Active = false;
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
	};
}