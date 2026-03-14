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
	class Mapper_HuC3 : public Mappers
	{
	public:
		
		bool RAM_enable;
		bool IR_signal;
		bool timer_read;

		uint8_t control;
		uint8_t chip_read;

		uint32_t ROM_bank;
		uint32_t RAM_bank;
		uint32_t ROM_mask;
		uint32_t RAM_mask;
		uint32_t time_val_shift;
		uint32_t time;
		uint32_t RTC_timer;
		uint32_t RTC_low_clock;
		uint32_t RTC_seconds;

		void Reset()
		{
			ROM_bank = 0;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;
			control = 0;
			chip_read = 1;
			timer_read = false;
			time_val_shift = 0;

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
				return Core_ROM[addr];
			}
			else
			{
				return Core_ROM[(addr - 0x4000) + ROM_bank * 0x4000];
			}
		}

		uint8_t ReadMemoryHigh(uint16_t addr)
		{
			if ((control >= 0xB) && (control < 0xE))
			{
				if (control == 0xD)
				{
					return 1;
				}
				return chip_read;
			}

			if (Core_Cart_RAM != nullptr)
			{
				if (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length)
				{
					return Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000];
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
			if (addr < 0x8000)
			{
				if (addr < 0x2000)
				{
					RAM_enable = (value & 0xA) == 0xA;
					control = value;
				}
				else if (addr < 0x4000)
				{
					if (value == 0) { value = 1; }

					ROM_bank = value;
					ROM_bank &= ROM_mask;
				}
				else if (addr < 0x6000)
				{
					RAM_bank = value;
					RAM_bank &= 0xF;
					RAM_bank &= RAM_mask;
				}
			}
			else
			{
				if ((control < 0xB) || (control > 0xE))
				{
					if (!RAM_enable)
					{
						return;
					}

					if (Core_Cart_RAM != nullptr)
					{
						if (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length)
						{
							Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000] = value;
						}
					}

					return;
				}
				
				if (control == 0xB)
				{
					switch (value & 0xF0)
					{
						case 0x10:
							if (timer_read)
							{
								// return timer value
								chip_read = (uint8_t)((time >> time_val_shift) & 0xF);
								time_val_shift += 4;
								if (time_val_shift == 28) { time_val_shift = 0; }
							}
							break;
						case 0x20:
							break;
						case 0x30:
							if (!timer_read)
							{
								// write to timer
								if (time_val_shift == 0) { time = 0; }							
								if (time_val_shift < 28)
								{
									time |= (uint32_t)((value & 0x0F) << time_val_shift);
									time_val_shift += 4;
									if (time_val_shift == 28) { timer_read = true; }
								}
							}
							break;
						case 0x40:
							// other commands
							switch (value & 0xF)
							{
								case 0x0:
									time_val_shift = 0;
									break;
								case 0x3:
									timer_read = false;
									time_val_shift = 0;
									break;
								case 0x7:
									timer_read = true;
									time_val_shift = 0;
									break;
								case 0xF:
									break;
							}
							break;
						case 0x50:
							break;
						case 0x60:
							timer_read = true;
							break;
					}
				}
				else if (control == 0xC)
				{
					// maybe IR
				}
				else if (control == 0xD)
				{
					// maybe IR
				}

				// Still write to RAM if another command executed
				if (Core_Cart_RAM != nullptr)
				{
					if (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length)
					{
						Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000] = value;
					}
				}
			}
		}

		void RTC_Set(int32_t value, uint32_t index)
		{
			time |= (uint32_t)((value & 0xFF) << index);
		}

		void Mapper_Tick()
		{
			RTC_timer++;

			if (RTC_timer == 128)
			{
				RTC_timer = 0;

				RTC_low_clock++;

				if (RTC_low_clock == 32768)
				{
					RTC_low_clock = 0;

					RTC_seconds++;
					if (RTC_seconds > 59)
					{
						RTC_seconds = 0;
						time++;
						if ((time & 0xFFF) > 1439)
						{
							time -= 1440;
							time += (1 << 12);
							if ((time >> 12) > 365)
							{
								time -= (365 << 12);
								time += (1 << 24);
							}
						}
					}
				}
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(RAM_enable, saver);
			saver = bool_saver(IR_signal, saver);
			saver = bool_saver(timer_read, saver);

			saver = byte_saver(control, saver);
			saver = byte_saver(chip_read, saver);
			
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(RAM_bank, saver);
			saver = int_saver(ROM_mask, saver);
			saver = int_saver(RAM_mask, saver);
			saver = int_saver(time_val_shift, saver);
			saver = int_saver(time, saver);
			saver = int_saver(RTC_timer, saver);
			saver = int_saver(RTC_low_clock, saver);
			saver = int_saver(RTC_seconds, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&RAM_enable, loader);
			loader = bool_loader(&IR_signal, loader);
			loader = bool_loader(&timer_read, loader);

			loader = byte_loader(&control, loader);
			loader = byte_loader(&chip_read, loader);
			
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&RAM_bank, loader);
			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&RAM_mask, loader);
			loader = int_loader(&time_val_shift, loader);
			loader = int_loader(&time, loader);
			loader = int_loader(&RTC_timer, loader);
			loader = int_loader(&RTC_low_clock, loader);
			loader = int_loader(&RTC_seconds, loader);

			return loader;
		}
	};
}
