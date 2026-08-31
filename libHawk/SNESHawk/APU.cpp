#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "SNES_System.h"
#include "APU.h"

namespace SNESHawk
{
	uint8_t APU::ReadMemory(uint32_t addr)
	{
		return 0;
	}
	
	uint8_t APU::DummyReadMemory(uint32_t addr)
	{
		return ReadMemory(addr);
	}

	void APU::WriteMemory(uint32_t addr, uint8_t value)
	{

	}

	uint8_t APU::PeekMemory(uint32_t addr)
	{
		uint8_t ret = 0;
		return ret;
	}

	uint16_t APU::Peek_Memory_16(uint32_t addr)
	{
		uint16_t ret = 0;

		ret = PeekMemory(addr);

		ret |= (uint16_t)(PeekMemory(++addr) << 8);

		return ret;
	}

	uint16_t APU::Peek_Memory_16_TCALL(uint32_t op)
	{
		uint16_t ret = 0;

		uint16_t addr = 0xFF00 | (0xDE - (op >> 5));

		ret = PeekMemory(addr);
		addr += 1;

		ret |= (PeekMemory(addr) << 8);

		return ret;
	}

	uint16_t APU::Peek_Memory_12(uint32_t addr)
	{
		uint16_t ret = 0;

		ret = PeekMemory(addr);

		ret |= (uint16_t)(PeekMemory(++addr) << 8);

		ret &= 0xFFF;

		return ret;
	}

	uint16_t APU::Peek_Memory_Bit(uint32_t addr)
	{
		uint16_t ret = 0;

		ret = PeekMemory(addr);

		ret &= 0x7;

		return ret;
	}

	uint16_t APU::Peek_Memory_JIND(uint32_t addr)
	{
		uint16_t ret = 0;
		uint16_t ret2 = 0;

		ret = PeekMemory(addr);

		ret |= (uint16_t)(PeekMemory(++addr) << 8);

		ret += X;

		ret2 = PeekMemory(ret);

		ret2 |= (uint16_t)(PeekMemory(++ret) << 8);

		return ret2;
	}
}