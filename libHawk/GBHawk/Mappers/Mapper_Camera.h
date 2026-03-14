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
	class Mapper_Camera : public Mappers
	{
	public:
		bool RAM_enable;
		bool regs_enable;
		
		uint32_t ROM_bank;
		uint32_t RAM_bank;
		uint32_t ROM_mask;
		uint32_t RAM_mask;

		uint8_t regs[0x80] = { };

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;

			RAM_mask = *Core_Cart_RAM_Length / 0x2000 - 1;

			regs_enable = false;
		}

		uint8_t ReadMemoryLow(uint16_t addr)
		{
			if (addr < 0x4000)
			{
				return Core_ROM[addr];
			}

			return Core_ROM[(addr - 0x4000) + ROM_bank * 0x4000];
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			if (regs_enable)
			{
				if ((addr & 0x7F) == 0)
				{
					return 0;// regs[0];
				}

				return 0;
			}

			if (/*RAM_enable && */(((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length))
			{
				return Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000];
			}

			return 0xFF;
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
				else if (addr < 0x4000)
				{
					ROM_bank = value;
					ROM_bank &= ROM_mask;
					//Console.WriteLine(addr + " " + value + " " + ROM_mask + " " + ROM_bank);
				}
				else if (addr < 0x6000)
				{
					if ((value & 0x10) == 0x10)
					{
						regs_enable = true;
					}
					else
					{
						regs_enable = false;
						RAM_bank = value & RAM_mask;
					}
				}
			}
			else
			{
				if (regs_enable)
				{
					regs[(addr & 0x7F)] = (uint8_t)(value & 0x7);
				}
				else
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
			saver = bool_saver(RAM_enable, saver);
			saver = bool_saver(regs_enable, saver);

			saver = int_saver(ROM_bank, saver);
			saver = int_saver(RAM_bank, saver);
			saver = int_saver(ROM_mask, saver);
			saver = int_saver(RAM_mask, saver);

			saver = byte_array_saver(regs, saver, 0x80);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&RAM_enable, loader);
			loader = bool_loader(&regs_enable, loader);

			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&RAM_bank, loader);
			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&RAM_mask, loader);

			loader = byte_array_loader(regs, loader, 0x80);

			return loader;
		}
	};
}
