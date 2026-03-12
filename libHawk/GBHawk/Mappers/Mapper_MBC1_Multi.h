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
	class Mapper_MBC1Multi : Mappers
	{
	public:
		int ROM_bank;
		int RAM_bank;
		bool RAM_enable;
		bool sel_mode;
		int ROM_mask;
		int RAM_mask;

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			sel_mode = false;
			ROM_mask = (Core._rom.Length / 0x4000 * 2) - 1; // due to how mapping works, we want a 1 bit higher mask
			RAM_mask = 0;
			if (Core.cart_RAM != null)
			{
				RAM_mask = Core.cart_RAM.Length / 0x2000 - 1;
				if (Core.cart_RAM.Length == 0x800) { RAM_mask = 0; }
			}
		}

		byte ReadMemoryLow(ushort addr)
		{
			if (addr < 0x4000)
			{
				// lowest bank is fixed, but is still effected by mode
				if (sel_mode)
				{
					return Core._rom[((ROM_bank & 0x60) >> 1) * 0x4000 + addr];
				}
				else
				{
					return Core._rom[addr];
				}
			}
			else
			{
				return Core._rom[(addr - 0x4000) + (((ROM_bank & 0x60) >> 1) | (ROM_bank & 0xF)) * 0x4000];			
			}
		}

		byte ReadMemoryHigh(ushort addr)
		{
			if (Core.cart_RAM != null)
			{
				if (RAM_enable && (((addr - 0xA000) + RAM_bank * 0x2000) < Core.cart_RAM.Length))
				{
					return Core.cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000];
				}
				else
				{
					return Core.cpu.TotalExecutedCycles > (Core.bus_access_time + 8)
						? (byte) 0xFF
						: Core.bus_value;
				}

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

		void WriteMemory(ushort addr, byte value)
		{
			if (addr < 0x8000)
			{
				if (addr < 0x2000)
				{
					RAM_enable = ((value & 0xA) == 0xA);
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
					if (sel_mode && Core.cart_RAM != null)
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

					if (sel_mode && Core.cart_RAM != null)
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
				if (Core.cart_RAM != null)
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
			ser.Sync(nameof(sel_mode), ref sel_mode);
		}
	}
}
