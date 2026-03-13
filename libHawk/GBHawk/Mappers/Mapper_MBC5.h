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
	class Mapper_MBC5 : Mappers
	{
	public:
		uint32_t ROM_bank;
		uint32_t RAM_bank;
		bool RAM_enable;
		uint32_t ROM_mask;
		uint32_t RAM_mask;

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }
			if (ROM_mask > 0x100) { ROM_mask |= 0xFF; }

			RAM_mask = 0;
			if (Core_Cart_RAM != nullptr)
			{
				RAM_mask = *Core_Cart_RAM_Length / 0x2000 - 1;
				if (*Core_Cart_RAM_Length == 0x800) { RAM_mask = 0; }
			}
		}

		uint8_t ReadMemoryLow(uint16_t addr)
		{
			if (addr < 0x4000)
			{
				return Core_ROM[addr];
			}
			else
			{
				return Core_ROM[(addr - 0x4000) + ROM_bank * 0x4000];
			}
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			if (Core_Cart_RAM != nullptr)
			{
				if (RAM_enable && (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length))
				{
					return Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000];
				}
				else
				{
					return 0xFF;
				}

			}
			else
			{
				return 0xFF;
			}
		}

		uint8_t PeekMemoryLow(uint16_t addr)
		{
			return ReadMemoryLow(addr);
		}

		void WriteMemory(uint16_t addr, uint8_t value)
		{
			if (addr < 0x8000)
			{
				if (addr < 0x2000)
				{
					RAM_enable = (value & 0xF) == 0xA;
				}
				else if (addr < 0x3000)
				{
					value &= 0xFF;

					ROM_bank &= 0x100;
					ROM_bank |= value;
					ROM_bank &= ROM_mask;
				}
				else if (addr < 0x4000)
				{
					value &= 1;

					ROM_bank &= 0xFF;
					ROM_bank |= (value << 8);
					ROM_bank &= ROM_mask;
				}
				else if (addr < 0x6000)
				{
					RAM_bank = value & 0xF;
					RAM_bank &= RAM_mask;
				}
			}
			else
			{
				if (Core_Cart_RAM != nullptr)
				{
					if (RAM_enable && (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length))
					{
						Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000] = value;
					}
				}
			}
		}

		void PokeMemory(uint16_t addr, uint8_t value)
		{
			WriteMemory(addr, value);
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(RAM_bank, saver);
			saver = bool_saver(RAM_enable, saver);
			saver = int_saver(ROM_mask, saver);
			saver = int_saver(RAM_mask, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&RAM_bank, loader);
			loader = bool_loader(&RAM_enable, loader);
			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&RAM_mask, loader);

			return loader;
		}
	};
}
