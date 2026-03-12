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
	class Mapper_Camera : Mappers
	{
	public:
		int ROM_bank;
		int RAM_bank;
		bool RAM_enable;
		int ROM_mask;
		int RAM_mask;
		bool regs_enable;
		byte[] regs = new byte[0x80];

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = Core._rom.Length / 0x4000 - 1;

			RAM_mask = Core.cart_RAM.Length / 0x2000 - 1;

			regs_enable = false;
		}

		byte ReadMemoryLow(ushort addr)
		{
			if (addr < 0x4000)
			{
				return Core._rom[addr];
			}

			return Core._rom[(addr - 0x4000) + ROM_bank * 0x4000];
		}

		byte ReadMemoryHigh(ushort addr)
		{
			if (regs_enable)
			{
				if ((addr & 0x7F) == 0)
				{
					return 0;// regs[0];
				}

				return 0;
			}

			if (/*RAM_enable && */(((addr - 0xA000) + RAM_bank * 0x2000) < Core.cart_RAM.Length))
			{
				return Core.cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000];
			}

			return 0xFF;
		}

		byte PeekMemoryLow(ushort addr)
		{
			return ReadMemoryLow(addr);
		}

		void WriteMemory(ushort addr, byte value)
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
					regs[(addr & 0x7F)] = (byte)(value & 0x7);
				}
				else
				{
					if (RAM_enable && (((addr - 0xA000) + RAM_bank * 0x2000) < Core.cart_RAM.Length))
					{
						Core.cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000] = value;
					}
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
			ser.Sync(nameof(RAM_mask), ref RAM_mask);
			ser.Sync(nameof(RAM_enable), ref RAM_enable);
			ser.Sync(nameof(regs_enable), ref regs_enable);
			ser.Sync(nameof(regs), ref regs, false);
		}
	}
}
