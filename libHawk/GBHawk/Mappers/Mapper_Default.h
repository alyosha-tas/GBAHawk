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

		byte ReadMemoryLow(ushort addr)
		{
			return Core._rom[addr];
		}

		byte ReadMemoryHigh(ushort addr)
		{
			if (Core.cart_RAM != null)
			{
				return Core.cart_RAM[addr - 0xA000];
			}
			else
			{
				return Core.cpu.TotalExecutedCycles > (Core.bus_access_time + 8)
					? (byte) 0xFF
					: Core.bus_value;
			}
		}

		byte PeekMemoryLow(ushort addr)
		{
			return ReadMemoryLow(addr);
		}

		byte PeekMemoryHigh(ushort addr)
		{
			return ReadMemoryHigh(addr);
		}

		void WriteMemory(ushort addr, byte value)
		{
			if (addr < 0x8000)
			{
				// no mapping hardware available
			}
			else
			{
				if (Core.cart_RAM != null)
				{
					Core.cart_RAM[addr - 0xA000] = value;
				}
			}
		}

		void PokeMemory(ushort addr, byte value)
		{
			WriteMemory(addr, value);
		}
	}
}
