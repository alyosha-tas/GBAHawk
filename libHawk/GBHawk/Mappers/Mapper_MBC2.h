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
	class Mapper_MBC2 : Mappers
	{
	public:
		int ROM_bank;
		int RAM_bank;
		bool RAM_enable;
		int ROM_mask;

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = Core._rom.Length / 0x4000 - 1;
		}

		byte ReadMemoryLow(ushort addr)
		{
			if (addr < 0x4000)
			{
				return Core._rom[addr];
			}
			else
			{
				return Core._rom[(addr - 0x4000) + ROM_bank * 0x4000];
			} 
		}

		byte ReadMemoryHigh(ushort addr)
		{
			if ((addr >= 0xA000) && (addr < 0xA200))
			{
				if (RAM_enable)
				{
					return Core.cart_RAM[addr - 0xA000];
				}
				return 0xFF;
			}
			else
			{
				return 0xFF;
			}
		}

		byte PeekMemoryLow(ushort addr)
		{
			return ReadMemoryLow(addr);
		}

		void WriteMemory(ushort addr, byte value)
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
					if (ROM_bank==0) { ROM_bank = 1; }
				}
			}
			else if ((addr >= 0xA000) && (addr < 0xA200))
			{
				if (RAM_enable)
				{
					Core.cart_RAM[addr - 0xA000] = (byte)(value & 0xF);
				}				
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
			ser.Sync(nameof(RAM_bank), ref RAM_bank);
			ser.Sync(nameof(RAM_enable), ref RAM_enable);
		}
	}
}