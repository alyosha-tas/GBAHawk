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
}