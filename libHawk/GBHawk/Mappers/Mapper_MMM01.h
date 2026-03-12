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
	class Mapper_MMM01 : Mappers
	{
	public:
		byte RAM_enable_Reg;
		byte ROM_bank_Reg;
		byte RAM_bank_Reg;
		byte Mode_Reg;

		int ROM_mask;
		int RAM_mask;
		bool SRAM_en;

		int ROM_bank;
		int Forced_Mask;
		int RAM_bank;
		bool sel_mode;

		void Reset()
		{
			RAM_enable_Reg = 0;
			ROM_bank_Reg = 0;
			RAM_bank_Reg = 0;
			Mode_Reg = 0;

			SRAM_en = false;
			ROM_mask = Core._rom.Length / 0x4000 - 1;

			ROM_bank = 0x1FF & ROM_mask;
			Forced_Mask = 0x1FE & ROM_mask;
			RAM_bank = 0;
			sel_mode = false;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }

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
				return Core._rom[addr + Forced_Mask * 0x4000];
			}
			else
			{
				return Core._rom[(addr - 0x4000) + ROM_bank * 0x4000];
			}
		}

		byte ReadMemoryHigh(ushort addr)
		{
			if (Core.cart_RAM != null)
			{
				return Core.cart_RAM[(addr - 0xA000) + (RAM_bank * 0x2000)];
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
			//Console.WriteLine(addr + " " + value);
			
			if (addr < 0x8000)
			{
				if (addr < 0x2000)
				{
					// bottom 4 bits always accessible
					RAM_enable_Reg &= 0xF0;
					RAM_enable_Reg |= (byte)(value & 0xF);

					if ((RAM_enable_Reg & 0x0A) == 0x0A)
					{
						SRAM_en = true;
					}
					else
					{
						SRAM_en = false;
					}

					// upper bits only accessible when bit 6 is reset
					if (!RAM_enable_Reg.Bit(6))
					{
						RAM_enable_Reg &= 0x0F;
						RAM_enable_Reg |= (byte)(value & 0xF0);
					}
				}
				else if (addr < 0x4000)
				{
					// bottom bit always accessible
					ROM_bank_Reg &= 0xFE;
					ROM_bank_Reg |= (byte)(value & 0x01);

					// next 4 bitss accessible based on status of mode register
					// zero adjusted
					if (!Mode_Reg.Bit(2))
					{
						ROM_bank_Reg &= 0xFD;
						ROM_bank_Reg |= (byte)(value & 0x2);
					}
					if (!Mode_Reg.Bit(3))
					{
						ROM_bank_Reg &= 0xFB;
						ROM_bank_Reg |= (byte)(value & 0x4);
					}
					if (!Mode_Reg.Bit(4))
					{
						ROM_bank_Reg &= 0xF7;
						ROM_bank_Reg |= (byte)(value & 0x8);
					}
					if (!Mode_Reg.Bit(5))
					{
						ROM_bank_Reg &= 0xEF;
						ROM_bank_Reg |= (byte)(value & 0x10);
					}


					// upper bits only accessible when bit 6 is reset in RAM_enable_Reg
					if (!RAM_enable_Reg.Bit(6))
					{
						ROM_bank_Reg &= 0x1F;
						ROM_bank_Reg |= (byte)(value & 0xE0);
					}
				}
				else if (addr < 0x6000)
				{
					// bottom 2 writable based on bits in RAM_enable_Reg
					if (!RAM_enable_Reg.Bit(4))
					{
						RAM_bank_Reg &= 0xFE;
						RAM_bank_Reg |= (byte)(value & 0x1);
					}
					if (!RAM_enable_Reg.Bit(5))
					{
						RAM_bank_Reg &= 0xFD;
						RAM_bank_Reg |= (byte)(value & 0x2);
					}

					// upper bits only accessible when bit 6 is reset in RAM_enable_Reg
					if (!RAM_enable_Reg.Bit(6))
					{
						RAM_bank_Reg &= 0x03;
						RAM_bank_Reg |= (byte)(value & 0xFC);
					}
				}
				else
				{
					// bit 0 only accessible when RAM_bank_reg bit 6 reset
					if (!RAM_bank_Reg.Bit(6))
					{
						Mode_Reg &= 0xFE;
						Mode_Reg |= (byte)(value & 1);
					}

					// upper bits only accessible when bit 6 is reset in RAM_enable_Reg
					if (!RAM_enable_Reg.Bit(6))
					{
						Mode_Reg &= 0x01;
						Mode_Reg |= (byte)(value & 0xFE);
					}
				}

				// update all memory access variables
				sel_mode = Mode_Reg.Bit(0);

				// setting this bit disables most special registers (the mapper is effectively MBC1 now)
				if (RAM_enable_Reg.Bit(6))
				{
					if (Mode_Reg.Bit(6))
					{
						ROM_bank = (((RAM_bank_Reg & 0x30) << 3) | ((RAM_bank_Reg & 0x03) << 5) | (ROM_bank_Reg & 0x1F)) & ROM_mask;
						Forced_Mask = (((Mode_Reg >> 1) & 0x1E) | 0x1E0) & ROM_bank;
					}
					else
					{
						ROM_bank = (((RAM_bank_Reg & 0x30) << 3) | (ROM_bank_Reg & 0x7F)) & ROM_mask;
						Forced_Mask = (((Mode_Reg >> 1) & 0x1E) | 0x1E0) & ROM_bank;
					}

					if (ROM_bank == Forced_Mask) { ROM_bank += 1; }
				}
				else
				{
					// Taito Variety pack expects bank 0x1FF to be mapped in even if not written to
					// does writing actually have any effect if bit 6 is clear?
				}

				if (Mode_Reg.Bit(6))
				{
					RAM_bank = ((RAM_bank_Reg & 0x0C) | ((ROM_bank_Reg & 0x60) >> 5)) & RAM_mask;
				}
				else
				{
					RAM_bank = (RAM_bank_Reg & 0xF) & RAM_mask;
				}
			}
			else
			{
				if (Core.cart_RAM != null && SRAM_en)
				{
					Core.cart_RAM[(addr - 0xA000) + (RAM_bank * 0x2000)] = value;
				}
			}
		}

		void PokeMemory(ushort addr, byte value)
		{
			WriteMemory(addr, value);
		}

		void SyncState(Serializer ser)
		{
			ser.Sync(nameof(RAM_enable_Reg), ref RAM_enable_Reg);
			ser.Sync(nameof(ROM_bank_Reg), ref ROM_bank_Reg);
			ser.Sync(nameof(RAM_bank_Reg), ref RAM_bank_Reg);
			ser.Sync(nameof(Mode_Reg), ref Mode_Reg);

			ser.Sync(nameof(ROM_mask), ref ROM_mask);	
			ser.Sync(nameof(RAM_mask), ref RAM_mask);
			ser.Sync(nameof(SRAM_en), ref SRAM_en);

			ser.Sync(nameof(ROM_bank), ref ROM_bank);
			ser.Sync(nameof(Forced_Mask), ref Forced_Mask);
			ser.Sync(nameof(RAM_bank), ref RAM_bank);
			ser.Sync(nameof(sel_mode), ref sel_mode);
		}
	}
}
