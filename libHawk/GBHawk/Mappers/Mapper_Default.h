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
	class Mapper_Default : Mappers
	{
	public:
		void Reset()
		{
			// nothing to initialize
		}

		uint8_t ReadMemoryLow(uint16_t addr)
		{
			return Core_ROM[addr];
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			if (Core_Cart_RAM != nullptr)
			{
				return Core_Cart_RAM[addr - 0xA000];
			}
			else
			{
				return *Core_Cycle_Count > (*Core_Bus_Access_Time + 8)
					? (uint8_t)0xFF
					: *Core_Bus_Value;
			}
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
			if (addr < 0x8000)
			{
				// no mapping hardware available
			}
			else
			{
				if (Core_Cart_RAM != nullptr)
				{
					Core_Cart_RAM[addr - 0xA000] = value;
				}
			}
		}

		void PokeMemory(uint16_t addr, uint8_t value)
		{
			WriteMemory(addr, value);
		}
	};
}
