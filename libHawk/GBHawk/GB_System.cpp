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

	bool GB_System::SubFrame_Advance(uint32_t input_cycle)
	{
		while (Frame_Cycle < 70224)
		{
			Sub_Single_Step();

			if (input_cycle == Frame_Cycle)
			{
				Frame_Cycle += 1;
				return false;
			}

			Frame_Cycle += 1;
		}

		return true;
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
			if (dma_Start && !cpu_Halted && !cpu_Stopped) { dma_Tick(); }
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

		if (dma_Start && !cpu_Halted && !cpu_Stopped) { dma_Tick(); }
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
			if (ppu_pntr->ScanlineCallbackLine == -1)
			{
				if (ppu_pntr->ScanlineCallback)
				{
					ppu_pntr->ScanlineCallback(ppu_pntr->LCDC);
				}
			}
			
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

	inline void GB_System::Sub_Single_Step()
	{
		// These things do not change speed in GBC double speed mode
		snd_Tick();
		ppu_pntr->Tick();
		if (Use_MT) { mapper_pntr->Mapper_Tick(); }

		// These things all tick twice as fast in GBC double speed mode
		// Note that DMA is halted when the CPU is halted

		if (Double_Speed)
		{
			if (dma_Start && !cpu_Halted && !cpu_Stopped) { dma_Tick(); }
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

		if (dma_Start && !cpu_Halted && !cpu_Stopped) { dma_Tick(); }
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
			if (ppu_pntr->ScanlineCallbackLine == -1)
			{
				if (ppu_pntr->ScanlineCallback)
				{
					ppu_pntr->ScanlineCallback(ppu_pntr->LCDC);
				}
			}

			// send the image on VBlank
			Send_Video_Buffer();

			// input is not checked on vblank in subframe mode
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

	void GB_System::clear_screen_func()
	{
		if (Is_GBC)
		{
			for (int j = 0; j < 160 * 144; j++) { frame_buffer[j] = frame_buffer[j] | (0x30303 << (clear_counter * 2)); }

			clear_counter++;
			if (clear_counter == 4)
			{
				ppu_pntr->clear_screen = false;
			}
		}
		else
		{
			for (int j = 0; j < 160 * 144; j++) { frame_buffer[j] = (uint32_t)color_palette[0]; }
			ppu_pntr->clear_screen = false;
		}
	}

	void GB_System::dma_Tick()
	{
		if (dma_Clock >= 12)
		{
			dma_Bus_Control = true;
			dma_OAM_Access = false;
			if ((dma_Clock % 4) == 1)
			{
				// the cpu can't access memory during this time, but we still need the ppu to be able to.
				dma_Bus_Control = false;
				// Gekkio reports that A14 being high on DMA transfers always represent WRAM accesses
				// So transfers nominally from higher memory areas are actually still from there (i.e. FF -> DF)
				uint8_t  DMA_actual = dma_Addr;
				if (dma_Addr > 0xDF) { DMA_actual &= 0xDF; }
				dma_Byte = Read_Memory((uint16_t)((DMA_actual << 8) + dma_Inc));
				dma_Bus_Control = true;
			}
			else if ((dma_Clock % 4) == 3)
			{
				if (!ppu_pntr->HDMA_active)
				{
					OAM[dma_Inc] = dma_Byte;
				}
				else
				{
					// TODO: timing is off by one, maybe HDMA is aligned with CPU cycles
					if (((ppu_pntr->cur_DMA_dest - 1) & 0xFF) <= 0x9F)
					{
						OAM[(ppu_pntr->cur_DMA_dest - 1) & 0xFF] = ppu_pntr->HDMA_byte;
					}
				}

				if (dma_Inc < 0x9F) { dma_Inc++; }
				else { dma_Clock = 0; }
			}
		}

		dma_Clock++;

		if (dma_Clock == 5)
		{
			dma_Start = false;
			dma_Bus_Control = false;
			dma_OAM_Access = true;
		}
	}
}