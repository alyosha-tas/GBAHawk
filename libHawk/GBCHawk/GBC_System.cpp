#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "GBC_System.h"
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

namespace GBCHawk
{
	void GBC_System::Frame_Advance()
	{
		Frame_Cycle = 0;
		for (int i = 0; i < 70224; i++)
		{
			Single_Step();
			Frame_Cycle += 1;
		}

		// turn off the screen so the image doesnt persist
		// but don't turn off blank_frame yet, it still needs to be true until the next VBL
		// GBCC games need to clear slow enough that games that turn the screen off briefly for cinematics still look smooth
		if (clear_screen)
		{
			clear_screen_func();
		}
	}

	bool GBC_System::SubFrame_Advance(uint32_t input_cycle)
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

	inline void GBC_System::Single_Step()
	{			
		// These things do not change speed in GBCC double speed mode
		snd_Tick();
		ppu_Tick();
		if (Use_MT) { mapper_pntr->Mapper_Tick(); }

		// These things all tick twice as fast in GBCC double speed mode
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

			Cycle_Count++;

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

	inline void GBC_System::Sub_Single_Step()
	{
		// These things do not change speed in GBCC double speed mode
		snd_Tick();
		ppu_Tick();
		if (Use_MT) { mapper_pntr->Mapper_Tick(); }

		// These things all tick twice as fast in GBCC double speed mode
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

			Cycle_Count++;

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



	void GBC_System::Send_Video_Buffer()
	{
		if (GBC_Compat)
		{
			if (!blank_frame)
			{
				for (int j = 0; j < 160 * 144; j++) { frame_buffer[j] = (uint32_t)Video_Buffer[j]; }
			}

			blank_frame = false;
		}
		else
		{
			if (blank_frame)
			{
				for (int i = 0; i < 160 * 144; i++)
				{
					Video_Buffer[i] = 0x0;
				}
			}

			for (int j = 0; j < 160 * 144; j++) { frame_buffer[j] = (uint32_t)Video_Buffer[j]; }

			blank_frame = false;
		}
	}

	void GBC_System::clear_screen_func()
	{
		for (int j = 0; j < 160 * 144; j++) { frame_buffer[j] = frame_buffer[j] | (0x30303 << (clear_counter * 2)); }

		clear_counter++;
		if (clear_counter == 4)
		{
			clear_screen = false;
		}
	}

	void GBC_System::dma_Tick()
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
				if (!HDMA_active)
				{
					OAM[dma_Inc] = dma_Byte;
				}
				else
				{
					// TODO: timing is off by one, maybe HDMA is aligned with CPU cycles
					if (((cur_DMA_dest - 1) & 0xFF) <= 0x9F)
					{
						OAM[(cur_DMA_dest - 1) & 0xFF] = HDMA_byte;
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

	void GBC_System::ppu_Tick()
	{
		// Do HDMA ticks
		if (HDMA_active && !cpu_Halted && !cpu_Stopped)
		{
			if (HDMA_length > 0)
			{
				if (!HDMA_mode)
				{
					if (HDMA_countdown > 0)
					{
						HDMA_countdown--;

						if (HDMA_countdown == (Double_Speed ? 1 : 3))
						{
							if ((Cycle_Count - cpu_Instruction_Start) == 0)
							{
								if (!HDMA_Transfer) { HDMA_Start_Stop(true); }
								VRAM_access_read_HDMA = false;
								VRAM_access_read = VRAM_access_read_PPU && VRAM_access_read_HDMA;
								VRAM_access_write_HDMA = false;
								VRAM_access_write = VRAM_access_write_PPU && VRAM_access_write_HDMA;

								// reading from open bus still returns 0xFF on DMA access, see dma_hiram_read_result_cgb04c_out1.gbc
								bus_value = 0xFF;
							}
							else
							{
								HDMA_countdown++;
							}
						}
					}
					else
					{
						// immediately transfer bytes, 2 bytes per cycles
						if ((HDMA_tick % 2) == 0)
						{
							if (HDMA_VRAM_access_glitch > 0)
							{
								HDMA_byte = Read_Memory(cpu_RegPCget());
								HDMA_VRAM_access_glitch--;
							}
							else
							{
								HDMA_byte = Read_Memory(cur_DMA_src);
							}
						}
						else
						{
							VRAM[(VRAM_Bank * 0x2000) + (cur_DMA_dest & 0x1FFF)] = HDMA_byte;

							// DMA destination address does not wrap and terminates DMA
							if (cur_DMA_dest == 0xFFFF)
							{
								HDMA_length = 0;
							}
							else
							{
								cur_DMA_dest = (uint16_t)((cur_DMA_dest + 1) & 0xFFFF);
								cur_DMA_src = (uint16_t)((cur_DMA_src + 1) & 0xFFFF);

								// similar to normal DMA, except HDMA transfers when A14 is high always access SRAM
								if (cur_DMA_src >= 0xE000) { cur_DMA_src &= 0xBFFF; }

								HDMA_length--;
							}
						}

						HDMA_tick++;
					}
				}
				else
				{
					// only transfer during mode 0, and only 16 bytes at a time
					// NOTE: state when first enabling ppu does not count as mode 0
					if (((STAT & 3) == 0) && (LY_read != last_HBL) && HBL_test && (LY_inc == 1) && !glitch_state && HDMA_can_start)
					{
						HBL_HDMA_go = true;
						HBL_test = false;
					}
					else if (HDMA_run_once)
					{
						HBL_HDMA_go = true;
						HBL_test = false;
						HDMA_run_once = false;
					}

					if (HBL_HDMA_go && (HBL_HDMA_count > 0))
					{
						if (HDMA_countdown > 0)
						{
							HDMA_countdown--;

							if (HDMA_countdown == (Double_Speed ? 1 : 3))
							{
								if ((Cycle_Count - cpu_Instruction_Start) == 0)
								{
									if (!HDMA_Transfer) { HDMA_Start_Stop(true); }
									VRAM_access_read_HDMA = false;
									VRAM_access_read = VRAM_access_read_PPU && VRAM_access_read_HDMA;
									VRAM_access_write_HDMA = false;
									VRAM_access_write = VRAM_access_write_PPU && VRAM_access_write_HDMA;

									// reading from open bus still returns 0xFF on DMA access, see dma_hiram_read_result_cgb04c_out1.gbc
									bus_value = 0xFF;

									if (LCDC_Bit(7)) { last_HBL = LY_read; }
									else { last_HBL = 0xFF; }
								}
								else
								{
									HDMA_countdown++;
								}
							}
						}
						else
						{
							if ((HDMA_tick % 2) == 0)
							{
								if (HDMA_VRAM_access_glitch > 0)
								{
									HDMA_byte = Read_Memory(cpu_RegPCget());
									HDMA_VRAM_access_glitch--;
								}
								else
								{
									HDMA_byte = Read_Memory(cur_DMA_src);
								}
							}
							else
							{
								VRAM[(VRAM_Bank * 0x2000) + (cur_DMA_dest & 0x1FFF)] = HDMA_byte;

								// DMA destination address does not wrap and terminates DMA
								if (cur_DMA_dest == 0xFFFF)
								{
									HDMA_length = 0;
								}
								else
								{
									cur_DMA_dest = (uint16_t)((cur_DMA_dest + 1) & 0xFFFF);
									cur_DMA_src = (uint16_t)((cur_DMA_src + 1) & 0xFFFF);

									// similar to normal DMA, except HDMA transfers when A14 is high always access SRAM
									if (cur_DMA_src >= 0xE000) { cur_DMA_src &= 0xBFFF; }

									HDMA_length--;
									HBL_HDMA_count--;
								}
							}

							if ((HBL_HDMA_count == 0) && (HDMA_length != 0))
							{
								HBL_test = true;
								HBL_HDMA_count = 0x10;
								HBL_HDMA_go = false;
								HDMA_countdown = Double_Speed ? 2 : 4;
							}

							HDMA_tick++;
						}
					}
					else
					{
						if (HDMA_Transfer) { HDMA_Start_Stop(false); }
						VRAM_access_read_HDMA = true;
						VRAM_access_read = VRAM_access_read_PPU && VRAM_access_read_HDMA;
						VRAM_access_write_HDMA = true;
						VRAM_access_write = VRAM_access_write_PPU && VRAM_access_write_HDMA;
					}
				}
			}
			else
			{
				HDMA_active = false;
				if (HDMA_Transfer) { HDMA_Start_Stop(false); }
				VRAM_access_read_HDMA = true;
				VRAM_access_read = VRAM_access_read_PPU && VRAM_access_read_HDMA;
				VRAM_access_write_HDMA = true;
				VRAM_access_write = VRAM_access_write_PPU && VRAM_access_write_HDMA;
			}
		}

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

					//STAT &= 0xFC;

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
				VRAM_access_read_PPU = true;
				VRAM_access_read = VRAM_access_read_PPU && VRAM_access_read_HDMA;
				VRAM_access_write_PPU = true;
				VRAM_access_write = VRAM_access_write_PPU && VRAM_access_write_HDMA;


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
				glitch_state = true;
				LY_inc = 1;

				// also the LCD doesn't turn on right away
				// also, the LCD does not enter mode 2 on scanline 0 when first turned on
				no_scan = true;

				// turning on the ppu seems to take one cycle longer in double speed mode, maybe because the 8 MHz clock is too fast
				cycle = Double_Speed ? 7 : 8;
				//cycle = 8;
			}

			if (In_Vblank)
			{
				if (cycle == 4)
				{
					// Timing note: LYC interrupt cannot block mode 1 stat by itself
					// But, the glitchy mode 2 stat check combined with LYC does block it. So adjust old stat line here
					stat_line_old = VBL_INT || HBL_INT || OAM_INT;

					if (LY == 144) { HBL_INT = false; }
				}

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
					// set STAT mode to 1 (VBlank) and interrupt flag if it is enabled
					STAT &= 0xFC;
					STAT |= 0x01;

					if ((REG_FFFF & 1) == 1) { cpu_FlagI = true; }
					REG_FF0F |= 0x01;
				}

				if ((cycle == 8) && (LY == 153))
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
							glitch_state = false;

							OAM_access_write = false;
							VRAM_access_read_PPU = false;
							VRAM_access_read = VRAM_access_read_PPU && VRAM_access_read_HDMA;
							VRAM_access_write_PPU = false;
							VRAM_access_write = VRAM_access_write_PPU && VRAM_access_write_HDMA;
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
							VRAM_access_read_PPU = false;
							VRAM_access_read = VRAM_access_read_PPU && VRAM_access_read_HDMA;
							VRAM_access_write_PPU = false;
							VRAM_access_write = VRAM_access_write_PPU && VRAM_access_write_HDMA;
						}

						// render the screen and handle hblank
						render(cycle - 83);
					}
				}
			}

			if (LY_inc == 0)
			{
				if (cycle == LY_153_change)
				{
					LY_read = LY;
				}
				else if (cycle == (10 + 1 * LYC_offset))
				{
					LYC_INT = false;
					STAT &= 0xFB;
				}
				else if (cycle == (10 + 2 * LYC_offset))
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
			// it is also the boundary where HDMA can no longe start if triggered by a write
			if ((cycle == 2) && (LY != 0))
			{
				LY_read = LY;
				HDMA_can_start = false;
			}
			else if ((cycle == (2 + 1 * LYC_offset)) && (LY != 0))
			{
				if (LY_inc == 1)
				{
					LYC_INT = false;
					STAT &= 0xFB;
				}
			}
			else if ((cycle == (2 + 2 * LYC_offset)) && (LY != 0))
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

			VBL_INT = HBL_INT = OAM_INT = false;

			In_Vblank = true;

			LCD_was_off = true;

			LY = 0;
			LY_read = 0;

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

		if (LYC_cd > 0)
		{
			LYC_cd--;
			if (LYC_cd == 0)
			{
				LYC = LYC_t;

				if (LCDC_Bit(7))
				{
					if (LY != LYC) { STAT &= 0xFB; LYC_INT = false; }
					else { STAT |= 0x4; LYC_INT = true; }
				}
			}
		}

		LCDC_Bit_4_glitch = false;
	}
}