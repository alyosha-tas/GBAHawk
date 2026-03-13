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
	class Mapper_MBC7 : Mappers
	{
	public:
		bool RAM_enable_1, RAM_enable_2;
		bool is_erased;

		uint8_t acc_x_low;
		uint8_t acc_x_high;
		uint8_t acc_y_low;
		uint8_t acc_y_high;
		
		uint32_t ROM_bank;
		uint32_t ROM_mask;

		// EEPROM related
		bool CS_prev;
		bool CLK_prev;
		bool DI_prev;
		bool DO;
		bool instr_read;
		bool perf_instr;
		bool WR_EN;
		bool countdown_start;

		uint32_t instr_bit_counter;
		uint32_t instr;
		uint32_t EE_addr;
		uint32_t instr_case;
		uint32_t instr_clocks;
		uint32_t EE_value;
		uint32_t countdown;

		void Reset()
		{
			ROM_bank = 1;
			RAM_enable_1 = RAM_enable_2 = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }

			acc_x_low = 0;
			acc_x_high = 0x80;
			acc_y_low = 0;
			acc_y_high = 0x80;

			// reset acceerometer
		is_erased = false;

		// EEPROM related
		CS_prev = CLK_prev = DI_prev = DO = instr_read = perf_instr = WR_EN = countdown_start = false;
		instr_bit_counter = instr = EE_addr = instr_case = instr_clocks = EE_value = countdown = 0;
	}

		uint8_t ReadMemoryLow(uint16_t addr)
		{
			if (addr < 0x4000)
			{
				return Core_ROM[addr];
			}
			else
			{
				return Core_ROM[(addr - 0x4000) + ROM_bank * 0x4000];
			}
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			if (addr < 0xA000)
			{
				return 0xFF;
			}
			else if (addr < 0xB000)
			{
				if (RAM_enable_1 && RAM_enable_2)
				{
					return Register_Access_Read(addr);
				}
				else
				{
					return 0xFF;
				}
			}
			else
			{
				return 0xFF;
			}
		}

		uint8_t PeekMemoryLow(uint16_t addr)
		{
			return ReadMemoryLow(addr);
		}

		void WriteMemory(uint16_t addr, uint8_t value)
		{
			if (addr < 0xA000)
			{
				if (addr < 0x2000)
				{
					RAM_enable_1 = (value & 0xF) == 0xA;
				}
				else if (addr < 0x4000)
				{
					value &= 0xFF;

					//Console.WriteLine(Core.cpu.TotalExecutedCycles);
					//Console.WriteLine(value);

					ROM_bank &= 0x100;
					ROM_bank |= value;
					ROM_bank &= ROM_mask;
				}
				else if (addr < 0x6000)
				{
					RAM_enable_2 = (value & 0xF0) == 0x40;		
				}
			}
			else
			{
				if (RAM_enable_1 && RAM_enable_2)
				{
					Register_Access_Write(addr, value);
				}
			}
		}

		void PokeMemory(uint16_t addr, uint8_t value)
		{
			WriteMemory(addr, value);
		}

		uint8_t Register_Access_Read(uint16_t addr)
		{
			if ((addr & 0xA0F0) == 0xA000)
			{
				return 0xFF;
			}
			else if ((addr & 0xA0F0) == 0xA010)
			{
				return 0xFF;
			}
			else if ((addr & 0xA0F0) == 0xA020)
			{
				return acc_x_low;
			}
			else if ((addr & 0xA0F0) == 0xA030)
			{
				return acc_x_high;
			}
			else if ((addr & 0xA0F0) == 0xA040)
			{
				return acc_y_low;
			}
			else if ((addr & 0xA0F0) == 0xA050)
			{
				return acc_y_high;
			}
			else if ((addr & 0xA0F0) == 0xA060)
			{
				return 0xFF;
			}
			else if ((addr & 0xA0F0) == 0xA070)
			{
				return 0xFF;
			}
			else if ((addr & 0xA0F0) == 0xA080)
			{
				return (uint8_t)((CS_prev ? 0x80 : 0) |
							(CLK_prev ? 0x40 : 0) |
							(DI_prev ? 2 : 0) |
							(DO ? 1 : 0));
			}
			else
			{
				return 0xFF;
			}
		}

		void Register_Access_Write(uint16_t addr, uint8_t value)
		{
			if ((addr & 0xA0F0) == 0xA000)
			{
				if (value == 0x55)
				{
					//Console.WriteLine("Erasing ACC");

					is_erased = true;
					acc_x_low = 0x00;
					acc_x_high = 0x80;
					acc_y_low = 0x00;
					acc_y_high = 0x80;
				}
			}
			else if ((addr & 0xA0F0) == 0xA010)
			{
				if ((value == 0xAA) && is_erased)
				{
					// latch new accelerometer values
					//Console.WriteLine("Latching ACC");
					acc_x_low = (uint8_t)(*Core_Acc_X_State & 0xFF);
					acc_x_high = (uint8_t)((*Core_Acc_X_State & 0xFF00) >> 8);
					acc_y_low = (uint8_t)(*Core_Acc_Y_State & 0xFF);
					acc_y_high = (uint8_t)((*Core_Acc_Y_State & 0xFF00) >> 8);
				}
			}
			else if ((addr & 0xA0F0) == 0xA080)
			{
				// EEPROM writes
				EEPROM_write(value);
			}
		}

		void EEPROM_write(uint8_t value)
		{
			bool CS = ((value & 0x80) == 0x80);
			bool CLK = ((value & 0x40) == 0x40);
			bool DI = ((value & 0x2) == 0x2);

			// if we deselect the chip, complete instructions or countdown and stop
			if (!CS)
			{
				CS_prev = CS;
				CLK_prev = CLK;
				DI_prev = DI;

				DO = true;
				countdown_start = false;
				perf_instr = false;
				instr_read = false;

				//Console.Write("Chip De-selected: ");
				//Console.WriteLine(Core.cpu.TotalExecutedCycles);
			}

			if (!instr_read && !perf_instr)
			{
				// if we aren't performing an operation or reading an incoming instruction, we are waiting for one
				// this is signalled by CS and DI both being 1 while CLK goes from 0 to 1
				if (CLK && !CLK_prev && DI && CS)
				{
					instr_read = true;
					instr_bit_counter = 0;
					instr = 0;
					DO = false;
					//Console.Write("Initiating command: ");
					//Console.WriteLine(Core.cpu.TotalExecutedCycles);
				}
			}
			else if (instr_read && CLK && !CLK_prev)
			{
				// all instructions are 10 bits long
				instr = (instr << 1) | ((value & 2) >> 1);

				instr_bit_counter++;
				if (instr_bit_counter == 10)
				{
					instr_read = false;
					instr_clocks = 0;
					EE_addr = instr & 0x7F;
					EE_value = 0;

					switch (instr & 0x300)
					{
						case 0x0:
							switch (instr & 0xC0)
							{
								case 0x0: // disable writes
									instr_case = 0;
									WR_EN = false;
									DO = true;
									break;
								case 0x40: // fill mem with value
									instr_case = 1;
									perf_instr = true;
									break;
								case 0x80: // fill mem with FF
									instr_case = 2;
									if (WR_EN)
									{
										for (uint32_t i = 0; i < 256; i++)
										{
											Core_Cart_RAM[i] = 0xFF;
										}
									}
									DO = true;
									break;
								case 0xC0: // enable writes
									instr_case = 3;
									WR_EN = true;
									DO = true;
									break;
							}
							break;
						case 0x100: // write to address
							instr_case = 4;
							perf_instr = true;
							break;
						case 0x200: // read from address
							instr_case = 5;
							perf_instr = true;
							break;
						case 0x300: // set address to FF
							instr_case = 6;
							if (WR_EN)
							{
								Core_Cart_RAM[EE_addr * 2] = 0xFF;
								Core_Cart_RAM[EE_addr * 2 + 1] = 0xFF;
							}
							DO = true;
							break;
					}

					//Console.Write("Selected Command: ");
					//Console.Write(instr_case);
					//Console.Write(" ");
					//Console.WriteLine(Core.cpu.TotalExecutedCycles);
				}
			}
			else if (perf_instr && CLK && !CLK_prev)
			{
				//Console.Write("Command In progress, Cycle: ");
				//Console.Write(instr_clocks);
				//Console.Write(" ");
				//Console.WriteLine(Core.cpu.TotalExecutedCycles);

				// for commands that require additional clocking
				switch (instr_case)
				{
					case 1:
						EE_value = (EE_value << 1) | ((value & 2) >> 1);

						if (instr_clocks == 15)
						{
							if (WR_EN)
							{
								for (uint32_t i = 0; i < 128; i++)
								{
									Core_Cart_RAM[i * 2] = (uint8_t)(EE_value & 0xFF);
									Core_Cart_RAM[i * 2 + 1] = (uint8_t)((EE_value & 0xFF00) >> 8);
								}
							}
							instr_case = 7;
							countdown = 8;
						}
						break;

					case 4:
						EE_value = (EE_value << 1) | ((value & 2) >> 1);

						if (instr_clocks == 15)
						{
							if (WR_EN)
							{
								Core_Cart_RAM[EE_addr * 2] = (uint8_t)(EE_value & 0xFF);
								Core_Cart_RAM[EE_addr * 2 + 1] = (uint8_t)((EE_value & 0xFF00) >> 8);
							}
							instr_case = 7;
							countdown = 8;
						}
						break;

					case 5:
						if ((instr_clocks >= 0) && (instr_clocks <= 7))
						{
							DO = ((Core_Cart_RAM[EE_addr * 2 + 1] >> (7 - instr_clocks)) & 1) == 1;
						}
						else if ((instr_clocks >= 8) && (instr_clocks <= 15))
						{
							DO = ((Core_Cart_RAM[EE_addr * 2] >> (15 - instr_clocks)) & 1) == 1;
						}

						if (instr_clocks == 15)
						{
							instr_case = 7;
							countdown = 8;
						}				
						break;

					case 6:

						instr_case = 7;
						countdown = 8;
						break;

					case 7:
						// completed operations take time, so countdown a bit here. 
						// not cycle accurate for operations like writing to all of the EEPROM, but good enough

						break;
				}

				if (instr_case == 7)
				{
					perf_instr = false;
					countdown_start = true;
				}

				instr_clocks++;
			}
			else if (countdown_start)
			{
				countdown--;
				if (countdown == 0)
				{
					countdown_start = false;
					DO = true;

					//Console.Write("Command Complete: ");
					//Console.WriteLine(Core.cpu.TotalExecutedCycles);
				}
			}

			CS_prev = CS;
			CLK_prev = CLK;
			DI_prev = DI;
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(RAM_enable_1, saver);
			saver = bool_saver(RAM_enable_2, saver);
			saver = bool_saver(is_erased, saver);

			saver = byte_saver(acc_x_low, saver);
			saver = byte_saver(acc_x_high, saver);
			saver = byte_saver(acc_y_low, saver);
			saver = byte_saver(acc_y_high, saver);

			saver = int_saver(ROM_bank, saver);
			saver = int_saver(ROM_mask, saver);

			// EEPROM related
			saver = bool_saver(CS_prev, saver);
			saver = bool_saver(CLK_prev, saver);
			saver = bool_saver(DI_prev, saver);
			saver = bool_saver(DO, saver);
			saver = bool_saver(instr_read, saver);
			saver = bool_saver(perf_instr, saver);
			saver = bool_saver(WR_EN, saver);
			saver = bool_saver(countdown_start, saver);

			saver = int_saver(instr_bit_counter, saver);
			saver = int_saver(instr, saver);
			saver = int_saver(EE_addr, saver);
			saver = int_saver(instr_case, saver);
			saver = int_saver(instr_clocks, saver);
			saver = int_saver(EE_value, saver);
			saver = int_saver(countdown, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&RAM_enable_1, loader);
			loader = bool_loader(&RAM_enable_2, loader);
			loader = bool_loader(&is_erased, loader);

			loader = byte_loader(&acc_x_low, loader);
			loader = byte_loader(&acc_x_high, loader);
			loader = byte_loader(&acc_y_low, loader);
			loader = byte_loader(&acc_y_high, loader);

			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&ROM_mask, loader);

			// EEPROM related
			loader = bool_loader(&CS_prev, loader);
			loader = bool_loader(&CLK_prev, loader);
			loader = bool_loader(&DI_prev, loader);
			loader = bool_loader(&DO, loader);
			loader = bool_loader(&instr_read, loader);
			loader = bool_loader(&perf_instr, loader);
			loader = bool_loader(&WR_EN, loader);
			loader = bool_loader(&countdown_start, loader);

			loader = int_loader(&instr_bit_counter, loader);
			loader = int_loader(&instr, loader);
			loader = int_loader(&EE_addr, loader);
			loader = int_loader(&instr_case, loader);
			loader = int_loader(&instr_clocks, loader);
			loader = int_loader(&EE_value, loader);
			loader = int_loader(&countdown, loader);

			return loader;
		}
	};
}
