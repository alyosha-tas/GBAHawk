#pragma once
#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cmath>

#include "../Mappers.h"

using namespace std;

namespace GBHawk
{
	class Mapper_WisdomTree : public Mappers
	{
	public:
		uint32_t ROM_bank;
		uint32_t ROM_mask;

		void Reset()
		{
			ROM_bank = 0;
			ROM_mask = *Core_ROM_Length / 0x8000 - 1;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }
			if (ROM_mask > 0x100) { ROM_mask |= 0xFF; }
		}

		uint8_t ReadMemoryLow(uint16_t addr)
		{
			return Core_ROM[ROM_bank * 0x8000 + addr];
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			return 0xFF;
		}

		uint8_t PeekMemoryLow(uint16_t addr)
		{
			return ReadMemoryLow(addr);
		}

		uint8_t PeekMemoryHigh(uint16_t addr)
		{
			return ReadMemoryHigh(addr);
		}

		void WriteMemory(uint16_t addr, uint8_t value)
		{
			if (addr < 0x4000)
			{
				ROM_bank = ((addr << 1) & 0x1ff) >> 1;
				ROM_bank &= ROM_mask;
			}
		}

		void PokeMemory(uint16_t addr, uint8_t value)
		{
			WriteMemory(addr, value);
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(ROM_mask, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&ROM_mask, loader);

			return loader;
		}
	};
}
