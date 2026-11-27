#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "GB_System.h"
#include "../Common/GB_Mappers/Mappers.h"

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

		// turn off the screen so the image doesnt persist
		// but don't turn off blank_frame yet, it still needs to be true until the next VBL
		// GBC games need to clear slow enough that games that turn the screen off briefly for cinematics still look smooth
		if (clear_screen)
		{
			clear_screen_func();
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
		ppu_Tick();
		if (Use_MT) { mapper_pntr->Mapper_Tick(); }

		if (dma_Start && !cpu_Halted && !cpu_Stopped) { dma_Tick(); }
		ser_Tick();

		// check state before changes from cpu writes
		DIV_edge_old = ((tim_Divider_Reg & 0x1000) == 0x1000);

		tim_Tick();
		cpu_Execute_One();
		tim_Divider_Reg++;

		DIV_falling_edge |= DIV_edge_old & ((tim_Divider_Reg & 0x1000) == 0);

		if (delays_to_process) { process_delays(); }

		Cycle_Count++;
		Single_Cycle_Count++;

		if (In_Vblank && !In_Vblank_old)
		{
			if (ScanlineCallbackLine == -1)
			{
				if (ScanlineCallback)
				{
					ScanlineCallback(LCDC);
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

		In_Vblank_old = In_Vblank;
	}

	inline void GB_System::Sub_Single_Step()
	{
		// These things do not change speed in GBC double speed mode
		snd_Tick();
		ppu_Tick();
		if (Use_MT) { mapper_pntr->Mapper_Tick(); }

		if (dma_Start && !cpu_Halted && !cpu_Stopped) { dma_Tick(); }
		ser_Tick();

		// check state before changes from cpu writes
		DIV_edge_old =((tim_Divider_Reg & 0x1000) == 0x1000);

		tim_Tick();
		cpu_Execute_One();
		tim_Divider_Reg++;

		DIV_falling_edge |= DIV_edge_old & ((tim_Divider_Reg & 0x1000) == 0);

		if (delays_to_process) { process_delays(); }

		Cycle_Count++;
		Single_Cycle_Count++;

		if (In_Vblank && !In_Vblank_old)
		{
			if (ScanlineCallbackLine == -1)
			{
				if (ScanlineCallback)
				{
					ScanlineCallback(LCDC);
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

		In_Vblank_old = In_Vblank;
	}



	void GB_System::Send_Video_Buffer()
	{
		if (blank_frame)
		{
			for (int i = 0; i < 160 * 144; i++)
			{
				Video_Buffer[i] = Color_Palette[0];
			}
		}

		for (int j = 0; j < 160 * 144; j++) { Frame_Buffer[j] = (uint32_t)Video_Buffer[j]; }

		blank_frame = false;
	}

	void GB_System::clear_screen_func()
	{
		for (int j = 0; j < 160 * 144; j++) { Frame_Buffer[j] = (uint32_t)Color_Palette[0]; }
		clear_screen = false;
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
				OAM[dma_Inc] = dma_Byte;

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

	void GB_System::ppu_Tick()
	{
		// the ppu only does anything if it is turned on via bit 7 of LCDC
		if (LCDC_Bit(7))
		{
			// start the next scanline
			if (cycle == 456)
			{
				// scanline callback
				if ((LY + LY_inc) == ScanlineCallbackLine)
				{
					if (ScanlineCallback) { ScanlineCallback(LCDC); }
				}

				cycle = 0;
				LY += LY_inc;

				no_scan = false;

				if (LY == 0 && LY_inc == 0)
				{
					LY_inc = 1;
					In_Vblank = false;

					STAT &= 0xFC;

					// special note here, the y coordiate of the window is kept if the window is deactivated
					// meaning it will pick up where it left off if re-enabled later
					// so we don't reset it in the scanline loop
					window_y_tile = 0;
					window_y_latch = window_y;
					window_y_tile_inc = 0;
					window_started = false;
					if (!LCDC_Bit(5)) { window_is_reset = true; }
				}

				// Automatically restore access to VRAM at this time (force end drawing)
				// Who Framed Roger Rabbit seems to run into this.
				VRAM_access_write = true;
				VRAM_access_read = true;

				if (LY == 144)
				{
					In_Vblank = true;
					On_VBlank();
				}
			}

			// exit vblank if LCD went from off to on
			if (LCD_was_off)
			{
				//VBL_INT = false;
				In_Vblank = false;
				LCD_was_off = false;

				// we exit vblank into mode 0 for 4 cycles
				// but no hblank interrupt, presumably this only happens transitioning from mode 3 to 0
				STAT &= 0xFC;
				LY_inc = 1;

				// also the LCD doesn't turn on right away
				// also, the LCD does not enter mode 2 on scanline 0 when first turned on
				no_scan = true;
				cycle = 8;
			}

			// Timing note: LYC=143 does not by itself block mode1 stat int. But, the glitchy mode 2 stat check combined with
			// LYC=143 does block it

			// the VBL stat is continuously asserted
			if (In_Vblank)
			{
				// glitchy check of mode 2
				if (LY == 144)
				{
					if (cycle <= 4)
					{
						if (!STAT_Bit(5)) { VBL_INT = false; }
					}

					if ((cycle >= 2) && (cycle < 4))
					{
						// there is an edge case where a VBL INT is triggered if STAT bit 5 is set
						if (STAT_Bit(5)) { VBL_INT = true; }
					}

					if (cycle >= 4)
					{
						if (STAT_Bit(4)) { VBL_INT = true; }
						else { VBL_INT = false; }
					}
				}
				else
				{
					// mode 1 is asserted continuously
					if (STAT_Bit(4)) { VBL_INT = true; }
					else { VBL_INT = false; }
				}

				if ((cycle == 4) && (LY == 144))
				{
					HBL_INT = false;

					// set STAT mode to 1 (VBlank) and interrupt flag if it is enabled
					STAT &= 0xFC;
					STAT |= 0x01;

					if ((REG_FFFF & 1) == 1) { cpu_FlagI = true; }
					REG_FF0F |= 0x01;
				}

				if ((cycle == 6) && (LY == 153))
				{
					LY = 0;
					LY_inc = 0;
				}
			}
			else
			{
				if (no_scan)
				{
					// timings are slightly different if we just turned on the LCD
					// there is no mode 2  (presumably it missed the trigger)
					if (cycle < 85)
					{
						if (cycle == 8)
						{
							// clear the sprite table
							for (int k = 0; k < 10; k++)
							{
								SL_sprites[k * 4] = 0;
								SL_sprites[k * 4 + 1] = 0;
								SL_sprites[k * 4 + 2] = 0;
								SL_sprites[k * 4 + 3] = 0;
							}

							if (LY != LYC)
							{
								LYC_INT = false;
								STAT &= 0xFB;
							}

							if ((LY == LYC) && !STAT_Bit(2))
							{
								// set STAT coincidence FLAG and interrupt flag if it is enabled
								STAT |= 0x04;
								if (STAT_Bit(6)) { LYC_INT = true; }
							}
						}

						if (cycle == 84)
						{
							OAM_access_read = false;

							rendering_complete = false;
						}
					}
					else if (!rendering_complete)
					{
						if (cycle == 86)
						{
							STAT &= 0xFC;
							STAT |= 0x03;
							OAM_INT = false;

							OAM_access_write = false;
							VRAM_access_read = false;
							VRAM_access_write = false;
							rendering_complete = false;
						}

						// render the screen and handle hblank
						render(cycle - 85);
					}
				}
				else
				{
					if (cycle < 83)
					{
						if (cycle <= 4)
						{
							if ((cycle == 2) && (LY != 0))
							{
								HBL_INT = false;

								if (STAT_Bit(5)) { OAM_INT = true; }
							}

							// the last few cycles of mode 1 still trigger mode 1 int
							if (cycle < 4)
							{
								if (STAT_Bit(4) && ((STAT & 3) == 1)) { VBL_INT = true; }
								else { VBL_INT = false; }
							}
							else
							{
								// apparently, writes can make it to OAM one cycle longer then reads
								OAM_access_write = false;

								// here mode 2 will be set to true and interrupts fired if enabled
								STAT &= 0xFC;
								STAT |= 0x2;

								if (LY == 0)
								{
									VBL_INT = false;
									if (STAT_Bit(5)) { OAM_INT = true; }
								}
							}
						}

						if (cycle == 80)
						{
							OAM_access_read = false;
							OAM_access_write = true;
							VRAM_access_read = false;
							rendering_complete = false;
						}
						else if (cycle < 80)
						{
							// here OAM scanning is performed
							OAM_scan(cycle);
						}
					}
					else if (!rendering_complete)
					{
						if (cycle == 84)
						{
							STAT &= 0xFC;
							STAT |= 0x03;
							OAM_INT = false;
							OAM_access_write = false;
							VRAM_access_write = false;
						}

						// render the screen and handle hblank
						render(cycle - 83);
					}
				}
			}

			if (LY_inc == 0)
			{
				if (cycle == 10)
				{
					LYC_INT = false;
					STAT &= 0xFB;
				}
				else if (cycle == 12)
				{
					// Special case of LY = LYC
					if ((LY == LYC) && !STAT_Bit(2))
					{
						// set STAT coincidence FLAG and interrupt flag if it is enabled
						STAT |= 0x04;
						if (STAT_Bit(6)) { LYC_INT = true; }
					}
				}
			}

			// here LY=LYC will be asserted or cleared (but only if LY isnt 0 as that's a special case)
			if ((cycle == 2) && (LY != 0))
			{
				if (LY_inc == 1)
				{
					LYC_INT = false;
					STAT &= 0xFB;
				}
			}
			else if ((cycle == 4) && (LY != 0))
			{
				if ((LY == LYC) && !STAT_Bit(2))
				{
					// set STAT coincidence FLAG and interrupt flag if it is enabled
					STAT |= 0x04;
					if (STAT_Bit(6)) { LYC_INT = true; }
				}
			}

			cycle++;
		}
		else
		{
			STAT &= 0xFC;

			VBL_INT = LYC_INT = HBL_INT = OAM_INT = false;

			In_Vblank = true;

			LCD_was_off = true;

			LY = 0;

			cycle = 0;
		}

		// assert the STAT IRQ line if the line went from zero to 1
		stat_line = VBL_INT || LYC_INT || HBL_INT || OAM_INT;

		if (stat_line && !stat_line_old)
		{
			if ((REG_FFFF & 2) == 2) { cpu_FlagI = true; }
			REG_FF0F |= 0x02;
		}

		stat_line_old = stat_line;

		// process latch delays
		//latch_delay();
	}
}