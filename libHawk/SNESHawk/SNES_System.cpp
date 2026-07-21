#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "SNES_System.h"

/*
*	How are consecutive writes on apu handled, do old or new values win?
*	Some coincidences of write and read still need testing in APU, ex DMC sample length
*	old core had condition for length clock and halt happening at the same time, is it still needed?
*/

using namespace std;

namespace SNESHawk
{
	void SNES_System::Frame_Advance()
	{
		FrameCycle = 0;

		if (ppudead > 0)
		{
			// One full frame while ppu powering up
			NewDeadPPU(241 * 341 - start_up_offset);
		}
		else
		{
			TickPPU(false);
		}
	}

	bool SNES_System::SubFrame_Advance(uint32_t reset_cycle)
	{
		FrameCycle = 0;
		
		while (!frame_is_done)
		{
			Single_Step();

			if (reset_cycle == FrameCycle)
			{
				HardReset();
				return true;
			}

			if (controller_was_latched && !frame_is_done)
			{
				return false;
			}

			FrameCycle += 1;
		}

		return true;
	}

	void SNES_System::HardReset()
	{
		ppu_Reset();
		cpu_Reset();

		Last_Controller_Poll_1 = 0;
		Last_Controller_Poll_2 = 0;

		Previous_Controller_Latch_1 = 0;
		Previous_Controller_Latch_2 = 0;

		uint32_t startup_color = 0xFF000000;

		for (int i = 0; i < 256 * 240; i++)
		{
			video_buffer[i] = startup_color;
		}

		// need to test
		for (int i = 0; i < 0x20000; i++)
		{
			RAM[i] = 0;		
		}

		for (int i = 0; i < 0x10000; i++)
		{
			VRAM[i] = 0;
		}
	}

	void SNES_System::Single_Step()
	{
		controller_was_latched = false;
		frame_is_done = false;

		current_strobe = new_strobe;
		if (ppudead > 0)
		{
			NewDeadPPU(1);
		}
		else
		{
			TickPPU(true);
		}
	}

	void SNES_System::RunCpuOne()
	{
		if (RDY)
		{
			cpu_Fetch_Cnt++;

			if (cpu_Fetch_Cnt == cpu_Fetch_Op)
			{
				switch (cpu_Cycle_Type)
				{
					case Cycle_Type::Read_Cycle:
						alu_temp = ReadMemory(address_bus);
						break;

					case Cycle_Type::Read_Cycle_Hi:
						alu_temp_hi = ReadMemory(address_bus);
						break;

					case Cycle_Type::Write_Cycle:
						break;

					case Cycle_Type::Fetch_ALU_Cycle:
						cpu_ALU_Operation();
						// fall through to normal fetch cycle

					case Cycle_Type::Fetch_Cycle:
						iflag_pending = cpu_FlagIget();
						// fall through to no check case

					case Cycle_Type::Fetch_Cycle_No_Check:
						Fetch1();
						break;
						
					case Cycle_Type::Internal_Cycle:

						break;

					case Cycle_Type::PC_Change_Cycle:
						break;

					case Cycle_Type::Fetch_Reset:
						// do nothing as this is just a cycle that immediately goes into the interrupt handler with a reset
						ReadMemory(address_bus);
						break;
				}
			}
			else if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
			{
				cpu_Instr_Cycle++;

				cpu_Fetch_Cnt = 0;

				ExecuteOneOp();

				cpu_Fetch_Wait = cpu_Calculate_Wait_States();
			}
		}
	}
}