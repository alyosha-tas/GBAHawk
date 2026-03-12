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
	class Mapper_Sachen1 : Mappers
	{
	public:
		// Sachen Bootleg Mapper
		// NOTE: Normally, locked mode is disabled after 31 rises of A15
		// this occurs when the Boot Rom is loading the nintendo logo into VRAM
		// instead of tracking that in the main memory map where it will just slow things down for no reason
		// we'll clear the 'locked' flag when the last byte of the logo is read
	
		int ROM_bank;
		bool locked;
		int ROM_mask;
		int ROM_bank_mask;
		int BASE_ROM_Bank;
		bool reg_access;
		ushort addr_last;
		int counter;

		void Reset()
		{
			ROM_bank = 1;
			ROM_mask = Core._rom.Length / 0x4000 - 1;
			BASE_ROM_Bank = 0;
			ROM_bank_mask = 0xFF;
			locked = true;
			reg_access = false;
			addr_last = 0;
			counter = 0;
		}

		byte ReadMemoryLow(ushort addr)
		{
			if (addr < 0x4000)
			{
				if (locked)
				{
					// header is scrambled
					if ((addr >= 0x100) && (addr < 0x200))
					{
						int temp0 = (addr & 1);
						int temp1 = (addr & 2);
						int temp4 = (addr & 0x10);
						int temp6 = (addr & 0x40);

						temp0 = temp0 << 6;
						temp1 = temp1 << 3;
						temp4 = temp4 >> 3;
						temp6 = temp6 >> 6;

						addr &= 0x1AC;
						addr |= (ushort)(temp0 | temp1 | temp4 | temp6);
					}
					addr |= 0x80;
				}

				return Core._rom[addr + BASE_ROM_Bank * 0x4000];
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
			if (addr < 0x2000)
			{
				if (reg_access)
				{
					BASE_ROM_Bank = value;
				}
			}
			else if (addr < 0x4000)
			{
				ROM_bank = (value > 0) ? value : 1;

				if ((value & 0x30) == 0x30)
				{
					reg_access = true;
				}
				else
				{
					reg_access = false;
				}
			}
			else if (addr < 0x6000)
			{
				if (reg_access)
				{
					ROM_bank_mask = value;
				}
			}
		}

		void PokeMemory(ushort addr, byte value)
		{
			WriteMemory(addr, value);
		}

		void Mapper_Tick()
		{
			if (locked)
			{
				if (((Core.addr_access & 0x8000) == 0) && ((addr_last & 0x8000) > 0) && (Core.addr_access >= 0x100))
				{
					counter++;
					Console.WriteLine(Core.cpu.TotalExecutedCycles);
				}

				if (Core.addr_access >= 0x100)
				{
					addr_last = Core.addr_access;
				}

				if (counter == 0x30)
				{
					locked = false;
					Console.WriteLine("Unlocked");
				}
			}
		}

		void SyncState(Serializer ser)
		{
			ser.Sync(nameof(ROM_bank), ref ROM_bank);
			ser.Sync(nameof(ROM_mask), ref ROM_mask);
			ser.Sync(nameof(locked), ref locked);
			ser.Sync(nameof(ROM_bank_mask), ref ROM_bank_mask);
			ser.Sync(nameof(BASE_ROM_Bank), ref BASE_ROM_Bank);
			ser.Sync(nameof(reg_access), ref reg_access);
			ser.Sync(nameof(addr_last), ref addr_last);
			ser.Sync(nameof(counter), ref counter);
		}
	}
}
