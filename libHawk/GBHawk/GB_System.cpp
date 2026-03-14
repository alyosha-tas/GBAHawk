#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "GB_System.h"
#include "Mappers.h"
#include "PPUs.h"

/*
	$10000000-$FFFFFFFF    Unmapped
	$0E010000-$0FFFFFFF    Unmapped
	$0E000000-$0E00FFFF    SRAM
	$0C000000-$0DFFFFFF    ROM - Wait State 2
	$0A000000-$0BFFFFFF    ROM - Wait State 1
	$08000000-$09FFFFFF    ROM - Wait State 0
	$07000400-$07FFFFFF    Unmapped
	$07000000-$070003FF    OAM
	$06018000-$06FFFFFF    Unmapped
	$06000000-$06017FFF    VRAM
	$05000400-$05FFFFFF    Unmapped
	$05000000-$050003FF    Palette RAM
	$04XX0800-$04XX0800    Mem Control
	$04000000-$040007FF    I/O Regs
	$03008000-$03FFFFFF    Unmapped
	$03000000-$03007FFF    IWRAM
	$02040000-$02FFFFFF    Unmapped
	$02000000-$0203FFFF    WRAM
	$00004000-$01FFFFFF    Unmapped
	$00000000-$00003FFF    BIOS
*/

using namespace std;

namespace GBHawk
{
	void GB_System::Frame_Advance()
	{
		Frame_Cycle = 0;
		for (int i = 0; i < 70224; i++)
		{
			Single_Step();
			Frame_Cycle += 1;
		}
	}

	bool GB_System::SubFrame_Advance(uint32_t reset_cycle)
	{
		Frame_Cycle = 0;
		
		while (!VBlank_Rise)
		{
			Single_Step();

			if (reset_cycle == Frame_Cycle)
			{
				return true;
			}

			Frame_Cycle += 1;
		}

		return false;
	}

	inline void GB_System::Single_Step()
	{			
		// These things do not change speed in GBC double speed mode
		snd_Tick();
		ppu_pntr->Tick();
		if (Use_MT) { mapper_pntr->Mapper_Tick(); }

		// These things all tick twice as fast in GBC double speed mode
		// Note that DMA is halted when the CPU is halted

		if (Double_Speed)
		{
			if (ppu_pntr->DMA_start && !cpu_Halted && !cpu_Stopped) { ppu_pntr->DMA_tick(); }
			ser_Tick();

			// check state before changes from cpu writes
			DIV_edge_old = (tim_Divider_Reg & 0x2000) == 0x2000;

			tim_Tick();
			cpu_Execute_One();
			tim_Divider_Reg++;

			DIV_falling_edge |= DIV_edge_old & ((tim_Divider_Reg & 0x2000) == 0);

			if (delays_to_process) { process_delays(); }

			REG_FF0F_OLD = REG_FF0F;
		}

		if (ppu_pntr->DMA_start && !cpu_Halted && !cpu_Stopped) { ppu_pntr->DMA_tick(); }
		ser_Tick();

		// check state before changes from cpu writes
		DIV_edge_old = Double_Speed ? ((tim_Divider_Reg & 0x2000) == 0x2000) : ((tim_Divider_Reg & 0x1000) == 0x1000);

		tim_Tick();
		cpu_Execute_One();
		tim_Divider_Reg++;

		DIV_falling_edge |= DIV_edge_old & (Double_Speed ? ((tim_Divider_Reg & 0x2000) == 0) : ((tim_Divider_Reg & 0x1000) == 0));

		if (delays_to_process) { process_delays(); }

		Cycle_Count++;

		if (ppu_pntr->In_Vblank && !In_Vblank_old)
		{
			Is_Lag = false;

			controller_was_checked = true;

			// update the controller state on VBlank
			Get_Controller_State();

			// check if controller state caused interrupt
			do_controller_check();

			// send the image on VBlank
			Send_Video_Buffer();

			if (Sync_Domains_VBL)
			{
				Sync_Domains_VBL_Func();
			}
		}

		REG_FF0F_OLD = REG_FF0F;

		In_Vblank_old = ppu_pntr->In_Vblank;
	}

	void GB_System::Send_Video_Buffer()
	{
		if (GBC_Compat)
		{
			if (!ppu_pntr->blank_frame)
			{
				for (int j = 0; j < 160 * 144; j++) { frame_buffer[j] = (uint32_t)video_buffer[j]; }
			}

			ppu_pntr->blank_frame = false;
		}
		else
		{
			if (ppu_pntr->blank_frame)
			{
				for (int i = 0; i < 160 * 144; i++)
				{
					video_buffer[i] = color_palette[0];
				}
			}

			for (int j = 0; j < 160 * 144; j++) { frame_buffer[j] = (uint32_t)video_buffer[j]; }

			ppu_pntr->blank_frame = false;
		}
	}
}