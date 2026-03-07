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
		uint8_t ret;

		if (addr >= 0x8000)
		{
			// easy optimization, since rom reads are so common, move this up (reordering the rest of these else ifs is not easy)
			ret = mapper_pntr->ReadPRG(addr - 0x8000);
		}
		else if (addr < 0x0800)
		{
			ret = RAM[addr];
		}
		else if (addr < 0x2000)
		{
			ret = RAM[addr & 0x7FF];
		}
		else if (addr < 0x4000)
		{
			ret = ppu_ReadReg(addr & 7);
		}
		else if (addr < 0x4020)
		{
			ret = ReadReg(addr);
		}
		else if (addr < 0x6000)
		{
			ret = mapper_pntr->ReadExp(addr - 0x4000);
		}
		else
		{
			ret = mapper_pntr->ReadWRAM(addr - 0x6000);
		}

		if (addr != 0x4015)
		{
			// This register is internal to the CPU and so the external CPU data bus is disconnected when reading it.
			// Therefore the returned value cannot be seen by external devices and the value does not affect open bus.
			DB = ret;
		}

		return ret;
	}

	uint8_t SNES_System::ReadMemoryDMA(uint32_t addr)
	{
		uint8_t ret;
		uint8_t ret2;

		// if the cpu address bus in the apu register range, they are activated
		// all unmapped memory in the range 0x4000 - 0x4FFF maps to the registers
		// otherwise, they are inactive and DMA cannot see them
		if ((address_bus >= 0x4000) && (address_bus <= 0x401F))
		{
			if (addr >= 0x8000)
			{
				// easy optimization, since rom reads are so common, move this up (reordering the rest of these else ifs is not easy)
				ret = mapper_pntr->ReadPRG(addr - 0x8000);
			}
			else if (addr < 0x0800)
			{
				ret = RAM[addr];
			}
			else if (addr < 0x2000)
			{
				ret = RAM[addr & 0x7FF];
			}
			else if (addr < 0x4000)
			{
				ret = ppu_ReadReg(addr & 7);
			}
			else if (addr < 0x4020)
			{
				ret = ReadReg(addr);
			}
			else if (addr < 0x6000)
			{
				ret = mapper_pntr->ReadExp(addr - 0x4000);
			}
			else
			{
				ret = mapper_pntr->ReadWRAM(addr - 0x6000);
			}

			// don't double clock if we already read from the exact address
			// dmc handles clocking on it's own, only read back the values
			if ((addr & 0x1F) == 0x16)
			{
				ret2 = ReadReg(0x4016);
				ret &= 0xE0;
				ret |= (ret2 & 0x1F);
			}

			if ((addr & 0x1F) == 0x17)
			{
				ret2 = ReadReg(0x4017);
				ret &= 0xE0;
				ret |= (ret2 & 0x1F);
			}

			// when $4015 is activated, the underlying read is what updates the data bus
			// however, the returned value is what is in $4015
			if (addr != 0x4015)
			{
				DB = ret;
			}

			if ((addr & 0x1F) == 0x15)
			{
				ret = ReadReg(0x4015);
			}
		}
		else
		{
			if (addr >= 0x8000)
			{
				// easy optimization, since rom reads are so common, move this up (reordering the rest of these else ifs is not easy)
				ret = mapper_pntr->ReadPRG(addr - 0x8000);
			}
			else if (addr < 0x0800)
			{
				ret = RAM[addr];
			}
			else if (addr < 0x2000)
			{
				ret = RAM[addr & 0x7FF];
			}
			else if (addr < 0x4000)
			{
				ret = ppu_ReadReg(addr & 7);
			}
			else if (addr < 0x6000)
			{
				ret = mapper_pntr->ReadExp(addr - 0x4000);
			}
			else
			{
				ret = mapper_pntr->ReadWRAM(addr - 0x6000);
			}

			DB = ret;
		}

		return ret;
	}

	uint8_t SNES_System::DummyReadMemory(uint32_t addr)
	{
		return ReadMemory(addr);
	}

	void SNES_System::OnExecFetch(uint16_t addr)
	{

	}

	void SNES_System::WriteMemory(uint32_t addr, uint8_t value)
	{
		address_bus = addr;
		
		if (addr < 0x0800)
		{			
			RAM[addr] = value;
		}
		else if (addr < 0x2000)
		{
			RAM[addr & 0x7FF] = value;
		}
		else if (addr < 0x4000)
		{
			ppu_WriteReg(addr & 7, value);
		}
		else if (addr < 0x4020)
		{
			WriteReg(addr, value);
		}
		else if (addr < 0x6000)
		{
			mapper_pntr->WriteExp(addr - 0x4000, value);
		}
		else if (addr < 0x8000)
		{
			mapper_pntr->WriteWRAM(addr - 0x6000, value);
		}
		else
		{
			mapper_pntr->WritePRG(addr - 0x8000, value);
		}

		DB = value;
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
				if (Use_APU_Test_Regs)
				{
					if (addr == 0x4018)
					{
						return (uint8_t)(((apu_Pulse_1_Sample & 0xF) << 4) | (apu_Pulse_0_Sample & 0xF));
					}
					if (addr == 0x4019)
					{
						return (uint8_t)(((apu_Noise_Sample & 0xF) << 4) | (apu_Triangle_Sample & 0xF));
					}
					if (addr == 0x401A)
					{
						ret_spec = (uint8_t)(apu_DMC_Out_Deltacounter & 0x7F);
						return (uint8_t)(apu_DMC_Out_Deltacounter & 0x7F);
					}
				}
				else
				{
					return DB;
				}

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


	uint8_t SNES_System::PeekMemory(uint32_t addr)
	{
		uint8_t ret;

		addr &= 0xFFFF;

		if (addr >= 0x4020)
		{
			//easy optimization, since rom reads are so common, move this up (reordering the rest of these elseifs is not easy)
			ret = mapper_pntr->PeekCart(addr);
		}
		else if (addr < 0x0800)
		{
			ret = RAM[addr];
		}
		else if (addr < 0x2000)
		{
			ret = RAM[addr & 0x7FF];
		}
		else if (addr < 0x4000)
		{
			ret = ppu_PeekReg(addr & 7);
		}
		else if (addr < 0x4020)
		{
			ret = PeekReg(addr); //we're not rebasing the register just to keep register names canonical
		}

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