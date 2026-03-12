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
	class Mapper_RockMan8 : Mappers
	{
	public:
		int ROM_bank;
		int ROM_mask;

		void Reset()
		{
			ROM_bank = 1;
			ROM_mask = Core._rom.Length / 0x4000 - 1;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }
	}

		byte ReadMemoryLow(ushort addr)
		{
			if (addr < 0x4000)
			{
				// lowest bank is fixed
				return Core._rom[addr];
			
			}
			else
			{
				return Core._rom[(addr - 0x4000) + ROM_bank * 0x4000];
			}
		}

		byte ReadMemoryHigh(ushort addr)
		{
			return 0xFF;
		}

		byte PeekMemoryLow(ushort addr)
		{
			return ReadMemoryLow(addr);
		}

		void WriteMemory(ushort addr, byte value)
		{
			if ((addr >= 0x2000) && (addr < 0x4000))
			{
				value &= 0x1F;

				if (value == 0) { value = 1; }

				// in hhugboy they just subtract 8, but to me looks like bits 4 and 5 are just swapped (and bit 4 is unused?)
				ROM_bank = ((value & 0xF) | ((value & 0x10) >> 1)) & ROM_mask;
			}
		}

		void PokeMemory(ushort addr, byte value)
		{
			WriteMemory(addr, value);
		}

		void SyncState(Serializer ser)
		{
			ser.Sync(nameof(ROM_bank), ref ROM_bank);
			ser.Sync(nameof(ROM_mask), ref ROM_mask);
		}
	}
}
