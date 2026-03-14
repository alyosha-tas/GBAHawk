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
	class Mapper_MBC2 : public Mappers
	{
	public:
		bool RAM_enable;
		
		uint32_t ROM_bank;
		uint32_t RAM_bank;
		uint32_t ROM_mask;

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;
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
			if ((addr >= 0xA000) && (addr < 0xA200))
			{
				if (RAM_enable)
				{
					return Core_Cart_RAM[addr - 0xA000];
				}
				return 0xFF;
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
			if (addr < 0x2000)
			{
				if ((addr & 0x100) == 0)
				{
					RAM_enable = ((value & 0xA) == 0xA);
				}
			}
			else if (addr < 0x4000)
			{
				if ((addr & 0x100) > 0)
				{
					ROM_bank = value & 0xF & ROM_mask;
					if (ROM_bank == 0) { ROM_bank = 1; }
				}
			}
			else if ((addr >= 0xA000) && (addr < 0xA200))
			{
				if (RAM_enable)
				{
					Core_Cart_RAM[addr - 0xA000] = (uint8_t)(value & 0xF);
				}
			}
		}

		void PokeMemory(uint16_t addr, uint8_t value)
		{
			WriteMemory(addr, value);
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(RAM_enable, saver);
			
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(RAM_bank, saver);
			saver = int_saver(ROM_mask, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&RAM_enable, loader);
			
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&RAM_bank, loader);
			loader = int_loader(&ROM_mask, loader);

			return loader;
		}
	};
}