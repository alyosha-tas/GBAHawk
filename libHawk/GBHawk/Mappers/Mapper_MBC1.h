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
	class Mapper_MBC1 : Mappers
	{
	public:
		bool RAM_enable;
		bool sel_mode;
		
		uint32_t ROM_bank;
		uint32_t RAM_bank;
		uint32_t ROM_mask;
		uint32_t RAM_mask;

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			sel_mode = false;
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
				// lowest bank is fixed, but is still effected by mode
				if (sel_mode)
				{
					return Core_ROM[(ROM_bank & 0x60) * 0x4000 + addr];
				}

				return Core_ROM[addr];
			}

			return Core_ROM[(addr - 0x4000) + ROM_bank * 0x4000];
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
					return *Core_Cycle_Count > (*Core_Bus_Access_Time + 8)
						? (uint8_t) 0xFF
						: *Core_Bus_Value;
				}
			}
			else
			{
				return *Core_Cycle_Count > (*Core_Bus_Access_Time + 8)
					? (uint8_t) 0xFF
					: *Core_Bus_Value;
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
				else if (addr < 0x4000)
				{
					value &= 0x1F;

					// writing zero gets translated to 1
					if (value == 0) { value = 1; }

					ROM_bank &= 0xE0;
					ROM_bank |= value;
					ROM_bank &= ROM_mask;
				}
				else if (addr < 0x6000)
				{
					if (sel_mode && Core_Cart_RAM != nullptr)
					{
						RAM_bank = value & 3;
						RAM_bank &= RAM_mask;
					}
					else
					{
						ROM_bank &= 0x1F;
						ROM_bank |= ((value & 3) << 5);
						ROM_bank &= ROM_mask;
					}
				}
				else
				{
					sel_mode = (value & 1) > 0;

					if (sel_mode && Core_Cart_RAM != nullptr)
					{
						ROM_bank &= 0x1F;
						ROM_bank &= ROM_mask;
					}
					else
					{
						RAM_bank = 0;
					}
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
			saver = bool_saver(RAM_enable, saver);
			saver = bool_saver(sel_mode, saver);
			
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(RAM_bank, saver);
			saver = int_saver(ROM_mask, saver);
			saver = int_saver(RAM_mask, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&RAM_enable, loader);
			loader = bool_loader(&sel_mode, loader);
			
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&RAM_bank, loader);
			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&RAM_mask, loader);

			return loader;
		}
	};
}
