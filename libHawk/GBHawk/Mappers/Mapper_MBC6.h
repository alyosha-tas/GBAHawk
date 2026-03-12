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
	class Mapper_MBC6 : Mappers
	{
	public:
		void Reset()
		{
			// nothing to initialize
		}

		byte ReadMemoryLow(ushort addr)
		{
			if (addr < 0x8000)
			{
				return Core._rom[addr];
			}
			else
			{
				if (Core.cart_RAM != null)
				{
					return Core.cart_RAM[addr - 0xA000];
				}
				else
				{
					return 0;
				}
			}
		}

		byte ReadMemoryHigh(ushort addr)
		{
			if (Core.cart_RAM != null)
			{
				return Core.cart_RAM[addr - 0xA000];
			}
			else
			{
				return 0;
			}
		}

		byte PeekMemoryLow(ushort addr)
		{
			return ReadMemoryLow(addr);
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
