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

			WRAM_Enable = true;
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
			saver = Base_Saver(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&CHR_Bank, loader);
			loader = int_loader(&CHR_Mask, loader);

			// common to all mappers
			loader = Base_Loader(loader);

			Remap_ROM();

			return loader;
		}
	};
}