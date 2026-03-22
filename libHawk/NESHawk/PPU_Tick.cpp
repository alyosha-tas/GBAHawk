#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "NES_System.h"
#include "Mappers.h"

// Note:

/* 
	Frames start on the first cycle of VBL, except for when the ppu is first powering up

	todo: need to check timing of sprite size 16 in sprite analysis

	todo: redo timing so there are fewer individual checks of PPUON
*/

namespace NESHawk
{
	void NES_System::TickPPU(bool do_single_tick)
	{
		bool tick_done = false;
		while (!tick_done)
		{		
			if (status_sl == 240)
			{
				if (PPUON() && (status_cycle == 0))
				{
					// here the address bus temporarily has the PT fetch address on it
					ppu_VRAM_Address = get_ptread(ppu_BG_NT_Addr);
					mapper_pntr->AddressPPU(ppu_VRAM_Address);
				}

				if (status_cycle == 1)
				{
					// Now Reg_v gets put onto the bus
					ppu_VRAM_Address = ppu_Reg_v;
					mapper_pntr->AddressPPU(ppu_VRAM_Address);
				}
			}
			
			if (status_sl == 241)
			{
				if (status_cycle == 0)
				{
					ppu_Reg2002_vblank_active_pending = true;

					// These things happen at the start of every frame
					ppuphase = PPU_PHASE_VBL;
				}
				else if (status_cycle == 1)
				{
					if (ppu_Reg2002_vblank_active_pending)
					{
						ppu_Reg2002_vblank_active = true;
						ppu_Reg2002_vblank_active_pending = false;
					}

					nmi_destiny = ppu_Vblank_NMI_Gen && ppu_Reg2002_vblank_active;
					if (cpu_stepcounter == 2) { NMI_offset = 1; }
					else if (cpu_stepcounter == 1) { NMI_offset = 2; }
					else { NMI_offset = 0; }
				}
				else if ((status_cycle <= 3) && nmi_destiny)
				{
					nmi_destiny &= ppu_Vblank_NMI_Gen && ppu_Reg2002_vblank_active;
				}
				else if (status_cycle == (4 + NMI_offset))
				{
					if (nmi_destiny) { NMI = true; }
					mapper_pntr->AtVsyncNmi();
				}
			}

			if (status_sl == 261)
			{
				if (status_cycle == 0)
				{
					ppu_Reg2002_vblank_clear_pending = true;
					ppu_IdleSynch ^= true;

					ppu_Reg2002_objhit = ppu_Reg2002_objoverflow = false;
				}
				else if (status_cycle == 1)
				{				
					if (ppu_Reg2002_vblank_clear_pending)
					{
						ppu_Reg2002_vblank_active = false;
						ppu_Reg2002_vblank_clear_pending = false;
					}
				}
				else if (status_cycle == 339)
				{
					evenOddDestiny = PPUON();
				}

				if ((status_cycle >= 280) && (status_cycle <= 304) && PPUON())
				{
					ppu_Install_v_Latches();
				}
			}

			// rendering
			if ((status_sl < 240) || (status_sl == 261))
			{
				if (status_cycle == 0)
				{
					// here the address bus temporarily has the PT fetch address on it
					// note that it could be missed if the dot is skipped
					if (status_sl != 261)
					{
						if (PPUON())
						{
							ppu_VRAM_Address = get_ptread(ppu_BG_NT_Addr);
							mapper_pntr->AddressPPU(ppu_VRAM_Address);
						}
					}
				}			
				else if (status_cycle <= 256)
				{
					if (status_sl != 261)
					{
						/////////////////////////////////////////////
						// Sprite Evaluation Start
						/////////////////////////////////////////////
						if (PPUON())
						{
							if (sprite_eval_cycle < 64)
							{
								// the first 64 cycles of each scanline are used to initialize sceondary OAM 
								// the actual effect setting a flag that always returns 0xFF from a OAM read
								// this is a bit of a shortcut to save some instructions
								// data is read from OAM as normal but never used
								if ((sprite_eval_cycle & 1) == 1)
								{
									soam[soam_index] = 0xFF;
									soam_index++;
								}

								sprite_zero_in_range = false;
							}
							// otherwise, scan through OAM and test if sprites are in range
							// if they are, they get copied to the secondary OAM 
							else
							{
								if (sprite_eval_cycle == 64)
								{
									soam_index = 0;
									oam_index = reg_2003;
								}

								if (oam_index >= 256)
								{
									oam_index &= 0xFF;
									sprite_eval_write = false;
								}

								if ((sprite_eval_cycle & 1) == 0)
								{
									read_value = OAM[oam_index];
								}
								else if (sprite_eval_write)
								{
									//look for sprites 
									if (spr_true_count == 0 && soam_index < 8)
									{
										soam[soam_index * 4] = read_value;
									}

									if (soam_index < 8)
									{
										if (yp >= read_value && yp < read_value + spriteHeight && spr_true_count == 0)
										{
											//a flag gets set if sprite zero is in range
											if (sprite_eval_cycle == 65)
											{
												sprite_zero_in_range = true;
											}

											spr_true_count++;
											oam_index++;
										}
										else if (spr_true_count > 0 && spr_true_count < 4)
										{
											soam[soam_index * 4 + spr_true_count] = read_value;

											oam_index++;
											spr_true_count++;

											if (spr_true_count == 4)
											{
												// glitchy check of x (interpretted as y)
												if (yp < read_value || yp >= read_value + spriteHeight)
												{
													oam_index &= 0xFFC;
												}

												soam_index++;
												spr_true_count = 0;
											}
										}
										else
										{
											oam_index += 4;
											oam_index &= 0xFFC;
										}
									}
									else
									{
										if (yp >= read_value && yp < read_value + spriteHeight && PPUON())
										{
											ppu_Reg2002_objoverflow = true;
										}

										if (yp >= read_value && yp < read_value + spriteHeight && spr_true_count == 0)
										{
											spr_true_count++;
											oam_index++;
										}
										else if (spr_true_count > 0 && spr_true_count < 4)
										{
											oam_index++;

											spr_true_count++;
											if (spr_true_count == 4)
											{
												// glitchy check of x (interpretted as y)
												if (yp < read_value || yp >= read_value + spriteHeight)
												{
													oam_index &= 0xFFC;
												}

												soam_index++;
												spr_true_count = 0;
											}
										}
										else
										{
											if (soam_index == 8)
											{
												// glitchy increments
												oam_index += 1;

												if ((oam_index & 3) != 0)
												{
													oam_index += 4;
												}
											}
											else
											{
												oam_index += 4;
												oam_index &= 0xFFC;
											}

										}

										read_value = soam[0]; //writes change to reads
									}
								}
								else
								{
									// if we don't write sprites anymore, just scan through the oam
									if (soam_index * 4 + spr_true_count < 32)
									{
										read_value = soam[soam_index * 4 + spr_true_count]; //writes change to reads
									}
									else
									{
										read_value = soam[0]; //writes change to reads
									}

									oam_index += 4;

									oam_index &= 0xFFC;
								}
							}
						}
						/////////////////////////////////////////////
						// Sprite Evaluation End
						/////////////////////////////////////////////

						int pixel = 0;
						int pixelcolor = PALRAM[pixel];

						Read_bgdata(xp, xt + 2);

						//according to qeed's doc, use palette 0 or $2006's value if it is & 0x3Fxx
						//at one point I commented this out to fix bottom-left garbage in DW4. but it's needed for full_nes_palette. 
						//solution is to only run when PPU is actually OFF (left-suppression doesnt count)
						if (!PPUON())
						{
							// if there's anything wrong with how we're doing this, someone please chime in
							int addr = ppu_Reg_v;
							if ((addr & 0x3F00) == 0x3F00)
							{
								pixel = addr & 0x1F;
							}

							pixelcolor = PALRAM[pixel];
						}

						uint8_t spixel = 0;
						uint8_t sattr = 0;
						bool is_spr_0 = false;
						
						// comparing always happens, but shifting doesn't happen unless ppu is on
						for (int i = 0; i < 8; i++)
						{
							if (ppu_Sprite_Shifters[i].X == ppu_Sprite_Draw_Cycle)
							{
								ppu_Sprite_Shifters[i].X_Start = true;
							}
						}

						//generate the BG and sprite data
						if (PPUON())
						{							
							if (show_bg_new && (xt > 0 || ppu_Show_BG_Leftmost))
							{
								pixel = ((ppu_BG_Pattern_0 >> ppu_Scroll_x) & 1) | (((ppu_BG_Pattern_1 >> ppu_Scroll_x) & 1) << 1);
								if (pixel != 0)
									pixel |= ppu_BG_Attr[0];
								pixelcolor = PALRAM[pixel];
							}

							ppu_BG_Pattern_0 >>= 1;
							ppu_BG_Pattern_1 >>= 1;

							// shift in 1's in high table
							ppu_BG_Pattern_1 |= 0x8000;

							//check if the pixel has a sprite in it
							for (int i = 0; i < 8; i++)
							{
								if (ppu_Sprite_Shifters[i].X_Start)
								{
									// only send data for lowest indexed sprite
									if (spixel == 0)
									{
										spixel = (ppu_Sprite_Shifters[i].Pattern_0 & 1);
										spixel |= ((ppu_Sprite_Shifters[i].Pattern_1 & 1) << 1);

										sattr = ppu_Sprite_Shifters[i].Attr;
									}

									// but update shifters regardless
									ppu_Sprite_Shifters[i].Pattern_0 >>= 1;
									ppu_Sprite_Shifters[i].Pattern_1 >>= 1;
								}

								if ((i == 0) && (spixel != 0))
								{
									is_spr_0 = true;
								}
							}
						}

						ppu_Sprite_Draw_Cycle += 1;

						if (ppu_was_on)
						{
							if ((xp + ppu_Scroll_x) == 7)
							{
								ppu_BG_Attr[0] = ppu_BG_Attr[1];
							}
							
							if (xp == 7)
							{
								ppu_BG_Pattern_0 &= 0xFF;
								ppu_BG_Pattern_1 &= 0xFF;

								ppu_BG_Pattern_0 |= ((uint16_t)BitReverse(ppu_Next_BG_Pt_0) << 8);
								ppu_BG_Pattern_1 |= ((uint16_t)BitReverse(ppu_Next_BG_Pt_1) << 8);

								ppu_BG_Attr[1] = ppu_BG_Attr[2];
							}				
						}

						if (!Settings_DispBackground)
							pixelcolor = 0;

						ppu_Sprite_Draw_Glitch = false;

						if (spixel != 0 && show_obj_new && (xt > 0 || ppu_Show_OBJ_Leftmost))
						{				
							//TODO - make sure we don't trigger spritehit if the edges are masked for either BG or OBJ
							//spritehit:
							//1. is it sprite#0?
							//2. is the bg pixel nonzero?
							//then, it is spritehit.
							ppu_Reg2002_objhit |= (sprite_zero_go && is_spr_0 && pixel != 0 && ppu_Raster_Pos < 255 && show_bg_new && show_obj_new);

							//priority handling, if in front of BG:
							bool drawsprite = !(((sattr & 0x20) != 0) && ((pixel & 3) != 0));
							if (drawsprite && Settings_DispSprites)
							{
								//bring in the palette bits and palettize
								spixel |= (sattr & 3) << 2;
								//save it for use in the framebuffer
								pixelcolor = PALRAM[0x10 + spixel];
							}
						}

						if (xp == 7 && PPUON())
						{
							ppu_Increment_hsc();

							if ((status_cycle == 256) && ppu_was_on)
							{
								ppu_Increment_vs();
							}

							if (status_cycle == 256) { race_2006 = true; }
						}

						pipeline(pixelcolor, xt * 8 + xp);
						target++;

						// end of visible part of the scanline
						sprite_eval_cycle++;
						xp++;
						ppu_Raster_Pos++;

						if (xp == 8)
						{
							xp = 0;
							xt++;

							xstart = xt << 3;
							target = yp_shift + xstart;
							ppu_Raster_Pos = xstart;

							spriteHeight = ppu_OBJ_Size_16 ? 16 : 8;
						}

						if (status_cycle > 64)
						{
							if (ppu_was_on && !PPUON())
							{
								//Console.WriteLine("oam addr glitch " + status_sl + " " + status_cycle);
								reg_2003++;
							}
						}
					}
					else
					{					
						// a glitchy version of sprite evaluation happens on the pre-render line
						// but nothing much happens here
						if (PPUON() && (status_cycle <= 65))
						{
							sprite_zero_in_range = false;
						}
					
						// if scanline is the pre-render line, we just read BG data
						Read_bgdata(xp, xt + 2);

						if (xp == 7 && PPUON())
						{
							ppu_Increment_hsc();

							if ((status_cycle == 256) && ppu_was_on)
							{
								ppu_Increment_vs();
							}

							if (status_cycle == 256) { race_2006 = true; }
						}

						sprite_eval_cycle++;
						xp++;

						if (xp == 8)
						{
							xp = 0;
							xt++;
						}
					}
				}
				else if (status_cycle <= 320)
				{
					// after we are done with the visible part of the frame, we reach sprite transfer to temp OAM tables and such
					if (status_cycle == 257)
					{
						ppuphase = PPU_PHASE_OBJ;

						s = 0;
						soam_index = -1;
						ppu_aux_index = 0;

						last_pipeline = target > 0;

						if (PPUON()) { ppu_Install_h_Latches(); }
					}

					switch (ppu_aux_index)
					{
						case 0:
							soam_index++;
							ppu_temp_oam_y = soam[soam_index];
							read_value = ppu_temp_oam_y;

							ppu_Commit_Read = PPUON();
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address &= 0xFF;
								ppu_VRAM_Address |= (ppu_Get_NT_Read() & 0xFF00);
								ppubus_clock(ppu_VRAM_Address);
							}

							if (last_pipeline)
							{
								pipeline(0, 256);
								target++;
							}
							break;
						case 1:
							// unused nametable read
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address = ppu_Get_NT_Read();
								ppubus_read(ppu_VRAM_Address);
							}

							soam_index++;
							ppu_temp_oam_ind = soam[soam_index];
							read_value = ppu_temp_oam_ind;

							if (last_pipeline)
							{
								pipeline(0, 257);
							}

							last_pipeline = false;

							break;

						case 2:
							ppu_Commit_Read = PPUON();
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address &= 0xFF;
								ppu_VRAM_Address |= (ppu_Get_NT_Read() & 0xFF00);
								ppubus_clock(ppu_VRAM_Address);
							}
							
							soam_index++;
							ppu_Sprite_Shifters[s].Attr = (soam[soam_index] & 0xE3);
							read_value = soam[soam_index];

							spriteHeight = ppu_OBJ_Size_16 ? 16 : 8;

							// only use the bottom 8 bits of counter
							line = yp_sp - ppu_temp_oam_y;

							if ((ppu_Sprite_Shifters[s].Attr & 0x80) != 0) // vflip
								line = spriteHeight - line - 1;

							patternNumber = ppu_temp_oam_ind;

							//8x16 sprite handling:
							if (ppu_OBJ_Size_16)
							{
								int bank = (patternNumber & 1) << 12;
								patternNumber = patternNumber & ~1;
								patternNumber |= (line >> 3) & 1;
								patternAddress = (patternNumber << 4) | bank;
							}
							else
							{
								patternAddress = (patternNumber << 4) | (ppu_OBJ_Pattern_High ? 1 << 12 : 0);
							}

							//offset into the pattern for the current line.
							//tricky: tall sprites have already had lines>8 taken care of by getting a new pattern number above.
							//so we just need the line offset for the second pattern
							patternAddress += line & 7;
							break;

						case 3:
							// unused nametable read
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address = ppu_Get_NT_Read();
								ppubus_read(ppu_VRAM_Address);
							}

							soam_index++;
							ppu_Sprite_Shifters[s].X = soam[soam_index];
							read_value = (uint8_t)ppu_Sprite_Shifters[s].X;
							break;

						case 4:
							// if the PPU is off, we don't put anything on the bus
							ppu_Commit_Read = PPUON();
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address &= 0xFF;
								ppu_VRAM_Address |= (patternAddress & 0xFF00);
								ppubus_clock(ppu_VRAM_Address);
							}
							read_value = (uint8_t)ppu_Sprite_Shifters[s].X;
							break;

						case 5:
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address = patternAddress;
								ppu_Sprite_Shifters[s].Pattern_0 = ppubus_read(ppu_VRAM_Address);
							}
							read_value = (uint8_t)ppu_Sprite_Shifters[s].X;
							break;

						case 6:
							// if the PPU is off, we don't put anything on the bus
							ppu_Commit_Read = PPUON();
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address &= 0xFF;
								ppu_VRAM_Address |= (patternAddress & 0xFF00);
								ppubus_clock(ppu_VRAM_Address);
							}
							read_value = (uint8_t)ppu_Sprite_Shifters[s].X;
							break;

						case 7:
							if (ppu_Commit_Read)
							{
								ppu_VRAM_Address = patternAddress;
								ppu_VRAM_Address += 8;
								ppu_Sprite_Shifters[s].Pattern_1 = ppubus_read(ppu_VRAM_Address);
							}
							
							if (PPUON())
							{
								// hflip
								if ((ppu_Sprite_Shifters[s].Attr & 0x40) == 0)
								{
									ppu_Sprite_Shifters[s].Pattern_0 = BitReverse(ppu_Sprite_Shifters[s].Pattern_0);
									ppu_Sprite_Shifters[s].Pattern_1 = BitReverse(ppu_Sprite_Shifters[s].Pattern_1);
								}

								// also check if sprites are out of range and zero patterns if they are
								if ((yp_sp < ppu_temp_oam_y) || (yp_sp >= (ppu_temp_oam_y + spriteHeight)))
								{
									ppu_Sprite_Shifters[s].Pattern_0 = 0;
									ppu_Sprite_Shifters[s].Pattern_1 = 0;
								}
							}

							read_value = (uint8_t)ppu_Sprite_Shifters[s].X;
							break;
					}

					ppu_aux_index++;
					if (ppu_aux_index == 8)
					{
						ppu_aux_index = 0;
						s++;
					}
				}
				else
				{
					if (status_cycle <= 336)
					{
						if (status_cycle == 321)
						{
							ppuphase = PPU_PHASE_BG;
							xt = 0;
							xp = 0;

							soam_index++;
							if (soam_index == 32) { soam_index = 0; }
						}

						// read first 2 tiles for next scanline
						Read_bgdata(xp, xt);

						if (xp == 7 && PPUON())
						{
							ppu_Increment_hsc();

							if (status_cycle == 256)
								ppu_Increment_vs();

							// refill the BG shift registers here
							if (xt == 0)
							{
								ppu_BG_Attr[0] = ppu_BG_Attr[2];
								
								ppu_BG_Pattern_0 &= 0xFF;
								ppu_BG_Pattern_1 &= 0xFF;
								
								ppu_BG_Pattern_0 |= ((uint16_t)BitReverse(ppu_Next_BG_Pt_0) << 8);
								ppu_BG_Pattern_1 |= ((uint16_t)BitReverse(ppu_Next_BG_Pt_1) << 8);
							}
							else
							{
								ppu_BG_Attr[1] = ppu_BG_Attr[2];
								
								ppu_BG_Pattern_0 >>= 8;
								ppu_BG_Pattern_1 >>= 8;

								ppu_BG_Pattern_0 |= ((uint16_t)BitReverse(ppu_Next_BG_Pt_0) << 8);
								ppu_BG_Pattern_1 |= ((uint16_t)BitReverse(ppu_Next_BG_Pt_1) << 8);
							}
						}

						xp++;

						if (xp == 8)
						{
							xp = 0;
							xt++;
						}
					}
					else if (status_cycle <= 340)
					{
						Read_bgdata(xp, xt);
						xp++;
						xp &= 1;
					}
				}
			}

			ppu_was_on = PPUON();

			ppu_Run(); // note cycle ticks inside runppu

			status_cycle += 1;

			FrameCycle += 1;

			if (status_cycle == 341)
			{
				status_cycle = 0;

				status_sl += 1;

				if (status_sl == 262)
				{
					status_sl = 0;
				}

				ppu_Dot_Was_Skipped = false;

				// After memory access 170, the PPU simply rests for 4 cycles (or the
				// equivelant of half a memory access cycle) before repeating the whole
				// pixel/scanline rendering process. If the scanline being rendered is the very
				// first one on every second frame, then this delay simply doesn't exist.
				if (status_sl == 0 && ppu_IdleSynch && evenOddDestiny && ppu_Chop_Dot)
				{
					status_cycle += 1;
					ppu_Dot_Was_Skipped = true;

				
					// a glitch happens here where all sprites on scnaline zero are triggered and try to draw to pixel 0
					ppu_Sprite_Draw_Glitch = true;
				}

				// reset rendering related parameters
				if ((status_sl < 240) || (status_sl == 261))
				{
					spr_true_count = 0;
					soam_index = 0;
					oam_index = 0;
					sprite_eval_write = true;

					// reset sprite comparers
					for (int i = 0; i < 8; i++)
					{
						ppu_Sprite_Shifters[i].X_Start = false;
					}

					sprite_zero_go = sprite_zero_in_range;

					if (status_sl == 261)
					{
						yp = 0;
						yp_sp = 5;
					}
					else
					{
						yp = status_sl;
						yp_sp = status_sl;
					}
					ppuphase = PPU_PHASE_BG;
		
					if (status_sl == 261 && PPUON())
					{
						// "If PPUADDR is not less then 8 when rendering starts, the first 8 bytes in OAM are written to from 
						// the current location of PPUADDR"	
						if (reg_2003 >= 8)
						{
							for (int i = 0; i < 8; i++)
							{
								OAM[i] = OAM[(reg_2003 & 0xF8) + i];
							}
						}

						// another form of oam corruption can occur from disabling rendering midscanline
						if (ppu_Can_Corrupt)
						{
							for (int i = 0; i < 8; i++)
							{
								OAM[ppu_OAM_Corrupt_Addr * 8 + i] = OAM[i];
							}

							soam[ppu_OAM_Corrupt_Addr * 8] = soam[0];

							ppu_Can_Corrupt = false;
						}
					}

					if (NTViewCallback && (yp == NTView_Scanline)) { NTViewCallback(); }
					if (PPUViewCallback && (yp == PPUView_Scanline)) { PPUViewCallback(); }

					// set up intial values to use later
					yp_shift = yp << 8;
					xt = 0;
					xp = 0;

					sprite_eval_cycle = 0;

					xstart = xt << 3;
					target = yp_shift + xstart;
					ppu_Raster_Pos = xstart;

					spriteHeight = ppu_OBJ_Size_16 ? 16 : 8;

					ppu_Sprite_Draw_Cycle = 0;
				}

				if (status_sl == 241)
				{
					tick_done = true;

					frame_is_done = true;
				}
			}

			tick_done |= do_single_tick;
		}
	}
}