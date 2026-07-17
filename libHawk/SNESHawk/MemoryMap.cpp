#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "SNES_System.h"

/*

*/
namespace SNESHawk
{
	#pragma region Memory Map

	uint8_t SNES_System::ReadMemory_Lo_ROM(uint32_t addr)
	{		
		uint8_t ret = 0;
		
		return ret;
	}

	uint8_t SNES_System::ReadMemory_Hi_ROM(uint32_t addr)
	{
		uint8_t ret = 0;

		return ret;
	}

	uint8_t SNES_System::ReadMemory_Ex_Hi_ROM(uint32_t addr)
	{
		uint8_t ret = 0;

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

	void SNES_System::WriteMemory_Lo_ROM(uint32_t addr, uint8_t value)
	{

	}

	void SNES_System::WriteMemory_Hi_ROM(uint32_t addr, uint8_t value)
	{

	}

	void SNES_System::WriteMemory_Ex_Hi_ROM(uint32_t addr, uint8_t value)
	{

	}

	uint8_t SNES_System::ReadReg(uint32_t addr)
	{
		uint8_t ret_spec;
		switch (addr)
		{
			case 0x4000:
			case 0x4001:
			case 0x4002:
			case 0x4003:
			case 0x4004:
			case 0x4005:
			case 0x4006:
			case 0x4007:
			case 0x4008:
			case 0x4009:
			case 0x400A:
			case 0x400B:
			case 0x400C:
			case 0x400D:
			case 0x400E:
			case 0x400F:
			case 0x4010:
			case 0x4011:
			case 0x4012:
			case 0x4013:
				return DB;
				//return apu.ReadReg(addr);
			case 0x4014: /*OAM DMA*/ break;
			case 0x4015: return (uint8_t)((uint8_t)(apu_ReadReg(addr) & 0xDF) + (uint8_t)(DB & 0x20));
			case 0x4016:
				// don't clock controllers from reads on consectuive cycles
				// this includes from DMC DMA
				if ((TotalExecutedCycles != (Last_Controller_Poll_1 + 1)) && (TotalExecutedCycles != Last_Controller_Poll_1))
				{
					ret_spec = read_joyport(addr);
					Previous_Controller_Latch_1 = ret_spec;
				}
				else
				{
					ret_spec = Previous_Controller_Latch_1;
				}

				Last_Controller_Poll_1 = TotalExecutedCycles;

				return ret_spec;

			case 0x4017:
				// don't clock controllers from reads on consectuive cycles
				if ((TotalExecutedCycles != (Last_Controller_Poll_2 + 1)) && (TotalExecutedCycles != Last_Controller_Poll_2))
				{
					ret_spec = read_joyport(addr);
					Previous_Controller_Latch_2 = ret_spec;
				}
				else
				{
					ret_spec = Previous_Controller_Latch_2;
				}

				Last_Controller_Poll_2 = TotalExecutedCycles;

				return ret_spec;

			case 0x4018:
			case 0x4019:
			case 0x401A:
				return DB;

			default:
				//Console.WriteLine("read register: {0:x4}", addr);
				break;

		}

		return DB;
	}

	uint8_t SNES_System::read_joyport(uint32_t addr)
	{
		uint8_t ret;
		ret = ReadController(addr == 0x4016);

		ret &= 0x1F;
		ret |= (uint8_t)(0xE0 & DB);
		return ret;
	}

	void SNES_System::WriteReg(uint32_t addr, uint8_t val)
	{
		switch (addr)
		{
			case 0x4000:
			case 0x4001:
			case 0x4002:
			case 0x4003:
			case 0x4004:
			case 0x4005:
			case 0x4006:
			case 0x4007:
			case 0x4008:
			case 0x4009:
			case 0x400A:
			case 0x400B:
			case 0x400C:
			case 0x400D:
			case 0x400E:
			case 0x400F:
			case 0x4010:
			case 0x4011:
			case 0x4012:
			case 0x4013:
				apu_WriteReg(addr, val);
				break;
			case 0x4014:
				//schedule a sprite dma event for beginning 1 cycle in the future.
				//this receives 2 because that's just the way it works out.
				oam_dma_addr = (uint16_t)(val << 8);
				sprdma_countdown = 1;

				if (sprdma_countdown > 0)
				{
					sprdma_countdown--;
					if (sprdma_countdown == 0)
					{
						if (!apu_Get_Cycle)
						{
							cpu_deadcounter = 2;
						}
						else
						{
							cpu_deadcounter = 1;
						}
						OAM_DMA_Exec = true;

						if (TraceCallback) TraceCallback(3);

						RDY = false;
						oam_dma_index = 0;
					}
				}
				break;
			case 0x4015: apu_WriteReg(addr, val); break;
			case 0x4016:
				//write_joyport(val);
				Controller_Strobed = true;
				Controller_Strobed_Value = val;
				break;
			case 0x4017: apu_WriteReg(addr, val); break;
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


	uint8_t SNES_System::PeekMemory_Lo_ROM(uint32_t addr)
	{
		uint8_t ret = 0;

		return ret;
	}

	uint8_t SNES_System::PeekMemory_Hi_ROM(uint32_t addr)
	{
		uint8_t ret = 0;

		return ret;
	}

	uint8_t SNES_System::PeekMemory_Ex_Hi_ROM(uint32_t addr)
	{
		uint8_t ret = 0;

		return ret;
	}

	uint8_t SNES_System::PeekReg(uint32_t addr)
	{
		switch (addr)
		{
			case 0x4000:
			case 0x4001:
			case 0x4002:
			case 0x4003:
			case 0x4004:
			case 0x4005:
			case 0x4006:
			case 0x4007:
			case 0x4008:
			case 0x4009:
			case 0x400A:
			case 0x400B:
			case 0x400C:
			case 0x400D:
			case 0x400E:
			case 0x400F:
			case 0x4010:
			case 0x4011:
			case 0x4012:
			case 0x4013:
				return apu_PeekReg(addr);
			case 0x4014: /*OAM DMA*/ break;
			case 0x4015: return apu_PeekReg(addr);
			case 0x4016:
			case 0x4017:
				return peek_joyport(addr);
			default:
				//Console.WriteLine("read register: {0:x4}", addr);
				break;
		}
		return 0xDB;
	}

	uint16_t SNES_System::Peek_Memory_16(uint32_t addr)
	{
		uint16_t ret = 0;

		ret = (this->*PeekMemory)(addr);

		ret |= (uint16_t)((this->*PeekMemory)(++addr) << 8);
		
		return ret;
	}

	uint16_t SNES_System::Peek_Memory_8_Branch(uint32_t addr)
	{
		int16_t ret = 0;

		ret = (int16_t)(this->*PeekMemory)(addr);

		if ((ret & 0x80) == 0x80)
		{
			ret |= 0xFF00;
		}

		return (uint16_t)((int16_t)addr + ret + 1);
	}

	#pragma endregion
}