#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "SNES_System.h"
#include "Mappers.h"

/*

*/
namespace SNESHawk
{
	#pragma region Memory Map

	uint8_t SNES_System::ReadMemory(uint32_t addr)
	{		
		uint8_t ret = 0;
		
		if ((addr & 0x8000) == 0x8000)
		{
			ret = mapper_pntr->Read_Memory_High(addr);
		}
		else if ((addr & 0x400000) == 0)
		{
			ret = ReadReg(addr);
		}
		else
		{
			ret = mapper_pntr->Read_Memory_Low(addr);
		}
		
		return ret;
	}

	uint8_t SNES_System::ReadMemoryDMA(uint32_t addr)
	{
		uint8_t ret = 0;

		return ret;
	}

	void SNES_System::OnExecFetch(uint16_t addr)
	{

	}

	void SNES_System::WriteMemory(uint32_t addr, uint8_t value)
	{
		if ((addr & 0x8000) == 0x8000)
		{
			mapper_pntr->Write_Memory_High(addr, value);
		}
		else if ((addr & 0x400000) == 0)
		{
			WriteReg(addr, value);
		}
		else
		{
			mapper_pntr->Write_Memory_Low(addr, value);
		}
	}

	uint8_t SNES_System::ReadReg(uint32_t addr)
	{
		uint8_t ret_spec;
		switch (addr)
		{

			default:
				//Console.WriteLine("read register: {0:x4}", addr);
				break;

		}

		return Data_Bus;
	}

	uint8_t SNES_System::read_joyport(uint32_t addr)
	{
		uint8_t ret;
		ret = ReadController(addr == 0x4016);

		ret &= 0x1F;
		ret |= (uint8_t)(0xE0 & Data_Bus);
		return ret;
	}

	void SNES_System::WriteReg(uint32_t addr, uint8_t val)
	{
		switch (addr)
		{

			default:
				//Console.WriteLine("wrote register: {0:x4} = {1:x2}", addr, val);
				break;
		}
	}

	void SNES_System::write_joyport(uint8_t value)
	{
		//Message_String = "Write " + to_string(TotalExecutedCycles);
		//MessageCallback(Message_String.length());

		StrobeController(latched4016, value);
		latched4016 = value;
		new_strobe = (value & 1) > 0;
		if (current_strobe && !new_strobe)
		{
			controller_was_latched = true;
			alt_lag = false;
			Is_Lag = false;
			InputPollCallback();
		}
		current_strobe = new_strobe;
	}


	uint8_t SNES_System::peek_joyport(uint32_t addr)
	{
		// at the moment, the new system doesn't support peeks
		return 0;
	}


	uint8_t SNES_System::PeekMemory(uint32_t addr)
	{
		uint8_t ret = 0;

		if ((addr & 0x8000) == 0x8000)
		{
			ret = mapper_pntr->Peek_Memory_High(addr);
		}
		else if ((addr & 0x400000) == 0)
		{
			ret = PeekReg(addr);
		}
		else
		{
			ret = mapper_pntr->Peek_Memory_Low(addr);
		}

		return ret;
	}

	uint8_t SNES_System::PeekReg(uint32_t addr)
	{
		switch (addr)
		{

			default:
				//Console.WriteLine("read register: {0:x4}", addr);
				break;
		}
		return 0xDB;
	}

	uint16_t SNES_System::Peek_Memory_16(uint32_t addr)
	{
		uint16_t ret = 0;

		ret = PeekMemory(addr);

		ret |= (uint16_t)(PeekMemory(++addr) << 8);
		
		return ret;
	}

	uint16_t SNES_System::Peek_Memory_8_Branch(uint32_t addr)
	{
		int16_t ret = 0;

		ret = (int16_t)PeekMemory(addr);

		if ((ret & 0x80) == 0x80)
		{
			ret |= 0xFF00;
		}

		return (uint16_t)((int16_t)addr + ret + 1);
	}

	#pragma endregion
}