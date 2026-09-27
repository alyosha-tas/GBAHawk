#ifndef Video_MAPPERS_H
#define Video_MAPPERS_H

#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace GBAHawk
{
	class Mapper_Video : public Mappers
	{
	public:

		void Reset()
		{
			Video_Bank_Start_Address = 0;
			Video_ROM_Space_Address = 0;
			Video_Banks_to_Map = 0;
			Video_Command = 0;

			// repeatedly map in the first bank, except first
			for (int i = 8; i < 8; i++)
			{
				Video_Banks[i] = 0;
			}

			for (int i = 8; i < 0x4000; i++)
			{
				Video_Banks[i] = i-7;
			}

			Load_Mapping();
		}

		uint8_t Read_Memory_8(uint32_t addr)
		{
			return 0xFF; // nothing mapped here
		}

		uint16_t Read_Memory_16(uint32_t addr)
		{
			return 0xFFFF; // nothing mapped here
		}

		uint32_t Read_Memory_32(uint32_t addr)
		{
			return 0xFFFFFFFF; // nothing mapped here
		}

		uint8_t PeekMemory(uint32_t addr)
		{
			return Read_Memory_8(addr);
		}

		// For now assume only 32 bit accesses work
		void Write_ROM_32(uint32_t addr, uint32_t value)
		{
			uint32_t cur_ROM_Space_addr;
			uint32_t cur_Bank_Start_addr;
			
			if (addr == 0x08800180)
			{
				Video_Command = value;

				if ((Video_Command == 0x11) || (Video_Command == 0x01))
				{
					// remap
					cur_Bank_Start_addr = (Video_Bank_Start_Address & 0x3FFFFFF) >> 9;
					cur_ROM_Space_addr = ((Video_ROM_Space_Address - 0x8000000) >> 9) & 0x3FFF;

					for (int i = 0; i < Video_Banks_to_Map; i++)
					{
						Video_Banks[cur_ROM_Space_addr] = cur_Bank_Start_addr;
					
						for (int j = 0; j < 512; j++)
						{
							Core_ROM[cur_ROM_Space_addr * 512 + j] = Core_Video_ROM[Video_Banks[cur_ROM_Space_addr] * 512 + j];
						}

						cur_ROM_Space_addr += 1;
						cur_Bank_Start_addr += 1;

						cur_ROM_Space_addr &= 0x3FFF;
						cur_Bank_Start_addr &= 0x1FFFF;
					}
				}
			}
			else if (addr == 0x08800184)
			{
				Video_Bank_Start_Address = value;
			}
			else if (addr == 0x08800188)
			{
				Video_ROM_Space_Address = value;
			}
			else if (addr == 0x0880018C)
			{
				Video_Banks_to_Map = value;
			}
		}

		void Write_ROM_16(uint32_t addr, uint16_t value)
		{
			uint32_t cur_ROM_Space_addr;
			uint32_t cur_Bank_Start_addr;

			if (addr == 0x08800180)
			{
				Video_Command = value;

				if ((Video_Command == 0x11) || (Video_Command == 0x01))
				{
					// remap
					cur_Bank_Start_addr = (Video_Bank_Start_Address & 0x3FFFFFF) >> 9;
					cur_ROM_Space_addr = ((Video_ROM_Space_Address - 0x8000000) >> 9) & 0x3FFF;

					for (int i = 0; i < Video_Banks_to_Map; i++)
					{
						Video_Banks[cur_ROM_Space_addr] = cur_Bank_Start_addr;

						for (int j = 0; j < 512; j++)
						{
							Core_ROM[cur_ROM_Space_addr * 512 + j] = Core_Video_ROM[Video_Banks[cur_ROM_Space_addr] * 512 + j];
						}

						cur_ROM_Space_addr += 1;
						cur_Bank_Start_addr += 1;

						cur_ROM_Space_addr &= 0x3FFF;
						cur_Bank_Start_addr &= 0x1FFFF;
					}
				}
			}
			else if (addr == 0x08800184)
			{
				Video_Bank_Start_Address &= 0xFFFF0000;
				Video_Bank_Start_Address |= value;
			}
			else if (addr == 0x08800188)
			{
				Video_ROM_Space_Address &= 0xFFFF0000;
				Video_ROM_Space_Address |= value;
			}
			else if (addr == 0x0880018C)
			{
				Video_Banks_to_Map &= 0xFFFF0000;
				Video_Banks_to_Map |= value;
			}
		}

		void Load_Mapping()
		{
			if (Core_Video_ROM != nullptr)
			{
				for (int i = 0; i < 0x4000; i++)
				{
					for (int j = 0; j < 512; j++)
					{
						Core_ROM[i * 512 + j] = Core_Video_ROM[Video_Banks[i] * 512 + j];
					}
				}
			}
		}
	};
}

#endif