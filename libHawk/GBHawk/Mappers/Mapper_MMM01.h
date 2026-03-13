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
		bool SRAM_en;
		bool sel_mode;

		uint8_t RAM_enable_Reg;
		uint8_t ROM_bank_Reg;
		uint8_t RAM_bank_Reg;
		uint8_t Mode_Reg;

		uint32_t ROM_mask;
		uint32_t RAM_mask;
		uint32_t ROM_bank;
		uint32_t Forced_Mask;
		uint32_t RAM_bank;

		void Reset()
		{
			RAM_enable_Reg = 0;
			ROM_bank_Reg = 0;
			RAM_bank_Reg = 0;
			Mode_Reg = 0;

			SRAM_en = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;

			ROM_bank = 0x1FF & ROM_mask;
			Forced_Mask = 0x1FE & ROM_mask;
			RAM_bank = 0;
			sel_mode = false;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }

			RAM_mask = 0;
			if (Core_Cart_RAM != nullptr)
			{
				RAM_mask = *Core_Cart_RAM_Length / 0x2000 - 1;
				if (*Core_Cart_RAM_Length == 0x800) { RAM_mask = 0; }
			}
		}

		uint8_t ReadMemoryLow(uint16_t addr)
		{
			if (addr < 0x4000)
			{
				return Core_ROM[addr + Forced_Mask * 0x4000];
			}
			else
			{
				return Core_ROM[(addr - 0x4000) + ROM_bank * 0x4000];
			}
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			if (Core_Cart_RAM != nullptr)
			{
				return Core_Cart_RAM[(addr - 0xA000) + (RAM_bank * 0x2000)];
			}
			else
			{
				return 0;
			}
		}

		uint8_t PeekMemoryLow(uint16_t addr)
		{
			return ReadMemoryLow(addr);
		}

		void WriteMemory(uint16_t addr, uint8_t value)
		{
			//Console.WriteLine(addr + " " + value);
			
			if (addr < 0x8000)
			{
				if (addr < 0x2000)
				{
					// bottom 4 bits always accessible
					RAM_enable_Reg &= 0xF0;
					RAM_enable_Reg |= (uint8_t)(value & 0xF);

					if ((RAM_enable_Reg & 0x0A) == 0x0A)
					{
						SRAM_en = true;
					}
					else
					{
						SRAM_en = false;
					}

					// upper bits only accessible when bit 6 is reset
					if (!Get_Bit(RAM_enable_Reg, 6))
					{
						RAM_enable_Reg &= 0x0F;
						RAM_enable_Reg |= (uint8_t)(value & 0xF0);
					}
				}
				else if (addr < 0x4000)
				{
					// bottom bit always accessible
					ROM_bank_Reg &= 0xFE;
					ROM_bank_Reg |= (uint8_t)(value & 0x01);

					// next 4 bitss accessible based on status of mode register
					// zero adjusted
					if (!Get_Bit(Mode_Reg, 2))
					{
						ROM_bank_Reg &= 0xFD;
						ROM_bank_Reg |= (uint8_t)(value & 0x2);
					}
					if (!Get_Bit(Mode_Reg, 3))
					{
						ROM_bank_Reg &= 0xFB;
						ROM_bank_Reg |= (uint8_t)(value & 0x4);
					}
					if (!Get_Bit(Mode_Reg, 4))
					{
						ROM_bank_Reg &= 0xF7;
						ROM_bank_Reg |= (uint8_t)(value & 0x8);
					}
					if (!Get_Bit(Mode_Reg, 5))
					{
						ROM_bank_Reg &= 0xEF;
						ROM_bank_Reg |= (uint8_t)(value & 0x10);
					}


					// upper bits only accessible when bit 6 is reset in RAM_enable_Reg
					if (!Get_Bit(RAM_enable_Reg, 6))
					{
						ROM_bank_Reg &= 0x1F;
						ROM_bank_Reg |= (uint8_t)(value & 0xE0);
					}
				}
				else if (addr < 0x6000)
				{
					// bottom 2 writable based on bits in RAM_enable_Reg
					if (!Get_Bit(RAM_enable_Reg, 4))
					{
						RAM_bank_Reg &= 0xFE;
						RAM_bank_Reg |= (uint8_t)(value & 0x1);
					}
					if (!Get_Bit(RAM_enable_Reg, 5))
					{
						RAM_bank_Reg &= 0xFD;
						RAM_bank_Reg |= (uint8_t)(value & 0x2);
					}

					// upper bits only accessible when bit 6 is reset in RAM_enable_Reg
					if (!Get_Bit(RAM_enable_Reg, 6))
					{
						RAM_bank_Reg &= 0x03;
						RAM_bank_Reg |= (uint8_t)(value & 0xFC);
					}
				}
				else
				{
					// bit 0 only accessible when RAM_bank_reg bit 6 reset
					if (!Get_Bit(RAM_enable_Reg, 6))
					{
						Mode_Reg &= 0xFE;
						Mode_Reg |= (uint8_t)(value & 1);
					}

					// upper bits only accessible when bit 6 is reset in RAM_enable_Reg
					if (!Get_Bit(RAM_enable_Reg, 6))
					{
						Mode_Reg &= 0x01;
						Mode_Reg |= (uint8_t)(value & 0xFE);
					}
				}

				// update all memory access variables
				sel_mode = Get_Bit(Mode_Reg, 0);

				// setting this bit disables most special registers (the mapper is effectively MBC1 now)
				if (Get_Bit(RAM_enable_Reg, 6))
				{
					if (Get_Bit(Mode_Reg, 6))
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

				if (Get_Bit(Mode_Reg, 6))
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
				if (Core_Cart_RAM != nullptr && SRAM_en)
				{
					Core_Cart_RAM[(addr - 0xA000) + (RAM_bank * 0x2000)] = value;
				}
			}
		}

		void PokeMemory(uint16_t addr, uint8_t value)
		{
			WriteMemory(addr, value);
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(SRAM_en, saver);
			saver = bool_saver(sel_mode, saver);

			saver = byte_saver(RAM_enable_Reg, saver);
			saver = byte_saver(ROM_bank_Reg, saver);
			saver = byte_saver(RAM_bank_Reg, saver);
			saver = byte_saver(Mode_Reg, saver);

			saver = int_saver(ROM_mask, saver);
			saver = int_saver(RAM_mask, saver);
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(Forced_Mask, saver);
			saver = int_saver(RAM_bank, saver);


			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&SRAM_en, loader);
			loader = bool_loader(&sel_mode, loader);

			loader = byte_loader(&RAM_enable_Reg, loader);
			loader = byte_loader(&ROM_bank_Reg, loader);
			loader = byte_loader(&RAM_bank_Reg, loader);
			loader = byte_loader(&Mode_Reg, loader);

			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&RAM_mask, loader);
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&Forced_Mask, loader);
			loader = int_loader(&RAM_bank, loader);

			return loader;
		}
	};
}
