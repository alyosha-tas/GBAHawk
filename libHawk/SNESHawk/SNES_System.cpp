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
		// apu has some specific power up bahaviour that we will emulate here
		apu_Reset();
		apu_HardReset();

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
		/////////////////////////////
		// apu tick start
		/////////////////////////////

		apu_Sequencer_Tick();
		apu_Pulse_0_Run();
		apu_Pulse_1_Run();
		apu_Triangle_Run();
		apu_Noise_Run();
		apu_DMC_Run();


		/////////////////////////////
		// apu tick end
		/////////////////////////////

		
		/////////////////////////////
		// OAM DMA start
		/////////////////////////////

		if (OAM_DMA_Exec && (apu_DMC_DMA_Countdown != 1) && !dmc_realign)
		{
			if (cpu_deadcounter == 0)
			{
				if (oam_dma_index % 2 == 0)
				{
					oam_dma_byte = ReadMemoryDMA(oam_dma_addr);
					oam_dma_addr++;
				}
				else
				{
					WriteMemory(0x2004, oam_dma_byte);
				}
				oam_dma_index++;
				if (oam_dma_index == 512)
				{
					OAM_DMA_Exec = false;
				}
			}
			else
			{
				cpu_deadcounter--;
			}
		}

		dmc_realign = false;

		/////////////////////////////
		// OAM DMA end
		/////////////////////////////


		/////////////////////////////
		// dmc dma start
		/////////////////////////////

		if (apu_DMC_DMA_Countdown > 0)
		{
			if (apu_DMC_DMA_Countdown == 1)
			{
				dmc_realign = true;
			}

			// By this point the cpu should be frozen, if it is not, then we are in a multi-write opcode, add another cycle delay
			if (apu_DMC_DMA_Countdown == apu_DMC_RDY_Check)
			{
				if (!RDY && !RDY_Freeze)
				{
					apu_DMC_DMA_Countdown += 2;
				}
				else
				{
					// setting up the address clocks certain registers, when the DMA changes it, it is like reading from the adress
					if (((address_bus & 0xF007) == 0x2007) || ((address_bus & 0xF007) == 0x3007))
					{
						// double read intended
						read_2007();
						read_2007();
					}

					if (((address_bus & 0xF007) == 0x2002) || ((address_bus & 0xF007) == 0x3002))
					{
						read_2002();
					}

					if (address_bus == 0x4015)
					{
						apu_ReadReg(address_bus);
					}

					// controllers get an extra clock under certain circumstances
					// these registers are always active when cpu is in the range 0x4000 - 0x401F inclusive
					if ((address_bus >= 0x4000) && (address_bus <= 0x401F))
					{
						if ((address_bus & 0x1F) == 0x16)
						{
							ReadReg(address_bus);
						}

						if ((address_bus & 0x1F) == 0x17)
						{
							ReadReg(address_bus);
						}
					}
				}
			}

			RDY = false;
			DMC_DMA_Exec = true;
			apu_DMC_DMA_Countdown--;

			// only runs for one cycle and aborted
			if (apu_DMC_Fill_Glitch_2)
			{
				apu_DMC_Fill_Glitch_2 = false;
				DMC_DMA_Exec = false;
				apu_DMC_DMA_Countdown = -1;
				apu_DMC_Sample_Length = 0;
			}

			if (apu_DMC_DMA_Countdown == 0)
			{
				apu_DMC_Fetch();

				// DCM was disabled during DMA
				if (apu_DMC_Fill_Glitch_3)
				{
					apu_DMC_Sample_Length = 0;
					apu_DMC_Fill_Glitch_3 = false;
					apu_DMC_Length_Countdown = 0;
				}

				// Buffer was filled at the same time it was emptied (revision dependent)
				if ((apu_DMC_Timer == 1) && (apu_DMC_Out_Bits_Remaining == 0))
				{
					apu_DMC_Fill_Glitch_1 = true;
				}

				// internally the length isn't recognized as zero in time to abort an automatic reload DMA
				if ((apu_DMC_Timer == 3) && (apu_DMC_Out_Bits_Remaining == 0) && (apu_DMC_Sample_Length == 1) && !apu_DMC_Loop_Flag)
				{
					apu_DMC_Fill_Glitch_2 = true;
				}

				DMC_DMA_Exec = false;
				apu_DMC_DMA_Countdown = -1;
			}
		}

		/////////////////////////////
		// dmc dma end
		/////////////////////////////
			
		_irq_apu = apu_Sequencer_IRQ || apu_DMC_IRQ;

		IRQ = _irq_apu;

		ExecuteOne();

		// take care of any delayed apu writes, this assumes old values win
		if (apu_Write_Countdown > 0)
		{
			apu_Write_Countdown--;
			if (apu_Write_Countdown == 0)
			{
				apu_Internal_WriteReg(apu_Write_Address, apu_Write_Value);
			}
		}

		apu_Len_Clock_Active = false;

		// Get an audio sample and mix in any external audio sources
		int32_t s = apu_EmitSample();

		if (s != old_s)
		{
			apu_Audio_Samples[apu_Audio_Num_Samples * 2] = apu_Audio_Sample_Clock;
			apu_Audio_Samples[apu_Audio_Num_Samples * 2 + 1] = s - old_s;

			apu_Audio_Num_Samples += 1;

			old_s = s;
		}

		apu_Audio_Sample_Clock++;

		if (apu_Get_Cycle)
		{
			if (Controller_Strobed)
			{
				write_joyport(Controller_Strobed_Value);
				Controller_Strobed = false;
			}
		}
		else
		{

		}

		if (!RDY && !DMC_DMA_Exec && !OAM_DMA_Exec)
		{
			RDY = true;
		}

		apu_Get_Cycle ^= true;
	}
}