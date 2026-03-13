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
	class Mapper_Sachen2 : Mappers
	{
	public:
		// Sachen Bootleg Mapper
		// NOTE: Normally, locked mode is disabled after 31 rises of A15
		// this occurs when the Boot Rom is loading the nintendo logo into VRAM
		// instead of tracking that in the main memory map where it will just slow things down for no reason
		// we'll clear the 'locked' flag when the last uint8_t of the logo is read
		uint32_t ROM_bank;
		bool locked, locked_GBC, finished;
		uint32_t ROM_mask;
		uint32_t ROM_bank_mask;
		uint32_t BASE_ROM_Bank;
		bool reg_access;
		uint16_t addr_last;
		uint32_t counter;

		void Reset()
		{
			ROM_bank = 1;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;
			BASE_ROM_Bank = 0;
			ROM_bank_mask = 0;
			locked = true;
			locked_GBC = false;
			finished = false;
			reg_access = false;
			addr_last = 0;
			counter = 0;
		}

		uint8_t ReadMemoryLow(uint16_t addr)
		{
			if (addr < 0x4000)
			{
				// header is scrambled
				if ((addr >= 0x100) && (addr < 0x200))
				{
					uint32_t temp0 = (addr & 1);
					uint32_t temp1 = (addr & 2);
					uint32_t temp4 = (addr & 0x10);
					uint32_t temp6 = (addr & 0x40);

					temp0 = temp0 << 6;
					temp1 = temp1 << 3;
					temp4 = temp4 >> 3;
					temp6 = temp6 >> 6;

					addr &= 0x1AC;
					addr |= (uint16_t)(temp0 | temp1 | temp4 | temp6);
				}

				if (locked_GBC) { addr |= 0x80; }

				return Core_ROM[addr + BASE_ROM_Bank * 0x4000];
			}
			else
			{
				uint32_t temp_bank = (ROM_bank & ~ROM_bank_mask) | (ROM_bank_mask & BASE_ROM_Bank);
				temp_bank &= ROM_mask;

				return Core_ROM[(addr - 0x4000) + temp_bank * 0x4000];
			}
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			return 0xFF;
		}

		uint8_t PeekMemoryLow(uint16_t addr)
		{
			return ReadMemoryLow(addr);
		}

		void WriteMemory(uint16_t addr, uint8_t value)
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
				ROM_bank = (value > 0) ? (value) : 1;

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

		void PokeMemory(uint16_t addr, uint8_t value)
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
				}

				if (Core.addr_access >= 0x100)
				{
					addr_last = Core.addr_access;
				}

				if (counter == 0x30)
				{
					locked = false;
					locked_GBC = true;
					counter = 0;
				}
			}
			else if (locked_GBC)
			{
				if (((Core.addr_access & 0x8000) == 0) && ((addr_last & 0x8000) > 0) && (Core.addr_access >= 0x100))
				{
					counter++;
				}

				if (Core.addr_access >= 0x100)
				{
					addr_last = Core.addr_access;
				}

				if (counter == 0x30)
				{
					locked_GBC = false;
					finished = true;
					Console.WriteLine("Finished");
					Console.WriteLine(Core.cpu.TotalExecutedCycles);
				}

				// The above condition seems to never be reached as described in the mapper notes
				// so for now add this one

				if ((Core.addr_access == 0x133) && (counter == 1))
				{
					locked_GBC = false;
					finished = true;
					Console.WriteLine("Unlocked");
				}
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(ROM_bank, saver);
			saver = bool_saver(locked, locked_GBC, finished, saver);
			saver = int_saver(ROM_mask, saver);
			saver = int_saver(ROM_bank_mask, saver);
			saver = int_saver(BASE_ROM_Bank, saver);
			saver = bool_saver(reg_access, saver);
			saver = short_saver(addr_last, saver);
			saver = int_saver(counter, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&ROM_bank, loader);
			loader = bool_loader(&locked, locked_GBC, finished, loader);
			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&ROM_bank_mask, loader);
			loader = int_loader(&BASE_ROM_Bank, loader);
			loader = bool_loader(&reg_access, loader);
			loader = short_loader(&addr_last, loader);
			loader = int_loader(&counter, loader);

			return loader;
		}
	};
}