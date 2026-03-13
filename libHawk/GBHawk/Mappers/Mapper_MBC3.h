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
	class Mapper_MBC3 : Mappers
	{
	public:
		uint32_t ROM_bank;
		uint32_t RAM_bank;
		bool RAM_enable;
		uint32_t ROM_mask;
		uint32_t RAM_mask;
		uint8_t[] RTC_regs = new uint8_t[5];
		uint8_t[] RTC_regs_latch = new uint8_t[5];
		bool RTC_regs_latch_wr;
		uint32_t RTC_timer;
		uint32_t RTC_low_clock;
		bool halt;
		uint32_t RTC_offset;

		void Reset()
		{
			ROM_bank = 1;
			RAM_bank = 0;
			RAM_enable = false;
			ROM_mask = *Core_ROM_Length / 0x4000 - 1;

			// some games have sizes that result in a degenerate ROM, account for it here
			if (ROM_mask > 4) { ROM_mask |= 3; }

			RAM_mask = 0;
			if (Core_Cart_RAM != nullptr)
			{
				RAM_mask = *Core_Cart_RAM_Length / 0x2000 - 1;
				if (*Core_Cart_RAM_Length == 0x800) { RAM_mask = 0; }
			}

			RTC_regs_latch[0] = 0;
			RTC_regs_latch[1] = 0;
			RTC_regs_latch[2] = 0;
			RTC_regs_latch[3] = 0;
			RTC_regs_latch[4] = 0;

			RTC_regs_latch_wr = true;
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
			if (RAM_enable)
			{
				if ((Core_Cart_RAM != nullptr) && (RAM_bank <= RAM_mask))
				{
					if (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length)
					{
						return Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000];
					}
					else
					{
						return Core.cpu.TotalExecutedCycles > (Core.bus_access_time + 8)
							? (uint8_t) 0xFF
							: Core.bus_value;
					}
				}

				if ((RAM_bank >= 8) && (RAM_bank <= 0xC))
				{
					//Console.WriteLine("reg: " + (RAM_bank - 8) + " value: " + RTC_regs_latch[RAM_bank - 8] + " cpu: " + Core.cpu.TotalExecutedCycles);
					return RTC_regs_latch[RAM_bank - 8];
				}
				else
				{
					return 0xFF;
				}
			}
			else
			{
				return Core.cpu.TotalExecutedCycles > (Core.bus_access_time + 8)
					? (uint8_t) 0xFF
					: Core.bus_value;
			}
		}

		uint8_t PeekMemoryLow(uint16_t addr)
		{
			return ReadMemoryLow(addr);
		}

		uint8_t PeekMemoryHigh(uint16_t addr)
		{

			if ((Core_Cart_RAM != nullptr) && (RAM_bank <= RAM_mask))
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

			if ((RAM_bank >= 8) && (RAM_bank <= 0xC))
			{
				//Console.WriteLine("reg: " + (RAM_bank - 8) + " value: " + RTC_regs_latch[RAM_bank - 8] + " cpu: " + Core.cpu.TotalExecutedCycles);
				return RTC_regs_latch[RAM_bank - 8];
			}
			else
			{
				return 0x0;
			}
		}

		void WriteMemory(uint16_t addr, uint8_t value)
		{
			if (addr < 0x8000)
			{
				if (addr < 0x2000)
				{					
					RAM_enable = ((value & 0xF) == 0xA);
				}
				else if (addr < 0x4000)
				{
					value &= 0x7F;

					// writing zero gets translated to 1
					if (value == 0) { value = 1; }

					ROM_bank = value;
					ROM_bank &= ROM_mask;
				}
				else if (addr < 0x6000)
				{
					RAM_bank = value;
				}
				else
				{
					if (!RTC_regs_latch_wr && ((value & 1) == 1))
					{
						for (uint32_t i = 0; i < 5; i++)
						{
							RTC_regs_latch[i] = RTC_regs[i];
						}
					}

					RTC_regs_latch_wr = (value & 1) > 0;
				}
			}
			else
			{
				if (RAM_enable)
				{
					if ((Core_Cart_RAM != nullptr) && (RAM_bank <= RAM_mask))
					{
						if (((addr - 0xA000) + RAM_bank * 0x2000) < *Core_Cart_RAM_Length)
						{
							Core_Cart_RAM[(addr - 0xA000) + RAM_bank * 0x2000] = value;
						}
					}
					else if ((RAM_bank >= 8) && (RAM_bank <= 0xC))
					{
						// not all bits are writable
						switch (RAM_bank - 8)
						{
							case 0: value &= 0x3F;		break;
							case 1: value &= 0x3F;		break;
							case 2: value &= 0x1F;		break;
							case 3: value &= 0xFF;		break;
							case 4: value &= 0xC1;		break;
						}

						RTC_regs[RAM_bank - 8] = value;

						if ((RAM_bank - 8) == 0) { RTC_low_clock = RTC_timer = 0; }

						halt = (RTC_regs[4] & 0x40) > 0;
					}
				}
			}
		}

		void PokeMemory(uint16_t addr, uint8_t value)
		{
			WriteMemory(addr, value);
		}

		void RTC_Get(uint32_t value, uint32_t index)
		{
			if (index < 5)
			{
				RTC_regs[index] = (uint8_t)value;
			}		
			else
			{
				RTC_offset = value;
			}
		}

		void Mapper_Tick()
		{
			if (!halt)
			{
				RTC_timer++;

				if (RTC_timer == 128)
				{
					RTC_timer = 0;

					RTC_low_clock++;

					if (RTC_low_clock == 32768)
					{
						RTC_low_clock = 0;
						RTC_timer = RTC_offset;

						RTC_regs[0]++;

						if (RTC_regs[0] == 60)
						{
							RTC_regs[0] = 0;
							RTC_regs[1]++;
							if (RTC_regs[1] == 60)
							{
								RTC_regs[1] = 0;
								RTC_regs[2]++;
								if (RTC_regs[2] == 24)
								{
									RTC_regs[2] = 0;
									if (RTC_regs[3] < 0xFF)
									{
										RTC_regs[3]++;
									}
									else
									{
										RTC_regs[3] = 0;

										if ((RTC_regs[4] & 1) == 0)
										{
											RTC_regs[4] |= 1;
										}
										else
										{
											RTC_regs[4] &= 0xFE;
											RTC_regs[4] |= 0x80;
										}
									}
								}
								else
								{
									RTC_regs[2] &= 0x1F;
								}
							}
							else
							{
								RTC_regs[1] &= 0x3F;
							}
						}
						else
						{
							RTC_regs[0] &= 0x3F;
						}
					}
				}
			}
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = int_saver(ROM_bank, saver);
			saver = int_saver(RAM_bank, saver);
			saver = bool_saver(RAM_enable, saver);
			saver = int_saver(ROM_mask, saver);
			saver = int_saver(RAM_mask, saver);
			saver = byte_saver([] RTC_regs = new saver = byte_saver([5], saver);
			saver = byte_saver([] RTC_regs_latch = new saver = byte_saver([5], saver);
			saver = bool_saver(RTC_regs_latch_wr, saver);
			saver = int_saver(RTC_timer, saver);
			saver = int_saver(RTC_low_clock, saver);
			saver = bool_saver(halt, saver);
			saver = int_saver(RTC_offset, saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = int_loader(&ROM_bank, loader);
			loader = int_loader(&RAM_bank, loader);
			loader = bool_loader(&RAM_enable, loader);
			loader = int_loader(&ROM_mask, loader);
			loader = int_loader(&RAM_mask, loader);
			loader = byte_loader(&[] RTC_regs = new loader = byte_loader(&[5], loader);
			loader = byte_loader(&[] RTC_regs_latch = new loader = byte_loader(&[5], loader);
			loader = bool_loader(&RTC_regs_latch_wr, loader);
			loader = int_loader(&RTC_timer, loader);
			loader = int_loader(&RTC_low_clock, loader);
			loader = bool_loader(&halt, loader);
			loader = int_loader(&RTC_offset, loader);

			return loader;
		}
	};
}
