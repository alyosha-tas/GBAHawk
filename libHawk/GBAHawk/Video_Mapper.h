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
			// nothing to initialize
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
	};
}

#endif