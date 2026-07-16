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

	void APU::OnExecFetch(uint16_t addr)
	{
		
	}

	void APU::WriteMemory(uint32_t addr, uint8_t value)
	{

	}
}