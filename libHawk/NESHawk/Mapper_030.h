#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_030 : public Mappers
	{
	public:

		uint32_t CHR_Bank;
		uint32_t CHR_Mask;
		uint32_t PRG_Bank;
		uint32_t PRG_Mask;
		
		void Reset()
		{
			CHR_Bank = 0;

			CHR_Mask = 3;

			PRG_Mask = (*Core_ROM_Length >> 13) - 1;

			PRG_Bank = 0;

			WRAM_Enable = true;

			Remap_ROM();
		}

		void Remap_ROM()
		{
			Core_ROM[0] = Core_ROM_Base + 0x4000 * PRG_Bank;
			Core_ROM[1] = Core_ROM_Base + 0x4000 * PRG_Bank + 0x2000;
			Core_ROM[2] = Core_ROM_Base + *Core_ROM_Length - 0x4000;
			Core_ROM[3] = Core_ROM_Base + *Core_ROM_Length - 0x2000;
		}

		uint8_t ReadPRG(uint32_t addr)
		{
			return Core_ROM[(addr >> 13) & 3][addr & 0x1FFF];
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			PRG_Bank = value & 0x1F & PRG_Mask;

			CHR_Bank = (value >> 5) & 3;

			Remap_ROM();
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				return VRAM[CHR_Bank * 0x2000 + addr];
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
			else
			{
				VRAM[CHR_Bank * 0x2000 + addr] = value;
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(CHR_Bank, saver);
			saver = int_saver(CHR_Mask, saver);
			saver = int_saver(PRG_Bank, saver);
			saver = int_saver(PRG_Mask, saver);

			// common to all mappers
			saver = Base_Saver(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Mask, loader);
			loader = int_loader(&PRG_Bank, loader);
			loader = int_loader(&PRG_Mask, loader);

			// common to all mappers
			loader = Base_Loader(loader);

			Remap_ROM();

			return loader;
		}
	};
}
