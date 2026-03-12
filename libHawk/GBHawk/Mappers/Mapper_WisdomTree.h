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
	class Mapper_WisdomTree : Mappers
	{
	public:
		int ROM_bank;
		int ROM_mask;

		void Reset()
		{
			ROM_bank = 0;
			ROM_mask = Core._rom.Length / 0x8000 - 1;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }
			if (ROM_mask > 0x100) { ROM_mask |= 0xFF; }
		}

		byte ReadMemoryLow(ushort addr)
		{
			return Core._rom[ROM_bank * 0x8000 + addr];
		}

		byte ReadMemoryHigh(ushort addr)
		{
			return 0xFF;
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
			if (addr < 0x4000)
			{
				ROM_bank = ((addr << 1) & 0x1ff) >> 1;
				ROM_bank &= ROM_mask;
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
