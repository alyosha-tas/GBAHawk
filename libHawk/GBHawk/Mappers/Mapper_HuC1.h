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
	class Mapper_HuC1 : public Mappers
	{
	public:
		bool RAM_enable;
		bool IR_signal;
		
		uint32_t ROM_bank;
		uint32_t RAM_bank;
		uint32_t ROM_mask;
		uint32_t RAM_mask;

		void Reset()
		{
			ROM_bank = 0;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }

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
			if ((addr >= 0xA000) && (addr < 0xC000))
			{
				if (RAM_enable)
				{
					if (Core_Cart_RAM != nullptr)
					{
						if (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length)
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
				else
				{
					// when RAM isn't enabled, reading from this area will return IR sensor reading
					// for now we'll assume it never sees light (0xC0)
					return 0xC0;
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
					RAM_enable = (value & 0xF) != 0xE;
				}
				else if (addr < 0x4000)
				{
					value &= 0x3F;

					ROM_bank &= 0xC0;
					ROM_bank |= value;
					ROM_bank &= ROM_mask;
				}
				else if (addr < 0x6000)
				{
					RAM_bank = value & 3;
					RAM_bank &= RAM_mask;
				}
			}
			else
			{
				if (RAM_enable)
				{
					if (Core_Cart_RAM != nullptr)
					{
						if (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length)
						{
							Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000] = value;
						}
					}
				}
				else
				{
					// I don't know if other bits here have an effect
					if (value == 1)
					{
						IR_signal = true;
					}
					else if (value == 0)
					{
						IR_signal = false;
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
			saver = bool_saver(IR_signal, saver);
			
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(RAM_bank, saver);
			saver = int_saver(ROM_mask, saver);
			saver = int_saver(RAM_mask, saver);


			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&RAM_enable, loader);
			loader = bool_loader(&IR_signal, loader);
			
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&RAM_bank, loader);
			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&RAM_mask, loader);

			return loader;
		}
	};
}
