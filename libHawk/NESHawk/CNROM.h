#pragma once

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

#include "Mappers.h"

using namespace std;

namespace NESHawk
{
	class Mapper_CNROM : public Mappers
	{
	public:

		void Reset()
		{
			CHR_Bank = 0;

			CHR_Mask = (*Core_CHR_ROM_Length >> 13) - 1;
		}

		void WritePRG(uint32_t addr, uint8_t value)
		{
			if (Bus_Conflicts)
			{
				value &= ReadPRG(addr);
			}

			CHR_Bank = value & CHR_Mask;
		}

		uint8_t ReadPPU(uint32_t addr)
		{
			if (addr < 0x2000)
			{
				return Core_CHR_ROM[addr + (CHR_Bank << 13)];
			}

			uint32_t ofs = addr & 0x3FF;
			uint32_t block = (addr >> 10) & 3;

			return Core_CIRAM[block][ofs];
		}
	};
}