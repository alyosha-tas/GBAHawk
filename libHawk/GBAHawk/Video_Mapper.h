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
		virtual void Write_ROM_32(uint32_t addr, uint32_t value)
		{
			if (addr == 0x08800180)
			{
				Video_Command = value;

				if (Video_Command == 0x11)
				{
					// remap
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
	};
}

#endif