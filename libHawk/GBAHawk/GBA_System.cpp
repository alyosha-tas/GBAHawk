#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "Memory.h"
#include "GBA_System.h"
#include "Mappers.h"

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

namespace GBAHawk
{
	void GBA_System::Frame_Advance()
	{
		FrameCycle = 0;
		while (!VBlank_Rise)
		{
			Single_Step();
			FrameCycle += 1;
		}
	}

	bool GBA_System::SubFrame_Advance(uint32_t reset_cycle)
	{
		FrameCycle = 0;
		
		while (!VBlank_Rise)
		{
			Single_Step();

			if (reset_cycle == FrameCycle)
			{
				return true;
			}

			FrameCycle += 1;
		}

		return false;
	}

	inline void GBA_System::Single_Step()
	{			
		INT_Flags_Use = INT_Flags_Gather;

		INT_Flags_Use |= INT_Flags;

		INT_Flags_Gather = 0;

		#pragma region Delays

		if (delays_to_process)
		{
			if (IRQ_Delays)
			{
				if (IRQ_Write_Delay)
				{
					// check if any remaining interrupts are still valid
					if (INT_Master_On)
					{
						if ((INT_EN & INT_Flags_Use & 0x3FFF) != 0)
						{
							cpu_Next_IRQ_Input_3 = true;
						}
						else
						{
							cpu_Next_IRQ_Input_3 = false;
						}
					}
					else
					{
						cpu_Next_IRQ_Input_3 = false;
					}

					// halting does not depend on master enable
					if ((INT_EN & INT_Flags_Use & 0x3FFF) == 0)
					{
						cpu_Trigger_Unhalt_4 = false;
					}
					else
					{
						cpu_Trigger_Unhalt_4 = true;
					}

					INT_Flags = INT_Flags_Use;

					IRQ_Write_Delay = false;

					// check if all delay sources are false
					if (!Halt_Enter && !Halt_Leave)
					{
						IRQ_Delays = false;

						if (!ppu_Delays && !Misc_Delays && !ppu_Sprite_Delays && !IRQ_Delays)
						{
							delays_to_process = false;
						}
					}
				}

				if (Halt_Enter)
				{
					if (Halt_Enter_cd > 0)
					{
						Halt_Enter_cd -= 1;

						if (Halt_Enter_cd == 0)
						{
							if (cpu_Instr_Type == cpu_Pause_For_DMA)
							{
								Halt_Held_CPU_Instr = dma_Held_CPU_Instr;
								dma_Held_CPU_Instr = cpu_Internal_Halted;
							}
							else
							{
								Halt_Held_CPU_Instr = cpu_Instr_Type;
								cpu_Instr_Type = cpu_Internal_Halted;
							}

							if (TraceCallback) TraceCallback(4);

							Halt_Enter = false;

							// check if all delay sources are false
							if (!IRQ_Write_Delay && !Halt_Leave)
							{
								IRQ_Delays = false;

								if (!ppu_Delays && !Misc_Delays && !ppu_Sprite_Delays)
								{
									delays_to_process = false;
								}
							}
						}
					}
				}

				if (Halt_Leave)
				{
					if (Halt_Leave_cd > 0)
					{
						Halt_Leave_cd -= 1;

						if (Halt_Leave_cd == 0)
						{
							if (cpu_Instr_Type == cpu_Pause_For_DMA)
							{
								dma_Held_CPU_Instr = Halt_Held_CPU_Instr;
							}
							else
							{
								cpu_Instr_Type = Halt_Held_CPU_Instr;
							}

							Halt_Leave = false;

							// check if all delay sources are false
							if (!IRQ_Write_Delay && !Halt_Enter)
							{
								IRQ_Delays = false;

								if (!ppu_Delays && !Misc_Delays && !ppu_Sprite_Delays)
								{
									delays_to_process = false;
								}
							}
						}
					}
				}
			}

			if (Misc_Delays)
			{
				if (VRAM_32_Delay)
				{
					if (VRAM_32_Check)
					{
						// always write first 16 bits when not blocked
						if (!ppu_VRAM_Access)
						{
							// Forced Align
							VRAM_32W_Addr &= 0xFFFFFFFC;

							if ((VRAM_32W_Addr & 0x00010000) == 0x00010000)
							{
								// mirrors behave differently depending on mode
								if ((VRAM_32W_Addr & 0x00008000) == 0x00008000)
								{
									if ((ppu_BG_Mode < 3) || ((VRAM_32W_Addr & 0x00004000) == 0x00004000))
									{
										VRAM_16[(VRAM_32W_Addr & 0x17FFF) >> 1] = (uint16_t)(VRAM_32W_Value & 0xFFFF);
									}
								}
								else
								{
									VRAM_16[(VRAM_32W_Addr & 0x17FFF) >> 1] = (uint16_t)(VRAM_32W_Value & 0xFFFF);
								}
							}
							else
							{
								VRAM_16[(VRAM_32W_Addr & 0xFFFF) >> 1] = (uint16_t)(VRAM_32W_Value & 0xFFFF);
							}
						}
					}
					else
					{
						VRAM_32_Delay = false;

						// check if all delay sources are false
						if (!PALRAM_32_Delay && !FIFO_DMA_A_Delay && !FIFO_DMA_B_Delay && !DMA_Any_IRQ && !DMA_Any_Start)
						{
							Misc_Delays = false;
						}
					}
				}

				if (PALRAM_32_Delay)
				{
					if (PALRAM_32_Check)
					{
						// always write first 16 bits when not blocked
						if (!ppu_PALRAM_Access)
						{
							// Forced Align
							PALRAM_32W_Addr &= 0xFFFFFFFC;

							PALRAM_16[(PALRAM_32W_Addr & 0x3FF) >> 1] = (uint16_t)(PALRAM_32W_Value & 0xFFFF);
						}
					}
					else
					{
						PALRAM_32_Delay = false;

						// check if all delay sources are false
						if (!VRAM_32_Delay && !FIFO_DMA_A_Delay && !FIFO_DMA_B_Delay && !DMA_Any_IRQ && !DMA_Any_Start)
						{
							Misc_Delays = false;
						}
					}
				}

				if (FIFO_DMA_A_Delay)
				{
					FIFO_DMA_A_cd--;

					if (FIFO_DMA_A_cd == 0)
					{
						if (dma_Go[1]) { dma_Run[1] = true; dma_All_Off = false; }

						FIFO_DMA_A_Delay = false;

						if (!FIFO_DMA_B_Delay && !VRAM_32_Delay && !PALRAM_32_Delay && !DMA_Any_IRQ && !DMA_Any_Start)
						{
							Misc_Delays = false;
						}
					}
				}

				if (FIFO_DMA_B_Delay)
				{
					FIFO_DMA_B_cd--;

					if (FIFO_DMA_B_cd == 0)
					{
						if (dma_Go[2]) { dma_Run[2] = true; dma_All_Off = false; }

						FIFO_DMA_B_Delay = false;

						if (!FIFO_DMA_A_Delay && !VRAM_32_Delay && !PALRAM_32_Delay && !DMA_Any_IRQ && !DMA_Any_Start)
						{
							Misc_Delays = false;
						}
					}
				}

				if (DMA_Any_IRQ)
				{
					for (int i = 0; i < 4; i++)
					{
						if (DMA_IRQ_Delay[i])
						{
							Trigger_IRQ((uint16_t)(8 + i));
							DMA_IRQ_Delay[i] = false;
						}
					}

					if (!DMA_IRQ_Delay[0] && !DMA_IRQ_Delay[1] && !DMA_IRQ_Delay[2] && !DMA_IRQ_Delay[3])
					{
						DMA_Any_IRQ = false;

						if (!FIFO_DMA_A_Delay && !FIFO_DMA_B_Delay && !VRAM_32_Delay && !PALRAM_32_Delay && !DMA_Any_Start)
						{
							Misc_Delays = false;
						}
					}
				}

				if (DMA_Any_Start)
				{
					for (int i = 0; i < 4; i++)
					{
						if (DMA_Start_Delay[i])
						{
							dma_Run_En_Time[i] -= 1;

							if (dma_Run_En_Time[i] == 0)
							{
								dma_Run[i] = dma_Go[i];
								if (dma_Run[i]) { dma_All_Off = false; }
								DMA_Start_Delay[i] = false;
							}
						}
					}

					if (!DMA_Start_Delay[0] && !DMA_Start_Delay[1] && !DMA_Start_Delay[2] && !DMA_Start_Delay[3])
					{
						DMA_Any_Start = false;

						if (!FIFO_DMA_A_Delay && !FIFO_DMA_B_Delay && !VRAM_32_Delay && !PALRAM_32_Delay && !DMA_Any_IRQ)
						{
							Misc_Delays = false;
						}
					}
				}

				if (!Misc_Delays && !ppu_Delays && !IRQ_Delays && !ppu_Sprite_Delays)
				{
					delays_to_process = false;
				}
			}

			if (ppu_Delays)
			{
				if (ppu_VBL_IRQ_cd > 0)
				{
					ppu_VBL_IRQ_cd -= 1;

					if (ppu_VBL_IRQ_cd == 3)
					{
						if ((ppu_STAT & 0x8) == 0x8) { Trigger_IRQ(0); }
					}
					else if (ppu_VBL_IRQ_cd == 1)
					{
						// trigger any DMAs with VBlank as a start condition
						if (dma_Go[0] && dma_Start_VBL[0]) { dma_Run[0] = true; dma_All_Off = false; }
						if (dma_Go[1] && dma_Start_VBL[1]) { dma_Run[1] = true; dma_All_Off = false; }
						if (dma_Go[2] && dma_Start_VBL[2]) { dma_Run[2] = true; dma_All_Off = false; }
						if (dma_Go[3] && dma_Start_VBL[3]) { dma_Run[3] = true; dma_All_Off = false; }
					}
					else if (ppu_VBL_IRQ_cd == 0)
					{
						// check for any additional ppu delays
						if ((ppu_HBL_IRQ_cd == 0) && (ppu_LYC_IRQ_cd == 0) && (ppu_LYC_Vid_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				if (ppu_HBL_IRQ_cd > 0)
				{
					ppu_HBL_IRQ_cd -= 1;

					if (ppu_HBL_IRQ_cd == 3)
					{
						if ((ppu_STAT & 0x10) == 0x10) { Trigger_IRQ(1); }
					}
					else if (ppu_HBL_IRQ_cd == 1)
					{
						// trigger any DMAs with HBlank as a start condition
						// but not if in vblank
						if (ppu_LY < 160)
						{
							if (dma_Go[0] && dma_Start_HBL[0]) { dma_Run[0] = true; dma_All_Off = false; }
							if (dma_Go[1] && dma_Start_HBL[1]) { dma_Run[1] = true; dma_All_Off = false; }
							if (dma_Go[2] && dma_Start_HBL[2]) { dma_Run[2] = true; dma_All_Off = false; }
							if (dma_Go[3] && dma_Start_HBL[3]) { dma_Run[3] = true; dma_All_Off = false; }
						}
					}
					else if (ppu_HBL_IRQ_cd == 0)
					{
						// check for any additional ppu delays
						if ((ppu_VBL_IRQ_cd == 0) && (ppu_LYC_IRQ_cd == 0) && (ppu_LYC_Vid_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				if (ppu_LYC_IRQ_cd > 0)
				{
					ppu_LYC_IRQ_cd -= 1;

					if (ppu_LYC_IRQ_cd == 3)
					{
						if ((ppu_STAT & 0x20) == 0x20) { Trigger_IRQ(2); }
					}
					else if (ppu_LYC_IRQ_cd == 0)
					{
						// check for any additional ppu delays
						if ((ppu_VBL_IRQ_cd == 0) && (ppu_HBL_IRQ_cd == 0) && (ppu_LYC_Vid_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				if (ppu_LYC_Vid_Check_cd > 0)
				{
					ppu_LYC_Vid_Check_cd -= 1;

					if (ppu_LYC_Vid_Check_cd == 5)
					{
						if (ppu_LY == ppu_LYC)
						{
							ppu_LYC_IRQ_cd = 4;
							ppu_Delays = true;
							delays_to_process = true;

							// set the flag bit
							ppu_STAT |= 4;
						}
					}
					else if (ppu_LYC_Vid_Check_cd == 4)
					{
						// latch rotation and scaling XY here
						if (ppu_LY < 160)
						{
							if ((ppu_LY == 0) || ppu_BG_Ref_LY_Change[2])
							{
								if (ppu_BG_Mosaic[2])
								{
									ppu_Base_LY_2 = ppu_MOS_BG_Y[ppu_LY];
								}
								else
								{
									ppu_Base_LY_2 = ppu_LY;
								}

								ppu_Current_Ref_Y_2 = ppu_BG_Ref_Y[2];
								if ((ppu_Current_Ref_Y_2 & 0x8000000) != 0)
								{
									ppu_Current_Ref_Y_2 |= 0xFFFFFFFFF0000000;
								}
							}

							if ((ppu_LY == 0) || ppu_BG_Ref_LY_Change[3])
							{
								if (ppu_BG_Mosaic[3])
								{
									ppu_Base_LY_3 = ppu_MOS_BG_Y[ppu_LY];
								}
								else
								{
									ppu_Base_LY_3 = ppu_LY;
								}

								ppu_Current_Ref_Y_3 = ppu_BG_Ref_Y[3];
								if ((ppu_Current_Ref_Y_3 & 0x8000000) != 0)
								{
									ppu_Current_Ref_Y_3 |= 0xFFFFFFFFF0000000;
								}
							}

							if ((ppu_LY == 0) || ppu_BG_Ref_X_Change[2])
							{
								ppu_Current_Ref_X_2 = ppu_BG_Ref_X[2];
								if ((ppu_Current_Ref_X_2 & 0x8000000) != 0)
								{
									ppu_Current_Ref_X_2 |= 0xFFFFFFFFF0000000;
								}
							}

							if ((ppu_LY == 0) || ppu_BG_Ref_X_Change[3])
							{
								ppu_Current_Ref_X_3 = ppu_BG_Ref_X[3];
								if ((ppu_Current_Ref_X_3 & 0x8000000) != 0)
								{
									ppu_Current_Ref_X_3 |= 0xFFFFFFFFF0000000;
								}
							}

							ppu_BG_Ref_X_Change[2] = ppu_BG_Ref_X_Change[3] = false;
							ppu_BG_Ref_LY_Change[2] = ppu_BG_Ref_LY_Change[3] = false;

							ppu_BG_Mosaic_X_Mod = ppu_BG_Mosaic_X;

							if (!ppu_Forced_Blank)
							{
								ppu_Rendering_Complete = false;
								ppu_PAL_Rendering_Complete = false;

								for (int i = 0; i < 4; i++)
								{
									ppu_BG_X_Latch[i] = (uint16_t)(ppu_BG_X[i] & 0xFFF8);
									ppu_BG_Y_Latch[i] = ppu_BG_Y[i];

									ppu_Fetch_Count[i] = 0;

									ppu_Scroll_Cycle[i] = 0;

									ppu_Pixel_Color[i] = 0;

									ppu_BG_Has_Pixel[i] = false;
								}

								if (ppu_BG_Mode <= 1)
								{
									ppu_BG_Start_Time[0] = (uint16_t)(32 - 4 * (ppu_BG_X[0] & 0x7));
									ppu_BG_Start_Time[1] = (uint16_t)(32 - 4 * (ppu_BG_X[1] & 0x7));

									ppu_BG_Rendering_Complete[0] = !ppu_BG_On[0];
									ppu_BG_Rendering_Complete[1] = !ppu_BG_On[1];

									if (ppu_BG_Mode == 0)
									{
										ppu_BG_Start_Time[2] = (uint16_t)(32 - 4 * (ppu_BG_X[2] & 0x7));
										ppu_BG_Start_Time[3] = (uint16_t)(32 - 4 * (ppu_BG_X[3] & 0x7));

										ppu_BG_Rendering_Complete[2] = !ppu_BG_On[2];
										ppu_BG_Rendering_Complete[3] = !ppu_BG_On[3];
									}
									else
									{
										ppu_BG_Start_Time[2] = 32;

										ppu_BG_Rendering_Complete[2] = !ppu_BG_On[2];
										ppu_BG_Rendering_Complete[3] = true;
									}
								}
								else
								{
									ppu_BG_Rendering_Complete[0] = true;
									ppu_BG_Rendering_Complete[1] = true;
									ppu_BG_Rendering_Complete[2] = !ppu_BG_On[2];
									ppu_BG_Rendering_Complete[3] = true;

									ppu_BG_Start_Time[2] = 32;

									if (ppu_BG_Mode == 2)
									{
										ppu_BG_Start_Time[3] = 32;

										ppu_BG_Rendering_Complete[3] = !ppu_BG_On[3];
									}
								}
							}
						}
					}
					else if (ppu_LYC_Vid_Check_cd == 0)
					{
						if (ppu_LY < 160)
						{
							// Latch B-D rotation scaling parameters here.
							ppu_F_Rot_B_2 = ppu_BG_Rot_B[2];
							ppu_F_Rot_B_3 = ppu_BG_Rot_B[3];
							ppu_F_Rot_D_2 = ppu_BG_Rot_D[2];
							ppu_F_Rot_D_3 = ppu_BG_Rot_D[3];

							if ((ppu_F_Rot_B_2 & 0x8000) != 0) { ppu_F_Rot_B_2 |= 0xFFFFFFFFFFFF0000; }
							if ((ppu_F_Rot_B_3 & 0x8000) != 0) { ppu_F_Rot_B_3 |= 0xFFFFFFFFFFFF0000; }
							if ((ppu_F_Rot_D_2 & 0x8000) != 0) { ppu_F_Rot_D_2 |= 0xFFFFFFFFFFFF0000; }
							if ((ppu_F_Rot_D_3 & 0x8000) != 0) { ppu_F_Rot_D_3 |= 0xFFFFFFFFFFFF0000; }

							if (ppu_BG_Mosaic[2])
							{
								if (ppu_MOS_BG_Y[ppu_LY] != ppu_Base_LY_2)
								{
									for (int i = 0; i < ppu_BG_Mosaic_Y; i++)
									{
										ppu_Current_Ref_X_2 += ppu_F_Rot_B_2;
										ppu_Current_Ref_Y_2 += ppu_F_Rot_D_2;
									}

									ppu_Base_LY_2 = ppu_MOS_BG_Y[ppu_LY];
								}
							}
							else if (ppu_LY != ppu_Base_LY_2)
							{
								ppu_Current_Ref_X_2 += ppu_F_Rot_B_2;
								ppu_Current_Ref_Y_2 += ppu_F_Rot_D_2;
							}

							if (ppu_BG_Mosaic[3])
							{
								if (ppu_MOS_BG_Y[ppu_LY] != ppu_Base_LY_3)
								{
									for (int i = 0; i < ppu_BG_Mosaic_Y; i++)
									{
										ppu_Current_Ref_X_3 += ppu_F_Rot_B_3;
										ppu_Current_Ref_Y_3 += ppu_F_Rot_D_3;
									}

									ppu_Base_LY_3 = ppu_MOS_BG_Y[ppu_LY];
								}
							}
							else if (ppu_LY != ppu_Base_LY_3)
							{
								ppu_Current_Ref_X_3 += ppu_F_Rot_B_3;
								ppu_Current_Ref_Y_3 += ppu_F_Rot_D_3;
							}
						}
							
						// video capture DMA, check timing
						if (dma_Go[3] && dma_Start_Snd_Vid[3])
						{
							// only starts on scanline 2
							if (ppu_LY == 2)
							{
								if (!dma_Video_DMA_Delay)
								{
									dma_Video_DMA_Start = true;
								}						
							}

							if ((ppu_LY >= 2) && (ppu_LY < 162) && dma_Video_DMA_Start)
							{
								dma_Run[3] = true;
								dma_All_Off = false;
							}

							if (ppu_LY == 162)
							{
								dma_Video_DMA_Start = false;
								dma_Video_DMA_Delay = false;
							}
						}

						// check for any additional ppu delays
						if ((ppu_VBL_IRQ_cd == 0) && (ppu_HBL_IRQ_cd == 0) && (ppu_LYC_IRQ_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				// check if all delay sources are false
				if (!ppu_Delays)
				{
					if (!Misc_Delays && !IRQ_Delays && !ppu_Sprite_Delays)
					{
						delays_to_process = false;
					}
				}
			}

			if (ppu_Sprite_Delays)
			{
				if (ppu_Sprite_Delay_SL)
				{
					ppu_Sprite_cd -= 1;

					if (ppu_Sprite_cd == 0)
					{
						ppu_Fetch_OAM_0 = true;
						ppu_Fetch_OAM_2 = false;
						ppu_Fetch_OAM_A_D = false;
						ppu_Fetch_Sprite_VRAM = false;

						ppu_Sprite_Next_Fetch = 3;

						ppu_Current_Sprite = 0;
						ppu_New_Sprite = true;

						if (ppu_Sprite_ofst_eval == 0)
						{
							ppu_Sprite_ofst_eval = 240;
							ppu_Sprite_ofst_draw = 0;
						}
						else
						{
							ppu_Sprite_ofst_eval = 0;
							ppu_Sprite_ofst_draw = 240;
						}

						ppu_Sprite_Eval_Finished = false;

						if ((ppu_LY < 159) || (ppu_LY == 227))
						{
							ppu_Sprite_LY_Check = (uint8_t)(ppu_LY + 1);

							if (ppu_LY == 227)
							{
								ppu_Sprite_LY_Check = 0;
								ppu_Sprite_Mosaic_Y_Counter = 0;
								ppu_Sprite_Mosaic_Y_Compare = 0;
							}
							else
							{
								ppu_Sprite_Mosaic_Y_Counter++;

								if (ppu_Sprite_Mosaic_Y_Counter == ppu_OBJ_Mosaic_Y)
								{
									ppu_Sprite_Mosaic_Y_Compare = (uint32_t)ppu_LY + 1;
									ppu_Sprite_Mosaic_Y_Counter = 0;
								}
								else if (ppu_Sprite_Mosaic_Y_Counter == 16)
								{
									ppu_Sprite_Mosaic_Y_Counter = 0;
								}
							}
						}

						// reset obj window detection for the scanline
						for (int i = ppu_Sprite_ofst_eval; i < (240 + ppu_Sprite_ofst_eval); i++)
						{
							ppu_Sprite_Pixels[i] = 0;
							ppu_Sprite_Priority[i] = 3;
							ppu_Sprite_Pixel_Occupied[i] = false;
							ppu_Sprite_Semi_Transparent[i] = false;
							ppu_Sprite_Object_Window[i] = false;
							ppu_Sprite_Is_Mosaic[i] = false;
						}

						ppu_Sprite_Render_Cycle = 0;

						ppu_Sprite_Delay_SL = false;

						if (!ppu_Sprite_Delay_Disp)
						{
							ppu_Sprite_Delays = false;

							if (!ppu_Delays && !Misc_Delays && !IRQ_Delays)
							{
								delays_to_process = false;
							}
						}

						// reset latches
						ppu_Sprite_Pixel_Latch = 0;
						ppu_Sprite_Priority_Latch = 0;

						ppu_Sprite_Semi_Transparent_Latch = false;
						ppu_Sprite_Mosaic_Latch = false;
						ppu_Sprite_Pixel_Occupied_Latch = false;
					}
				}

				if (ppu_Sprite_Delay_Disp)
				{
					ppu_Sprite_Disp_cd -= 1;

					if (ppu_Sprite_Disp_cd == 0)
					{
						if ((ppu_CTRL & 0x1000) == 0)
						{
							ppu_OBJ_On_Disp = false;
						}

						ppu_OBJ_On = (ppu_CTRL & 0x1000) == 0x1000;

						ppu_Sprite_Delay_Disp = false;

						if (!ppu_Sprite_Delay_SL)
						{
							ppu_Sprite_Delays = false;

							if (!ppu_Delays && !Misc_Delays && !IRQ_Delays)
							{
								delays_to_process = false;
							}
						}
					}
				}
			}
		}
		#pragma endregion

		#pragma region Audio

		snd_Divider++;
		if ((snd_Divider & 0x3) == 0)
		{
			// calculate square1's output
			if (snd_SQ1_enable)
			{
				snd_SQ1_intl_cntr--;
				if (snd_SQ1_intl_cntr == 0)
				{
					snd_SQ1_intl_cntr = (2048 - snd_SQ1_frq) * 4;
					snd_SQ1_duty_cntr++;
					snd_SQ1_duty_cntr &= 7;

					if (snd_Duty_Cycles[snd_SQ1_duty * 8 + snd_SQ1_duty_cntr])
					{
						snd_SQ1_output = snd_SQ1_vol_state * 2;
					}
					else
					{
						snd_SQ1_output = 0;
					}
				}
			}

			// calculate square2's output
			if (snd_SQ2_enable)
			{
				snd_SQ2_intl_cntr--;
				if (snd_SQ2_intl_cntr == 0)
				{
					snd_SQ2_intl_cntr = (2048 - snd_SQ2_frq) * 4;
					snd_SQ2_duty_cntr++;
					snd_SQ2_duty_cntr &= 7;

					if (snd_Duty_Cycles[snd_SQ2_duty * 8 + snd_SQ2_duty_cntr])
					{
						snd_SQ2_output = snd_SQ2_vol_state * 2;
					}
					else
					{
						snd_SQ2_output = 0;
					}
				}
			}

			// calculate wave output
			snd_WAVE_can_get = false;
			if (snd_WAVE_enable)
			{
				snd_WAVE_intl_cntr--;

				if (snd_WAVE_intl_cntr == 0)
				{
					snd_WAVE_can_get = true;

					snd_WAVE_intl_cntr = (2048 - snd_WAVE_frq) * 2;

					snd_Sample = (uint8_t)((snd_Wave_RAM[snd_Wave_Bank_Playing] >> 4) & 0xF);

					if (snd_Wave_Vol_Force)
					{
						snd_Sample = (uint8_t)(((snd_Sample * 3) >> 2) & 0xF);
					}
					else
					{
						if (snd_WAVE_vol_code == 0)
						{
							snd_Sample = (uint8_t)((snd_Sample & 0xF) >> 4);
						}
						else if (snd_WAVE_vol_code == 1)
						{
							snd_Sample = (uint8_t)(snd_Sample & 0xF);
						}
						else if (snd_WAVE_vol_code == 2)
						{
							snd_Sample = (uint8_t)((snd_Sample & 0xF) >> 1);
						}
						else
						{
							snd_Sample = (uint8_t)((snd_Sample & 0xF) >> 2);
						}
					}

					snd_WAVE_output = snd_Sample * 2;

					uint8_t temp_samp = (uint8_t)((snd_Wave_RAM[snd_Wave_Bank_Playing] & 0xF0) >> 4);

					snd_Wave_RAM[snd_Wave_Bank_Playing] = (uint8_t)((snd_Wave_RAM[snd_Wave_Bank_Playing] & 0xF) << 4);

					for (int i = 1; i <= 15; i++)
					{
						snd_Wave_RAM[snd_Wave_Bank_Playing + i - 1] |= (uint8_t)((snd_Wave_RAM[snd_Wave_Bank_Playing + i] & 0xF0) >> 4);

						snd_Wave_RAM[snd_Wave_Bank_Playing + i] = (uint8_t)((snd_Wave_RAM[snd_Wave_Bank_Playing + i] & 0xF) << 4);
					}

					snd_Wave_RAM[snd_Wave_Bank_Playing + 15] |= temp_samp;

					// NOTE: The snd_Sample buffer is only reloaded after the current snd_Sample is played, even if just triggered
					snd_WAVE_wave_cntr++;

					// swap playing banks if selected
					if (snd_Wave_Size)
					{
						if (snd_WAVE_wave_cntr == 32)
						{
							if (snd_Wave_Bank_Playing == 16)
							{
								snd_Wave_Bank_Playing = 0;
							}
							else
							{
								snd_Wave_Bank_Playing = 16;
							}
						}
					}

					snd_WAVE_wave_cntr &= 0x1F;
				}
			}

			// calculate noise output
			if (snd_NOISE_enable)
			{
				snd_NOISE_intl_cntr--;
				if (snd_NOISE_intl_cntr == 0)
				{
					snd_NOISE_intl_cntr = (snd_Divisor[snd_NOISE_div_code] << snd_NOISE_clk_shft);
					int bit_lfsr = (snd_NOISE_LFSR & 1) ^ ((snd_NOISE_LFSR & 2) >> 1);

					snd_NOISE_LFSR = (snd_NOISE_LFSR >> 1) & 0x3FFF;
					snd_NOISE_LFSR |= (bit_lfsr << 14);

					if (snd_NOISE_wdth_md)
					{
						snd_NOISE_LFSR = snd_NOISE_LFSR & 0x7FBF;
						snd_NOISE_LFSR |= (bit_lfsr << 6);
					}

					if ((snd_NOISE_LFSR & 1) > 0)
					{
						snd_NOISE_output = 0;
					}
					else
					{
						snd_NOISE_output = snd_NOISE_vol_state * 2;
					}
				}
			}

			// Frame Sequencer ticks at a rate of 512 hz
			snd_Internal_cnt++;
			snd_Internal_cnt &= 0x1FFF;

			if ((snd_Internal_cnt == 0) && snd_CTRL_power)
			{
				snd_Sequencer_vol++; snd_Sequencer_vol &= 0x7;
				snd_Sequencer_len++; snd_Sequencer_len &= 0x7;
				snd_Sequencer_swp++; snd_Sequencer_swp &= 0x7;

				// clock the lengths
				if ((snd_Sequencer_len & 1) == 0)
				{
					if (snd_SQ1_len_en && snd_SQ1_len_cntr > 0)
					{
						snd_SQ1_len_cntr--;
						if (snd_SQ1_len_cntr == 0) { snd_SQ1_enable = snd_SQ1_swp_enable = false; }
					}
					if (snd_SQ2_len_en && snd_SQ2_len_cntr > 0)
					{
						snd_SQ2_len_cntr--;
						if (snd_SQ2_len_cntr == 0) { snd_SQ2_enable = false; }
					}
					if (snd_WAVE_len_en && snd_WAVE_len_cntr > 0)
					{
						snd_WAVE_len_cntr--;
						if (snd_WAVE_len_cntr == 0) { snd_WAVE_enable = false; }
					}
					if (snd_NOISE_len_en && snd_NOISE_len_cntr > 0)
					{
						snd_NOISE_len_cntr--;
						if (snd_NOISE_len_cntr == 0) { snd_NOISE_enable = false; }
					}

					snd_Update_NR52();
				}

				// clock the sweep
				if ((snd_Sequencer_swp == 0) || (snd_Sequencer_swp == 4))
				{
					snd_SQ1_intl_swp_cnt--;
					if ((snd_SQ1_intl_swp_cnt == 0) && snd_SQ1_swp_enable)
					{
						snd_SQ1_intl_swp_cnt = snd_SQ1_swp_prd > 0 ? snd_SQ1_swp_prd : 8;

						if ((snd_SQ1_swp_prd > 0))
						{
							int shadow_frq = snd_SQ1_frq_shadow;
							shadow_frq = shadow_frq >> snd_SQ1_shift;
							if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
							shadow_frq += snd_SQ1_frq_shadow;

							// set negate mode flag that disables channel is negate clerar
							if (snd_SQ1_negate) { snd_SQ1_calc_done = true; }

							// disable channel if overflow
							if ((uint32_t)shadow_frq > 2047)
							{
								snd_SQ1_enable = snd_SQ1_swp_enable = false;
							}
							else
							{
								if (snd_SQ1_shift > 0)
								{
									// NOTE: no need to write back to registers as they are not readable
									shadow_frq &= 0x7FF;
									snd_SQ1_frq = shadow_frq;
									snd_SQ1_frq_shadow = shadow_frq;

									// after writing, we repeat the process and do another overflow check
									shadow_frq = snd_SQ1_frq_shadow;
									shadow_frq = shadow_frq >> snd_SQ1_shift;
									if (snd_SQ1_negate) { shadow_frq = -shadow_frq; }
									shadow_frq += snd_SQ1_frq_shadow;

									if ((uint32_t)shadow_frq > 2047)
									{
										snd_SQ1_enable = snd_SQ1_swp_enable = false;
									}
								}
							}
						}

						snd_Update_NR52();
					}
				}

				// clock the volume envelope
				if (snd_Sequencer_vol == 5)
				{
					if (snd_SQ1_per > 0)
					{
						snd_SQ1_vol_per--;
						if (snd_SQ1_vol_per == 0)
						{
							snd_SQ1_vol_per = (snd_SQ1_per > 0) ? snd_SQ1_per : 8;
							if (!snd_SQ1_vol_done)
							{
								if (snd_SQ1_env_add)
								{
									if (snd_SQ1_vol_state < 15) { snd_SQ1_vol_state++; }
									else { snd_SQ1_vol_done = true; }
								}
								else
								{
									if (snd_SQ1_vol_state >= 1) { snd_SQ1_vol_state--; }
									else { snd_SQ1_vol_done = true; }
								}
							}
						}
					}

					if (snd_SQ2_per > 0)
					{
						snd_SQ2_vol_per--;
						if (snd_SQ2_vol_per == 0)
						{
							snd_SQ2_vol_per = (snd_SQ2_per > 0) ? snd_SQ2_per : 8;
							if (!snd_SQ2_vol_done)
							{
								if (snd_SQ2_env_add)
								{
									if (snd_SQ2_vol_state < 15) { snd_SQ2_vol_state++; }
									else { snd_SQ2_vol_done = true; }
								}
								else
								{
									if (snd_SQ2_vol_state >= 1) { snd_SQ2_vol_state--; }
									else { snd_SQ2_vol_done = true; }
								}
							}
						}
					}

					if (snd_NOISE_per > 0)
					{
						snd_NOISE_vol_per--;
						if (snd_NOISE_vol_per == 0)
						{
							snd_NOISE_vol_per = (snd_NOISE_per > 0) ? snd_NOISE_per : 8;
							if (!snd_NOISE_vol_done)
							{
								if (snd_NOISE_env_add)
								{
									if (snd_NOISE_vol_state < 15) { snd_NOISE_vol_state++; }
									else { snd_NOISE_vol_done = true; }
								}
								else
								{
									if (snd_NOISE_vol_state >= 1) { snd_NOISE_vol_state--; }
									else { snd_NOISE_vol_done = true; }
								}
							}
						}
					}
				}
			}

			if (snd_Sequencer_reset_cd > 0)
			{
				snd_Sequencer_reset_cd--;

				if (snd_Sequencer_reset_cd == 0)
				{
					// any side effects like GBC?
					snd_Sequencer_len = 0;
					snd_Sequencer_vol = 0;
					snd_Sequencer_swp = 0;
				}
			}
		}

		// caclulate FIFO outputs
		if (snd_FIFO_A_Tick)
		{
			if (snd_FIFO_A_ptr > 0)
			{
				snd_FIFO_A_Sample = snd_FIFO_A[0];

				for (int i = 1; i <= 31; i++)
				{
					snd_FIFO_A[i - 1] = snd_FIFO_A[i];
				}

				snd_FIFO_A[31] = 0;

				snd_FIFO_A_ptr -= 1;
			}

			if (snd_FIFO_A_ptr <= 14)
			{
				if (dma_Go[1] && dma_Start_Snd_Vid[1])
				{
					FIFO_DMA_A_cd = 3;
					FIFO_DMA_A_Delay = true;
					Misc_Delays = true;
					delays_to_process = true;
				}
			}

			snd_FIFO_A_Output = ((int8_t)(snd_FIFO_A_Sample)) * snd_FIFO_A_Mult;

			snd_FIFO_A_Tick = false;
		}

		if (snd_FIFO_B_Tick)
		{
			if (snd_FIFO_B_ptr > 0)
			{
				snd_FIFO_B_Sample = snd_FIFO_B[0];

				for (int i = 1; i <= 31; i++)
				{
					snd_FIFO_B[i - 1] = snd_FIFO_B[i];
				}

				snd_FIFO_B[31] = 0;

				snd_FIFO_B_ptr -= 1;
			}

			if (snd_FIFO_B_ptr <= 14)
			{
				if (dma_Go[2] && dma_Start_Snd_Vid[2])
				{
					FIFO_DMA_B_cd = 3;
					FIFO_DMA_B_Delay = true;
					Misc_Delays = true;
					delays_to_process = true;
				}
			}

			snd_FIFO_B_Output = ((int8_t)(snd_FIFO_B_Sample)) * snd_FIFO_B_Mult;

			snd_FIFO_B_Tick = false;
		}

		if ((snd_Divider & snd_Sample_Rate) == 0)
		{
			// add up components to each channel
			int32_t L_final = 0;
			int32_t R_final = 0;

			if (snd_CTRL_sq1_L_en) { L_final += snd_SQ1_output; }
			if (snd_CTRL_sq2_L_en) { L_final += snd_SQ2_output; }
			if (snd_CTRL_wave_L_en) { L_final += snd_WAVE_output; }
			if (snd_CTRL_noise_L_en) { L_final += snd_NOISE_output; }

			// channels 1-4 are effected by L/R volume controls, but not FIFO
			L_final *= (snd_CTRL_vol_L + 1);

			if (snd_FIFO_A_Enable_L) { L_final += snd_FIFO_A_Output; }
			if (snd_FIFO_B_Enable_L) { L_final += snd_FIFO_B_Output; }

			if (snd_CTRL_sq1_R_en) { R_final += snd_SQ1_output; }
			if (snd_CTRL_sq2_R_en) { R_final += snd_SQ2_output; }
			if (snd_CTRL_wave_R_en) { R_final += snd_WAVE_output; }
			if (snd_CTRL_noise_R_en) { R_final += snd_NOISE_output; }

			// channels 1-4 are effected by L/R volume controls, but not FIFO
			R_final *= (snd_CTRL_vol_R + 1);

			if (snd_FIFO_A_Enable_R) { R_final += snd_FIFO_A_Output; }
			if (snd_FIFO_B_Enable_R) { R_final += snd_FIFO_B_Output; }

			R_final += snd_BIAS_Offset;
			L_final += snd_BIAS_Offset;

			//R_final &= 0x3FF;
			//L_final &= 0x3FF;

			L_final *= 24;
			R_final *= 24;
				
			if (L_final != snd_Latched_Sample_L)
			{
				samples_L[num_samples_L * 2] = snd_Master_Clock;
				samples_L[num_samples_L * 2 + 1] = L_final - snd_Latched_Sample_L;

				num_samples_L += 1;

				snd_Latched_Sample_L = L_final;
			}

			if (R_final != snd_Latched_Sample_R)
			{
				samples_R[num_samples_R * 2] = snd_Master_Clock;
				samples_R[num_samples_R * 2 + 1] = R_final - snd_Latched_Sample_R;

				num_samples_R += 1;

				snd_Latched_Sample_R = R_final;
			}
		}

		snd_Master_Clock++;

		#pragma endregion

		#pragma region PPU Tick

		// start the next scanline
		if (ppu_Cycle == 1232)
		{
			// scanline callback

			if ((ppu_LY + 1) == Callback_Scanline)
			{
				if (ScanlineCallback)
				{
					ScanlineCallback();
				}
			}

			ppu_Cycle = 0;
			ppu_Display_Cycle = 0;
			ppu_LY += 1;

			if (ppu_LY == 228)
			{
				// display starts occuring at scanline 0
				ppu_In_VBlank = false;
				ppu_LY = 0;

				// reset rotation and scaling offsetreference Y value
				ppu_ROT_REF_LY[2] = 0;
				ppu_ROT_REF_LY[3] = 0;
			}

			// exit HBlank
			ppu_STAT &= 0xFD;

			// clear the LYC flag bit
			ppu_STAT &= 0xFB;

			// Check LY = LYC in 1 cycle
			// Check video DMA in 6 cycles
			ppu_LYC_Vid_Check_cd = 6;
			ppu_Delays = true;
			delays_to_process = true;

			if (ppu_LY == 160)
			{
				// vblank flag turns on on scanline 160
				VBlank_Rise = true;
				ppu_In_VBlank = true;
				ppu_STAT |= 1;

				// trigger VBL IRQ
				ppu_VBL_IRQ_cd = 4;
				ppu_Delays = true;
				delays_to_process = true;

				// do core actions that happen on VBlank
				On_VBlank();
			}
			else if (ppu_LY == 227)
			{
				// vblank flag turns off on scanline 227
				ppu_STAT &= 0xFE;
			}

			// reset sprite evaluation  in 40 cycles
			if ((ppu_LY == 227) || (ppu_LY <= 159))
			{
				ppu_Sprite_Delays = true;
				ppu_Sprite_Delay_SL = true;
				delays_to_process = true;
				ppu_Sprite_cd = 40;
			}

			// update BG toggles
			for (int i = 0; i < 4; i++)
			{
				if (ppu_BG_On_Update_Time[i] > 0)
				{
					ppu_BG_On_Update_Time[i]--;

					if (ppu_BG_On_Update_Time[i] == 0)
					{
						ppu_BG_On[i] = true;
					}
				}
			}

			if (ppu_OBJ_On_Time > 0)
			{
				ppu_OBJ_On_Time--;

				if (ppu_OBJ_On_Time == 0)
				{
					ppu_OBJ_On_Disp = true;
				}
			}

			if (ppu_Forced_Blank_Time > 0)
			{
				ppu_Forced_Blank_Time--;

				if (ppu_Forced_Blank_Time == 0)
				{
					ppu_Forced_Blank = false;
				}
			}

			// check Y range for windows
			// checks happen even if window is disabled
			if (ppu_LY == ppu_WIN0_Top) { ppu_WIN0_Active = true; }
			if (ppu_LY == ppu_WIN0_Bot) { ppu_WIN0_Active = false; }

			if (ppu_LY == ppu_WIN1_Top) { ppu_WIN1_Active = true; }
			if (ppu_LY == ppu_WIN1_Bot) { ppu_WIN1_Active = false; }
		}
		else if (ppu_Cycle == 1007)
		{
			// Enter HBlank
			ppu_STAT |= 2;
					
			ppu_HBL_IRQ_cd = 4;
			ppu_Delays = true;
			delays_to_process = true;
		}

		if (ppu_In_VBlank)
		{
			if (ppu_LY == 227)
			{
				ppu_VRAM_High_Access = false;
				ppu_OAM_Access = false;

				if (!ppu_Sprite_Eval_Finished && (ppu_Sprite_Render_Cycle < ppu_Sprite_Eval_Time))
				{
					if ((ppu_Cycle & 1) == 1)
					{
						if (ppu_Cycle >= 40)
						{
							ppu_Render_Sprites();
						}
					}
				}
			}
		}
		else
		{
			ppu_VRAM_High_Access = false;
			ppu_VRAM_Access = false;
			ppu_PALRAM_Access = false;
			ppu_OAM_Access = false;

			if (!ppu_Sprite_Eval_Finished && (ppu_Sprite_Render_Cycle < ppu_Sprite_Eval_Time))
			{
				if (ppu_LY == 3)
				{
					Message_String = "rendering  " + to_string(CycleCount);

					MessageCallback(Message_String.length());
				}
				
				if ((ppu_Cycle & 1) == 1)
				{
					ppu_Render_Sprites();
				}
			}
			else if ((ppu_LY == 3) && (ppu_Cycle < 100))
			{
				if (ppu_LY == 3)
				{
					Message_String = "not rendering  " + to_string(ppu_Sprite_Eval_Finished) + " " + to_string(ppu_OBJ_On);

					MessageCallback(Message_String.length());
				}
			}

			// sprite vram fetching is uneffected by forced blank based on new ram access timing test
			if (!ppu_Rendering_Complete && !ppu_Forced_Blank)
			{
				ppu_Render();
			}
		}

		ppu_Cycle += 1;

		#pragma endregion

		#pragma region Serial Port
		ser_div_cnt += 1;

		if (ser_Start)
		{
			if (ser_Internal_Clock)
			{
				// transfer 1 bit
				if ((ser_div_cnt & ser_Mask) == 0)
				{
					ser_Bit_Count += 1;

					if (ser_Bit_Count == ser_Bit_Total)
					{
						// reset start bit
						ser_Start = false;
						ser_CTRL &= 0xFF7F;
						ser_Ext_Tick = true;

						//Message_String = "Complete";

						//MessageCallback(Message_String.length());

						// trigger interrupt if needed
						if ((ser_CTRL & 0x4000) == 0x4000)
						{
							Trigger_IRQ(7);
						}
					}
				}
			}
			else if (GBP_Mode_Enabled)
			{
				// GBP rumble feature
				if (CycleCount >= ser_GBP_Next_Start_Time)
				{
					ser_GBP_Div_Count += 1;

					if ((ser_GBP_Div_Count & 0xF) == 0)
					{
						ser_Bit_Count += 1;

						if (ser_Bit_Count == ser_Bit_Total)
						{
							// reset start bit
							ser_Start = false;
							ser_CTRL &= 0xFF7F;

							if (ser_GBP_Transfer_Count >= 12)
							{
								if ((ser_Data_0 & 0xFF) != 0x04)
								{
									//Message_String = "Complete: " + to_string(ser_GBP_Transfer_Count) + " " + to_string(ser_Data_0 & 0xFF) + " " + to_string(CycleCount);

									//MessageCallback(Message_String.length());
								}
								if ((ser_Data_0 & 0xFF) == 0x26)
								{
									if (!Rumble_State)
									{
										if (RumbleCallback) { RumbleCallback(true); }

										//Message_String = "Rumble On";

										//MessageCallback(Message_String.length());

										Rumble_State = true;
									}
								}
								if ((ser_Data_0 & 0xFF) == 0x04)
								{
									if (Rumble_State)
									{
										if (RumbleCallback) { RumbleCallback(false); }

										//Message_String = "Rumble Off";

										//MessageCallback(Message_String.length());

										Rumble_State = false;
									}
								}
							}

							ser_Data_0 = (uint16_t)GBP_TRansfer_List[ser_GBP_Transfer_Count];
							ser_Data_1 = (uint16_t)(GBP_TRansfer_List[ser_GBP_Transfer_Count] >> 16);

							ser_GBP_Transfer_Count += 1;

							if (ser_GBP_Transfer_Count == 16)
							{
								ser_GBP_Transfer_Count = 0;

								//Message_String = "Complete";

								//MessageCallback(Message_String.length());
							}

							// trigger interrupt if needed
							if ((ser_CTRL & 0x4000) == 0x4000)
							{
								Trigger_IRQ(7);
							}
						}
					}
				}
			}
		}
		#pragma endregion

		#pragma region Timer

		bool tim_do_tick = false;

		tim_SubCnt += 1;

		if (!tim_All_Off)
		{
			for (int i = 0; i < 4; i++)
			{
				if (tim_ST_Time[i] > 0)
				{
					tim_ST_Time[i] -= 1;

					if (tim_ST_Time[i] == 1)
					{
						if (tim_Glitch_Tick[i])
						{
							if ((tim_SubCnt & tim_PreSc_En[i]) == 0)
							{
								if (((tim_Control[i] & 0x40) == 0x40) || tim_Old_IRQ[i])
								{
									Trigger_IRQ((uint16_t)(3 + i));
								}

								// Trigger sound FIFO updates
								if (snd_FIFO_A_Timer == i) { snd_FIFO_A_Tick = snd_CTRL_power; }
								if (snd_FIFO_B_Timer == i) { snd_FIFO_B_Tick = snd_CTRL_power; }

								tim_Prev_Tick[i + 1] = true;
							}

							tim_Glitch_Tick[i] = false;
						}

						if (tim_Enable_Not_Update[i])
						{
							tim_Timer[i] = tim_Reload[i];

							// if the reload register was just written to, use the old value
							if (tim_Just_Reloaded == i)
							{
								tim_Timer[i] = tim_Old_Reload;
							}
						}
					}

					if (tim_ST_Time[i] == 0)
					{
						tim_Go[i] = true;

						tim_PreSc[i] = tim_PreSc_En[i];
					}
				}

				if (tim_Go[i])
				{
					tim_do_tick = false;

					if (!tim_Tick_By_Prev[i])
					{
						if ((tim_SubCnt & tim_PreSc[i]) == 0)
						{
							tim_do_tick = true;
						}
					}
					else if (tim_Prev_Tick[i]) { tim_do_tick = true; }

					if (tim_do_tick)
					{
						tim_Timer[i] += 1;

						if (tim_Timer[i] == 0)
						{
							if (((tim_Control[i] & 0x40) == 0x40) || tim_Old_IRQ[i])
							{
								Trigger_IRQ((uint16_t)(3 + i));
							}

							// reload the timer
							tim_Timer[i] = tim_Reload[i];

							// if the reload register was just written to, use the old value
							if (tim_Just_Reloaded == i)
							{
								tim_Timer[i] = tim_Old_Reload;
							}

							//tim_Just_Reloaded = i;

							// Trigger sound FIFO updates
							if (snd_FIFO_A_Timer == i) { snd_FIFO_A_Tick = snd_CTRL_power; }
							if (snd_FIFO_B_Timer == i) { snd_FIFO_B_Tick = snd_CTRL_power; }

							tim_Prev_Tick[i + 1] = true;
						}
					}

					if (tim_Disable[i])
					{
						tim_Go[i] = false;

						tim_ST_Time[i] = 0;

						tim_All_Off = true;

						for (int k = 0; k < 4; k++)
						{
							tim_All_Off &= !tim_Go[k];
							tim_All_Off &= (tim_ST_Time[k] == 0);
						}

						tim_Disable[i] = false;

						tim_Old_IRQ[i] = false;
					}
				}

				tim_Prev_Tick[i] = false;
			}
		}

		tim_Just_Reloaded = 5;
		#pragma endregion

		#pragma region Prefetch

		pre_Cycle_Glitch = false;

		// if enabled, try to read from ROM if buffer is not full
		// if not enabled, finish current fetch
		if (pre_Run)
		{
			if (pre_Inactive || (pre_Check_Addr == 0) || pre_Buffer_Was_Full)
			{
				// if we reached an 0x20000 boundary, and haven't immediately encountered an instruction access, add a cycle glitch
				if (pre_Boundary_Reached)
				{
					pre_Cycle_Glitch_2 = true;
				}
			}		
			else if (pre_Fetch_Cnt == 0)
			{
				if (pre_Buffer_Cnt == 8) { pre_Buffer_Was_Full = true; } // don't start a read if buffer is full
				else if ((pre_Read_Addr & 0x1FFFE) == 0)
				{
					pre_Boundary_Reached = true;

					pre_Buffer_Was_Full = true;

					pre_Fetch_Wait = 0;

					pre_Inactive = true;

					if (pre_Buffer_Cnt == 0)
					{
						pre_Check_Addr = 0;
					}
				}
				else
				{
					if (pre_Read_Addr < 0x0A000000)
					{
						pre_Fetch_Wait = ROM_Waits_0_S + 1; // ROM 0				
					}
					else if (pre_Read_Addr < 0x0C000000)
					{
						pre_Fetch_Wait = ROM_Waits_1_S + 1; // ROM 1
					}
					else
					{
						pre_Fetch_Wait = ROM_Waits_2_S + 1; // ROM 2
					}

					// NOTE: minimum time to fetch somethiung from cart is 2 cycles
					pre_Fetch_Cnt = 1;
				}
			}
			else
			{

				pre_Following = true;

				pre_Fetch_Cnt += 1;

				if (pre_Fetch_Cnt == pre_Fetch_Wait)
				{
					pre_Buffer_Cnt += 1;
					pre_Fetch_Cnt = 0;
					pre_Read_Addr += 2;

					pre_Cycle_Glitch = true;

					if (!pre_Enable) { pre_Run = false; }
				}
			}
		}

		#pragma endregion

		#pragma region DMA Tick
		// if no channels are on, skip
		if (!dma_All_Off)
		{
			if (dma_Pausable)
			{
				// if no channel is currently running, or we have completed a memory cycle, look for a new one to run
				// zero is highest priority channel
				for (int i = 0; i < 4; i++)
				{
					if (dma_Run[i])
					{
						// if the current channel is less then the previous (i.e. not just unpaused) reset execution timing
						if (i < dma_Chan_Exec)
						{
							if ((dma_SRC_intl[i] >= 0xE000000) && (dma_DST_intl[i] >= 0xE000000))
							{
								dma_ST_Time[i] = 4;
							}
							else
							{
								dma_ST_Time[i] = 2;
							}

							if ((dma_Chan_Exec < 4) || dma_Shutdown)
							{
								dma_ST_Time[i] -= 1;
							}

							dma_Chan_Exec = i;

							//Message_String = "dma : " + to_string(dma_Chan_Exec) + " ly " + to_string(ppu_LY) + " cyc " + to_string(ppu_Cycle) + " cpu cyc " + to_string(CycleCount);

							//MessageCallback(Message_String.length());

							// Is this correct for all cases?
							dma_Use_ROM_Addr_DST[dma_Chan_Exec] = dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = false;

							dma_ROM_Being_Used[dma_Chan_Exec] = false;

							if (((dma_DST_intl[i] & 0x08000000) != 0) && ((dma_DST_intl[i] & 0x0E000000) != 0x0E000000))
							{
								// special case for ROM area
								dma_ROM_Addr[dma_Chan_Exec] = dma_DST_intl[i];
								dma_Use_ROM_Addr_DST[dma_Chan_Exec] = true;
								dma_ROM_Being_Used[dma_Chan_Exec] = true;
							}

							if (((dma_SRC_intl[i] & 0x08000000) != 0) && ((dma_SRC_intl[i] & 0x0E000000) != 0x0E000000))
							{
								// special case for ROM area
								dma_ROM_Addr[dma_Chan_Exec] = dma_SRC_intl[i];
								dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = true;
								dma_ROM_Being_Used[dma_Chan_Exec] = true;
							}
						}
					}
				}

				if (dma_Chan_Exec < 4)
				{
					dma_Shutdown = false;

					if (dma_ST_Time[dma_Chan_Exec] > 0)
					{
						dma_ST_Time[dma_Chan_Exec] -= 1;

						if (!cpu_Is_Paused)
						{
							if (cpu_Instr_Type == cpu_Pause_For_DMA)
							{
								// we just ended a DMA and immediately want to start another one
								// with 1 cycle in between
								cpu_Is_Paused = true;

								cpu_Restore_IRQ_Clock = false;

								dma_Seq_Access = false;

								dma_Access_Cnt = 0;
								dma_Access_Wait = 0;
							}
							else
							{
								// CPU can be paused on the edge of a memory access, if bus is not locked
								if ((cpu_Fetch_Cnt == 0) && !cpu_Swap_Lock)
								{
									// hold the current cpu instruction
									dma_Held_CPU_Instr = cpu_Instr_Type;
									cpu_Instr_Type = cpu_Pause_For_DMA;

									dma_Held_CPU_LDM_Glitch_Instr = cpu_LDM_Glitch_Instr_Type;
									cpu_LDM_Glitch_Instr_Type = cpu_Pause_For_DMA;

									cpu_Is_Paused = true;

									cpu_Restore_IRQ_Clock = false;

									dma_Seq_Access = false;

									dma_Access_Cnt = 0;
									dma_Access_Wait = 0;

									cpu_Seq_Access = false;
								}
								else
								{
									dma_ST_Time[dma_Chan_Exec] += 1;
								}
							}
						}
						else
						{
							// we paused a dma for a higher priority one
							cpu_Is_Paused = true;

							cpu_Restore_IRQ_Clock = false;

							dma_Seq_Access = false;

							dma_Access_Cnt = 0;
							dma_Access_Wait = 0;
						}
					}

					if (dma_ST_Time[dma_Chan_Exec] == 0)
					{
						dma_Pausable = false;
					}
				}
			}
			
			if (dma_Shutdown)
			{
				cpu_Is_Paused = false;

				dma_All_Off = true;

				for (int i = 0; i < 4; i++) { dma_All_Off &= !dma_Run[i]; }

				dma_Shutdown = false;
			}

			if (!dma_Pausable)
			{
				// if a channel is in the middle of read/write cycle, continue with the cycle
				if (dma_Read_Cycle[dma_Chan_Exec])
				{
					if (dma_Access_Cnt == 0)
					{
						if (dma_Use_ROM_Addr_SRC[dma_Chan_Exec])
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								if (dma_ROM_Dec_Glitch_Read[dma_Chan_Exec])
								{
									dma_ROM_Dec_Glitch_Read[dma_Chan_Exec] = false;
									dma_Seq_Access = false;
								}

								if ((dma_SRC_intl[dma_Chan_Exec] & 0x1FFFC) == 0x1FFFC)
								{
									dma_ROM_Dec_Glitch_Read[dma_Chan_Exec] = true;
								}
								
								dma_Access_Wait = Wait_State_Access_32_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								if (dma_ROM_Dec_Glitch_Read[dma_Chan_Exec])
								{
									dma_ROM_Dec_Glitch_Read[dma_Chan_Exec] = false;
									dma_Seq_Access = false;
								}

								if ((dma_SRC_intl[dma_Chan_Exec] & 0x1FFFE) == 0x1FFFE)
								{
									dma_ROM_Dec_Glitch_Read[dma_Chan_Exec] = true;
								}
								
								dma_Access_Wait = Wait_State_Access_16(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}

							// for ROM to ROM transfers, every access after the first is sequential (even though the write address is unrelated)
							if (!dma_Seq_Access && dma_Use_ROM_Addr_DST[dma_Chan_Exec])
							{
								dma_Seq_Access = true;
							}
						}
						else
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								dma_Access_Wait = Wait_State_Access_32_DMA(dma_SRC_intl[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								dma_Access_Wait = Wait_State_Access_16(dma_SRC_intl[dma_Chan_Exec], dma_Seq_Access);
							}
						}
					}

					dma_Access_Cnt++;

					if (dma_Access_Cnt == dma_Access_Wait)
					{
						if (dma_Access_32[dma_Chan_Exec])
						{
							// This updates the bus value.
							if (dma_Use_ROM_Addr_SRC[dma_Chan_Exec])
							{
								Read_Memory_32_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_Chan_Exec);

								dma_ROM_Addr[dma_Chan_Exec] += 4;
							}
							else
							{
								Read_Memory_32_DMA(dma_SRC_intl[dma_Chan_Exec], dma_Chan_Exec);
							}
							// The transfer value is now whatever the bus value is.
							dma_TFR_Word = dma_Last_Bus_Value[dma_Chan_Exec];
							// also update the cpu open bus state
							cpu_Last_Bus_Value = dma_TFR_Word;
						}
						else
						{
							// This updates the bus value.
							if (dma_Use_ROM_Addr_SRC[dma_Chan_Exec])
							{
								Read_Memory_16_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_Chan_Exec);

								dma_ROM_Addr[dma_Chan_Exec] += 2;
							}
							else
							{
								Read_Memory_16_DMA(dma_SRC_intl[dma_Chan_Exec], dma_Chan_Exec);
							}
							// The transfer value might be a rotated version of the bus value, depending on the destination address.
							// In normal cases this doesn't matter since both the upper and lower half words of the bus are updated with the same value.
							// But in open bus cases they could be different.
							if ((dma_DST_intl[dma_Chan_Exec] & 2) == 0)
							{
								dma_TFR_HWord = (uint16_t)(dma_Last_Bus_Value[dma_Chan_Exec] & 0xFFFF);
							}
							else
							{
								// does it also update the bus?
								dma_TFR_HWord = (uint16_t)((dma_Last_Bus_Value[dma_Chan_Exec] >> 16) & 0xFFFF);
							}
							// also update the cpu open bus state
							cpu_Last_Bus_Value = dma_TFR_HWord;
						}

						dma_SRC_intl[dma_Chan_Exec] += dma_SRC_INC[dma_Chan_Exec];
						dma_SRC_intl[dma_Chan_Exec] &= dma_SRC_Mask[dma_Chan_Exec];

						if (((dma_SRC_intl[dma_Chan_Exec] & 0x08000000) != 0) && ((dma_SRC_intl[dma_Chan_Exec] & 0x0E000000) != 0x0E000000))
						{
							// If ROM is not already being accessed
							if (!dma_ROM_Being_Used[dma_Chan_Exec])
							{
								dma_ROM_Addr[dma_Chan_Exec] = dma_SRC_intl[dma_Chan_Exec];
							}

							dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = true;

							dma_ROM_Being_Used[dma_Chan_Exec] = true;
						}
						else
						{
							dma_Use_ROM_Addr_SRC[dma_Chan_Exec] = false;
						}

						dma_Read_Cycle[dma_Chan_Exec] = !dma_Read_Cycle[dma_Chan_Exec];
						dma_Access_Cnt = 0;
					}
				}
				else
				{
					if (dma_Access_Cnt == 0)
					{
						if (dma_Use_ROM_Addr_DST[dma_Chan_Exec])
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								if (dma_ROM_Dec_Glitch_Write[dma_Chan_Exec])
								{
									dma_ROM_Dec_Glitch_Write[dma_Chan_Exec] = false;
									dma_Seq_Access = false;
								}

								if ((dma_DST_intl[dma_Chan_Exec] & 0x1FFFC) == 0x1FFFC)
								{
									dma_ROM_Dec_Glitch_Write[dma_Chan_Exec] = true;
								}
								
								dma_Access_Wait = Wait_State_Access_32_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								if (dma_ROM_Dec_Glitch_Write[dma_Chan_Exec])
								{
									dma_ROM_Dec_Glitch_Write[dma_Chan_Exec] = false;
									dma_Seq_Access = false;
								}

								if ((dma_DST_intl[dma_Chan_Exec] & 0x1FFFE) == 0x1FFFE)
								{
									dma_ROM_Dec_Glitch_Write[dma_Chan_Exec] = true;
								}
								
								dma_Access_Wait = Wait_State_Access_16(dma_ROM_Addr[dma_Chan_Exec], dma_Seq_Access);
							}
						}
						else
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								dma_Access_Wait = Wait_State_Access_32_DMA(dma_DST_intl[dma_Chan_Exec], dma_Seq_Access);
							}
							else
							{
								dma_Access_Wait = Wait_State_Access_16(dma_DST_intl[dma_Chan_Exec], dma_Seq_Access);
							}
						}
					}

					dma_Access_Cnt++;

					if (dma_Access_Cnt == dma_Access_Wait)
					{
						if (dma_Use_ROM_Addr_DST[dma_Chan_Exec])
						{
							if (dma_Access_32[dma_Chan_Exec])
							{							
								Write_Memory_32_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_TFR_Word, dma_Chan_Exec);
							}
							else
							{
								Write_Memory_16_DMA(dma_ROM_Addr[dma_Chan_Exec], dma_TFR_HWord, dma_Chan_Exec);
							}

							dma_ROM_Addr[dma_Chan_Exec] += (uint32_t)(dma_Access_32[dma_Chan_Exec] ? 4 : 2);
						}
						else
						{
							if (dma_Access_32[dma_Chan_Exec])
							{
								Write_Memory_32_DMA(dma_DST_intl[dma_Chan_Exec], dma_TFR_Word, dma_Chan_Exec);
							}
							else
							{
								Write_Memory_16_DMA(dma_DST_intl[dma_Chan_Exec], dma_TFR_HWord, dma_Chan_Exec);
							}
						}

						dma_DST_intl[dma_Chan_Exec] += dma_DST_INC[dma_Chan_Exec];
						dma_DST_intl[dma_Chan_Exec] &= dma_DST_Mask[dma_Chan_Exec];

						if (((dma_DST_intl[dma_Chan_Exec] & 0x08000000) != 0) && ((dma_DST_intl[dma_Chan_Exec] & 0x0E000000) != 0x0E000000))
						{
							// If ROM is not already being accessed
							if (!dma_ROM_Being_Used[dma_Chan_Exec])
							{
								dma_ROM_Addr[dma_Chan_Exec] = dma_DST_intl[dma_Chan_Exec];
							}

							dma_Use_ROM_Addr_DST[dma_Chan_Exec] = true;

							dma_ROM_Being_Used[dma_Chan_Exec] = true;
						}
						else
						{
							dma_Use_ROM_Addr_DST[dma_Chan_Exec] = false;
						}

						dma_Read_Cycle[dma_Chan_Exec] = !dma_Read_Cycle[dma_Chan_Exec];
						dma_Access_Cnt = 0;
						dma_Seq_Access = true;

						// end of a write cycle allows a possibility for a pause
						dma_Pausable = true;

						// check if the DMA is complete
						dma_CNT_intl[dma_Chan_Exec] -= 1;

						if (dma_CNT_intl[dma_Chan_Exec] == 0)
						{
							// generate an IRQ if needed
							if ((dma_CTRL[dma_Chan_Exec] & 0x4000) == 0x4000)
							{
								Misc_Delays = true;
								delays_to_process = true;
								DMA_IRQ_Delay[dma_Chan_Exec] = true;
								DMA_Any_IRQ = true;
							}

							// Repeat if necessary, or turn the channel off
							// don't repeat if start condition is immediate (see Shrek 2)
							if (((dma_CTRL[dma_Chan_Exec] & 0x200) == 0x200) && ((dma_CTRL[dma_Chan_Exec] & 0x3000) != 0))
							{
								dma_CNT_intl[dma_Chan_Exec] = dma_CNT[dma_Chan_Exec];

								if ((dma_Chan_Exec == 1) || (dma_Chan_Exec == 2))
								{
									if (dma_Start_Snd_Vid[dma_Chan_Exec] == true)
									{
										// ignore word count
										dma_CNT_intl[dma_Chan_Exec] = 4;
									}
								}

								// reload destination register
								if ((dma_CTRL[dma_Chan_Exec] & 0x60) == 0x60)
								{
									dma_DST_intl[dma_Chan_Exec] = (uint32_t)(dma_DST[dma_Chan_Exec] & dma_DST_Mask[dma_Chan_Exec]);
								}

								dma_Run[dma_Chan_Exec] = false;

								// for channel 3 running video capture mode, turn off after scanline 161
								if ((dma_Chan_Exec == 3) && dma_Start_Snd_Vid[3] && (ppu_LY == 161))
								{
									dma_CTRL[dma_Chan_Exec] &= 0x7FFF;

									dma_Run[dma_Chan_Exec] = false;
									dma_Go[dma_Chan_Exec] = false;
								}
							}
							else
							{
								dma_CTRL[dma_Chan_Exec] &= 0x7FFF;

								dma_Run[dma_Chan_Exec] = false;
								dma_Go[dma_Chan_Exec] = false;
							}

							dma_ROM_Dec_Glitch_Read[dma_Chan_Exec] = false;
							dma_ROM_Dec_Glitch_Write[dma_Chan_Exec] = false;

							// In any case, we start a new DMA
							dma_Chan_Exec = 4;
							dma_Shutdown = true;
						}
						else if (!dma_Run[dma_Chan_Exec])
						{
							dma_ROM_Dec_Glitch_Read[dma_Chan_Exec] = false;
							dma_ROM_Dec_Glitch_Write[dma_Chan_Exec] = false;
							
							// DMA channel was turned off by the DMA itself
							dma_Chan_Exec = 4;
							dma_Shutdown = true;

							dma_All_Off = false;
						}
					}
				}
			}
		}
		#pragma endregion

		#pragma region CPU Tick
		if (cpu_Restore_IRQ_Clock)
		{
			cpu_No_IRQ_Clock = false;
			cpu_Restore_IRQ_Clock = false;
		}

		if (!cpu_No_IRQ_Clock)
		{
			cpu_IRQ_Input = cpu_Next_IRQ_Input;
			cpu_Next_IRQ_Input = cpu_Next_IRQ_Input_2;

			cpu_Trigger_Unhalt = cpu_Trigger_Unhalt_2;
			cpu_Trigger_Unhalt_2 = cpu_Trigger_Unhalt_3;
		}

		cpu_Next_IRQ_Input_2 = cpu_Next_IRQ_Input_3;
		cpu_Trigger_Unhalt_3 = cpu_Trigger_Unhalt_4;
		
		switch (cpu_Instr_Type)
		{
			case cpu_Internal_And_Prefetch_ARM:
				// In this code path the instruction takes only one internal cycle, a pretech is also done
				// so necessarily the condition code check happens here, and interrupts may occur
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = true;
				}
				break;

			case cpu_Internal_And_Prefetch_2_ARM:
				// In this code path the instruction takes 2 internal cycles, a pretech is also done
				// so necessarily the condition code check happens here, and interrupts may occur if it fails
				// TODO: In ARM mode, does a failed condition execute 1 cycle or 2? (For now assume 1)
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					if (cpu_ARM_Cond_Passed)
					{
						cpu_Instr_Type = cpu_Internal_Can_Save_ARM;

						// instructions with internal cycles revert to non-sequential accesses 
						cpu_Seq_Access = false;
					}
					else
					{
						// do interrupt check and proceed to next instruction
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;			
				}
				break;

			case cpu_Internal_And_Prefetch_3_ARM:
				// This code path comes from instructions that modify CPSR (only from ARM mode)
				// if we end up in thumb state, invalidate instruction pipeline
				// otherwise check interrupts
				// NOTE: Here we must use the old value of the I flag
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_FlagI_Old = cpu_FlagIget();

					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					if (cpu_Thumb_Mode)
					{
						// invalidate the instruction pipeline
						cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;

						cpu_Seq_Access = false;
					}
					else
					{
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Internal_And_Branch_1_ARM:
				// decide whether or not to branch. If no branch taken, interrupts may occur
				if (cpu_Fetch_Cnt == 0)
				{
					// whether or not to take the branch is determined in the instruction execution
					cpu_Take_Branch = false;

					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

					if (cpu_Take_Branch)
					{
						cpu_Regs[15] = cpu_Temp_Reg;

						// Invalidate instruction pipeline
						cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
					}
					else
					{
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = cpu_Take_Branch ? false : true;
				}
				break;

			case cpu_Internal_And_Branch_2_ARM:
				// this code path comes from an ALU instruction in ARM mode using R15 as the destination register
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_FlagI_Old = cpu_FlagIget();
				
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

					cpu_Execute_Internal_Only_ARM();

					if (cpu_ARM_Cond_Passed)
					{
						// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
						if (cpu_ALU_S_Bit)
						{
							if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
							{
								// nothing to swap
							}
							else
							{
								//upper bit of mode must always be set
								cpu_Swap_Regs((cpu_Regs[17] & 0x1F) | 0x10, false, true);
							}

							if (cpu_FlagTget()) { cpu_Thumb_Mode = true; cpu_Clear_Pipeline = true; }
							else { cpu_Thumb_Mode = false; }

							cpu_ALU_S_Bit = false;
						}

						// Invalidate instruction pipeline if necessary
						if (cpu_Clear_Pipeline)
						{
							if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
							else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }
							cpu_Seq_Access = false;
						}
						else
						{
							cpu_Regs[15] += 4;

							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = true;
						}
					}
					else
					{
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Internal_And_Branch_3_ARM:
				// This code path comes from an ALU instruction in ARM mode using R15 as the destination register
				// and the shift is defined by another register (therefore taking an extra cycle to complete)
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

					if (cpu_ARM_Cond_Passed)
					{
						cpu_Instr_Type = cpu_Internal_And_Branch_4_ARM;
					}
					else
					{
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Prefetch_Only_1_ARM:
				// In this code path the instruction pipeline is being refilled, and is part of an atomic instruction (cannot be interrupted)
				// so no instruction execution takes place
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Instr_Type = cpu_Prefetch_Only_2_ARM;
					cpu_Seq_Access = true;
				}
				break;

			case cpu_Prefetch_Only_2_ARM:
				// This code path is the last cycle of pipeline refill, no instruction execution but interrupts may occur
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = true;
				}
				break;

			case cpu_Prefetch_And_Load_Store_ARM:
				// First cycle of load / store, cannot be interrupted (if executed), prefetch occurs
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					if (cpu_ARM_Cond_Passed)
					{
						cpu_Instr_Type = cpu_Next_Load_Store_Type;
						cpu_Seq_Access = false;
					}
					else
					{
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }
						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Load_Store_Word_ARM:
				// Single load / store word, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// 32 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						// deal with misaligned accesses
						if ((cpu_Temp_Addr & 3) == 0)
						{
							cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
						}
						else
						{
							cpu_ALU_Temp_Val = Read_Memory_32(cpu_Temp_Addr);

							for (int i = 0; i < (cpu_Temp_Addr & 3); i++)
							{
								cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
							}

							cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
					}
					else
					{
						Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
					}

					if (cpu_Overwrite_Base_Reg)
					{
						cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						cpu_Overwrite_Base_Reg = false;
					}

					if (cpu_LS_Is_Load)
					{
						if (cpu_Temp_Reg_Ptr == 15)
						{
							// Invalidate instruction pipeline
							cpu_Invalidate_Pipeline = true;
							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
							cpu_Seq_Access = false;
						}
						else
						{
							// if the next cycle is a memory access, one cycle can be saved
							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
							cpu_Internal_Save_Access = true;
							cpu_Seq_Access = false;
						}
					}
					else
					{
						// when writing, there is no last internal cycle, proceed to the next instruction
						// also check for interrupts
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = false;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Load_Store_Half_ARM:
				// Single load / store half word, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// 16 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_16(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						// deal with misaligned accesses
						if ((cpu_Temp_Addr & 1) == 0)
						{
							cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

							if (cpu_Sign_Extend_Load)
							{
								if ((cpu_ALU_Temp_Val & 0x8000) == 0x8000)
								{
									cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFF0000);
								}
							}

							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
						else
						{
							if (cpu_Sign_Extend_Load)
							{
								cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

								if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
								{
									cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFFFF00);
								}

								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}
							else
							{
								cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

								cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));

								cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}
						}
					}
					else
					{
						Write_Memory_16(cpu_Temp_Addr, (uint16_t)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFFFF));
					}

					if (cpu_Overwrite_Base_Reg)
					{
						cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						cpu_Overwrite_Base_Reg = false;
					}

					cpu_Sign_Extend_Load = false;

					if (cpu_LS_Is_Load)
					{
						if (cpu_Temp_Reg_Ptr == 15)
						{
							// Invalidate instruction pipeline
							cpu_Invalidate_Pipeline = true;
							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
							cpu_Seq_Access = false;
						}
						else
						{
							// if the next cycle is a memory access, one cycle can be saved
							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
							cpu_Internal_Save_Access = true;
							cpu_Seq_Access = false;
						}
					}
					else
					{
						// when writing, there is no last internal cycle, proceed to the next instruction
						// also check for interrupts
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = false;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Load_Store_Byte_ARM:
				// Single load / store byte, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// 8 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_8(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

						if (cpu_Sign_Extend_Load)
						{
							if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
							{
								cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFFFF00);
							}
						}

						cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
					}
					else
					{
						Write_Memory_8(cpu_Temp_Addr, (uint8_t)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFF));
					}

					if (cpu_Overwrite_Base_Reg)
					{
						cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						cpu_Overwrite_Base_Reg = false;
					}

					cpu_Sign_Extend_Load = false;

					if (cpu_LS_Is_Load)
					{
						if (cpu_Temp_Reg_Ptr == 15)
						{
							// Invalidate instruction pipeline
							cpu_Invalidate_Pipeline = true;
							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
							cpu_Seq_Access = false;
						}
						else
						{
							// if the next cycle is a memory access, one cycle can be saved
							cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
							cpu_Internal_Save_Access = true;
							cpu_Seq_Access = false;
						}
					}
					else
					{
						// when writing, there is no last internal cycle, proceed to the next instruction
						// also check for interrupts
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = false;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Multi_Load_Store_ARM:
				// Repeated load / store operations, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// update this here so the wait state processor knows about it for 32 bit accesses to VRAM and PALRAM
					cpu_Temp_Reg_Ptr = cpu_Regs_To_Access[cpu_Multi_List_Ptr];
					
					// 32 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
					}
					else
					{
						Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
					}

					// base register is updated after the first memory access
					if (cpu_LS_First_Access && cpu_Overwrite_Base_Reg)
					{
						if (cpu_Multi_Inc)
						{
							if (cpu_Multi_Before)
							{
								cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size) - 4);
							}
							else
							{
								cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size));
							}

							if (cpu_Special_Inc)
							{
								cpu_Special_Inc = false;
								cpu_Regs[cpu_Base_Reg] += 0x3C;
							}
						}
						else
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						}

						cpu_LS_First_Access = false;
						cpu_Overwrite_Base_Reg = false;
					}

					// always incrementing since addresses always start at the lowest one
					// always after because 'before' cases built into address at initialization
					cpu_Temp_Addr += 4;

					cpu_Multi_List_Ptr++;

					// if done, the next cycle depends on whether or not Reg 15 was written to
					if (cpu_Multi_List_Ptr == cpu_Multi_List_Size)
					{
						cpu_LDM_Glitch_Mode = false;
					
						// note that reg 15 cannot be used if swapping out regs, so glitch mode can only be enterred in ARm mode
						if (cpu_Multi_Swap)
						{
							cpu_LDM_Glitch_Mode = true;
						
							cpu_Swap_Regs(cpu_Temp_Mode, false, false);
						}

						if (cpu_LS_Is_Load)
						{
							if (cpu_Regs_To_Access[cpu_Multi_List_Ptr - 1] == 15)
							{
								// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
								if (cpu_Multi_S_Bit)
								{
									cpu_Regs[16] = cpu_Regs[17];

									if (cpu_FlagTget()) { cpu_Thumb_Mode = true; }
									else { cpu_Thumb_Mode = false; }

									cpu_Multi_S_Bit = false;
								}

								// is the timing of this correct?
								if (cpu_Thumb_Mode) { cpu_Regs[15] &= 0xFFFFFFFE; }
								else { cpu_Regs[15] &= 0xFFFFFFFC; }

								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Seq_Access = false;

								if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Internal_Can_Save_TMB; }
								else { cpu_Instr_Type = cpu_Internal_Can_Save_ARM; }
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								if (cpu_LDM_Glitch_Mode)
								{
									cpu_Instr_Type = cpu_LDM_Glitch_Mode_Execute;

									cpu_LDM_Glitch_Instr_Type = cpu_Internal_Can_Save_ARM;
								}
								else
								{
									cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
								}

								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// stm does not cause the glitch
							cpu_LDM_Glitch_Mode = false;
							
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
							cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

							if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_ARM(); }

							cpu_Seq_Access = false;
						}
					}
					else
					{
						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Multiply_ARM:
				// Multiplication with possibly early termination
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_ARM_Cond_Passed) { cpu_Instr_Type = cpu_Multiply_Cycles; }
					else
					{
						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = true;
				}
				break;

			case cpu_Prefetch_Swap_ARM:
				// First cycle of swap, locks the bus at the end of the cycle
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					cpu_Regs[15] += 4;

					if (cpu_ARM_Cond_Passed)
					{
						cpu_Instr_Type = cpu_Next_Load_Store_Type;
						cpu_Seq_Access = false;

						// Lock the bus
						cpu_Swap_Lock = true;
					}
					else
					{
						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }
						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Swap_ARM:
				// Swap data between registers and memory
				// no intervening memory accesses can occur between the read and the write
				// note that interrupts are checked on the last instruction cycle, handled in cpu_Internal_Can_Save
				if (cpu_Fetch_Cnt == 0)
				{
					// need this here for the 32 bit wait state processor in case VRAM and PALRAM accesses are interrupted
					cpu_LS_Is_Load = !cpu_Swap_Store;
					
					if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
					{
						cpu_Fetch_Wait = Wait_State_Access_32(cpu_Regs[cpu_Base_Reg], cpu_Seq_Access);
					}
					else
					{
						cpu_Fetch_Wait = Wait_State_Access_8(cpu_Regs[cpu_Base_Reg], cpu_Seq_Access);
					}
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_Swap_Store)
					{
						if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
						{
							Write_Memory_8(cpu_Regs[cpu_Base_Reg], (uint8_t)cpu_Regs[cpu_Temp_Reg_Ptr]);
						}
						else
						{
							Write_Memory_32(cpu_Regs[cpu_Base_Reg], cpu_Regs[cpu_Temp_Reg_Ptr]);
						}

						cpu_Regs[cpu_Base_Reg_2] = cpu_Temp_Data;

						cpu_Instr_Type = cpu_Internal_Can_Save_ARM;
						cpu_Invalidate_Pipeline = false;
						cpu_Internal_Save_Access = true;

						// unlock the bus
						cpu_Swap_Lock = false;
					}
					else
					{
						if ((cpu_Instr_ARM_2 & 0x00400000) == 0x00400000)
						{
							cpu_Temp_Data = Read_Memory_8(cpu_Regs[cpu_Base_Reg]);
						}
						else
						{
							// deal with misaligned accesses
							cpu_Temp_Addr = cpu_Regs[cpu_Base_Reg];

							if ((cpu_Temp_Addr & 3) == 0)
							{
								cpu_Temp_Data = Read_Memory_32(cpu_Temp_Addr);
							}
							else
							{
								cpu_ALU_Temp_Val = Read_Memory_32(cpu_Temp_Addr);

								for (int i = 0; i < (cpu_Temp_Addr & 3); i++)
								{
									cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
								}

								cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
								cpu_Temp_Data = cpu_ALU_Temp_Val;
							}
						}

						cpu_Swap_Store = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = false;
				}
				break;

			case cpu_Prefetch_And_Branch_Ex_ARM:
				// Branch from ARM mode to Thumb (possibly)
				// interrupt only if condition failed
				if (cpu_Fetch_Cnt == 0)
				{
					// start in thumb mode, always branch
					cpu_Take_Branch = true;

					cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_ARM();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_ARM_Cond_Passed)
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

						cpu_Regs[15] = cpu_Temp_Reg & 0xFFFFFFFE;

						cpu_FlagTset((cpu_Temp_Reg & 1) == 1);
						cpu_Thumb_Mode = cpu_FlagTget();

						// Invalidate instruction pipeline
						if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
						else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }

						cpu_Seq_Access = false;
					}
					else
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_ARM(); }

						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Internal_And_Prefetch_TMB:
				// In this code path the instruction takes only one internal cycle, a pretech is also done
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_TMB();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
					cpu_Regs[15] += 2;

					cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
					cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_TMB(); }

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = true;
				}
				break;

			case cpu_Internal_And_Prefetch_2_TMB:
				// In this code path the instruction takes 2 internal cycles, a pretech is also done
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_TMB();
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
					cpu_Regs[15] += 2;

					cpu_Instr_Type = cpu_Internal_Can_Save_TMB;

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					// instructions with internal cycles revert to non-sequential accesses 
					cpu_Seq_Access = false;
				}
				break;

			case cpu_Internal_And_Branch_1_TMB:
				// decide whether or not to branch. If no branch taken, interrupts may occur
				if (cpu_Fetch_Cnt == 0)
				{
					// whether or not to take the branch is determined in the instruction execution
					cpu_Take_Branch = false;

					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_TMB();

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);

					if (cpu_Take_Branch)
					{
						cpu_Regs[15] = (cpu_Temp_Reg & 0xFFFFFFFE);

						// Invalidate instruction pipeline
						cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;
					}
					else
					{
						cpu_Regs[15] += 2;

						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_TMB(); }
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = cpu_Take_Branch ? false : true;
				}
				break;

			case cpu_Prefetch_Only_1_TMB:
				// In this code path the instruction pipeline is being refilled, and is part of an atomic instruction (cannot be interrupted)
				// so no instruction execution takes place
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
					cpu_Regs[15] += 2;

					cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
					cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Instr_Type = cpu_Prefetch_Only_2_TMB;
					cpu_Seq_Access = true;
				}
				break;

			case cpu_Prefetch_Only_2_TMB:
				// This code path is the last cycle of pipeline refill, no instruction execution but interrupts may occur
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
					cpu_Regs[15] += 2;

					cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
					cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_TMB(); }

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = true;
				}
				break;

			case cpu_Prefetch_And_Load_Store_TMB:
				// First cycle of load / store, cannot be interrupted (if executed), prefetch occurs
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_TMB();
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
					cpu_Regs[15] += 2;

					cpu_Instr_Type = cpu_Next_Load_Store_Type;
					cpu_Seq_Access = false;

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Load_Store_Word_TMB:
				// Single load / store word, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// 32 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						// deal with misaligned accesses
						if ((cpu_Temp_Addr & 3) == 0)
						{
							cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
						}
						else
						{
							cpu_ALU_Temp_Val = Read_Memory_32(cpu_Temp_Addr);

							for (int i = 0; i < (cpu_Temp_Addr & 3); i++)
							{
								cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));
							}

							cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
					}
					else
					{
						Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
					}

					if (cpu_Overwrite_Base_Reg)
					{
						cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						cpu_Overwrite_Base_Reg = false;
					}

					if (cpu_LS_Is_Load)
					{
						if (cpu_Temp_Reg_Ptr == 15)
						{
							// Invalidate instruction pipeline
							cpu_Invalidate_Pipeline = true;
							cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
							cpu_Seq_Access = false;
						}
						else
						{
							// if the next cycle is a memory access, one cycle can be saved
							cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
							cpu_Internal_Save_Access = true;
							cpu_Seq_Access = false;
						}
					}
					else
					{
						// when writing, there is no last internal cycle, proceed to the next instruction
						// also check for interrupts
						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_TMB(); }

						cpu_Seq_Access = false;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Load_Store_Half_TMB:
				// Single load / store half word, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// 16 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_16(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						// deal with misaligned accesses
						if ((cpu_Temp_Addr & 1) == 0)
						{
							cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

							if (cpu_Sign_Extend_Load)
							{
								if ((cpu_ALU_Temp_Val & 0x8000) == 0x8000)
								{
									cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFF0000);
								}
							}

							cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
						}
						else
						{
							if (cpu_Sign_Extend_Load)
							{
								cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

								if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
								{
									cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFFFF00);
								}

								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}
							else
							{
								cpu_ALU_Temp_Val = Read_Memory_16(cpu_Temp_Addr);

								cpu_ALU_Temp_Val = (uint32_t)((cpu_ALU_Temp_Val >> 8) | ((cpu_ALU_Temp_Val & 0xFF) << 24));

								cpu_Last_Bus_Value = cpu_ALU_Temp_Val;
								cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
							}
						}
					}
					else
					{
						Write_Memory_16(cpu_Temp_Addr, (uint16_t)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFFFF));
					}

					if (cpu_Overwrite_Base_Reg)
					{
						cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						cpu_Overwrite_Base_Reg = false;
					}

					cpu_Sign_Extend_Load = false;

					if (cpu_LS_Is_Load)
					{
						if (cpu_Temp_Reg_Ptr == 15)
						{
							// Invalidate instruction pipeline
							cpu_Invalidate_Pipeline = true;
							cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
							cpu_Seq_Access = false;
						}
						else
						{
							// if the next cycle is a memory access, one cycle can be saved
							cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
							cpu_Internal_Save_Access = true;
							cpu_Seq_Access = false;
						}
					}
					else
					{
						// when writing, there is no last internal cycle, proceed to the next instruction
						// also check for interrupts
						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_TMB(); }

						cpu_Seq_Access = false;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Load_Store_Byte_TMB:
				// Single load / store byte, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// 8 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_8(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						cpu_ALU_Temp_Val = Read_Memory_8(cpu_Temp_Addr);

						if (cpu_Sign_Extend_Load)
						{
							if ((cpu_ALU_Temp_Val & 0x80) == 0x80)
							{
								cpu_ALU_Temp_Val = (uint32_t)(cpu_ALU_Temp_Val | 0xFFFFFF00);
							}
						}

						cpu_Regs[cpu_Temp_Reg_Ptr] = cpu_ALU_Temp_Val;
					}
					else
					{
						Write_Memory_8(cpu_Temp_Addr, (uint8_t)(cpu_Regs[cpu_Temp_Reg_Ptr] & 0xFF));
					}

					if (cpu_Overwrite_Base_Reg)
					{
						cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						cpu_Overwrite_Base_Reg = false;
					}

					cpu_Sign_Extend_Load = false;

					if (cpu_LS_Is_Load)
					{
						if (cpu_Temp_Reg_Ptr == 15)
						{
							// Invalidate instruction pipeline
							cpu_Invalidate_Pipeline = true;
							cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
							cpu_Seq_Access = false;
						}
						else
						{
							// if the next cycle is a memory access, one cycle can be saved
							cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
							cpu_Internal_Save_Access = true;
							cpu_Seq_Access = false;
						}
					}
					else
					{
						// when writing, there is no last internal cycle, proceed to the next instruction
						// also check for interrupts
						cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
						cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
						else { cpu_Decode_TMB(); }

						cpu_Seq_Access = false;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Multi_Load_Store_TMB:
				// Repeated load / store operations, last cycle of a write can be interrupted
				// Can be interrupted by an abort, but those don't occur in GBA
				if (cpu_Fetch_Cnt == 0)
				{
					// update this here so the wait state processor knows about it for 32 bit accesses to VRAM and PALRAM
					cpu_Temp_Reg_Ptr = cpu_Regs_To_Access[cpu_Multi_List_Ptr];
					
					// 32 bit fetch regardless of mode
					cpu_Fetch_Wait = Wait_State_Access_32(cpu_Temp_Addr, cpu_Seq_Access);

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_LS_Is_Load)
					{
						cpu_Regs[cpu_Temp_Reg_Ptr] = Read_Memory_32(cpu_Temp_Addr);
					}
					else
					{
						Write_Memory_32(cpu_Temp_Addr, cpu_Regs[cpu_Temp_Reg_Ptr]);
					}

					// base register is updated after the first memory access
					if (cpu_LS_First_Access && cpu_Overwrite_Base_Reg)
					{
						if (cpu_Multi_Inc)
						{
							if (cpu_Multi_Before)
							{
								cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size) - 4);
							}
							else
							{
								cpu_Regs[cpu_Base_Reg] = (uint32_t)(cpu_Temp_Addr + (4 * cpu_Multi_List_Size));
							}

							if (cpu_Special_Inc)
							{
								cpu_Special_Inc = false;
								cpu_Regs[cpu_Base_Reg] += 0x3C;
							}
						}
						else
						{
							cpu_Regs[cpu_Base_Reg] = cpu_Write_Back_Addr;
						}

						cpu_LS_First_Access = false;
						cpu_Overwrite_Base_Reg = false;
					}

					// always incrementing since addresses always start at the lowest one
					// always after because 'before' cases built into address at initialization
					cpu_Temp_Addr += 4;

					cpu_Multi_List_Ptr++;

					// if done, the next cycle depends on whether or not Reg 15 was written to
					if (cpu_Multi_List_Ptr == cpu_Multi_List_Size)
					{
						if (cpu_Multi_Swap)
						{
							cpu_Swap_Regs(cpu_Temp_Mode, false, false);
						}

						if (cpu_LS_Is_Load)
						{
							if (cpu_Regs_To_Access[cpu_Multi_List_Ptr - 1] == 15)
							{
								// Thumb mode has a special case of adding 0x40 to the base reg
								// is it maybe 16 accesses all writing to PC? (effects timing)
								cpu_Regs[15] &= 0xFFFFFFFE;

								// Invalidate instruction pipeline
								cpu_Invalidate_Pipeline = true;
								cpu_Seq_Access = false;
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
							}
							else
							{
								// if the next cycle is a memory access, one cycle can be saved
								cpu_Instr_Type = cpu_Internal_Can_Save_TMB;
								cpu_Internal_Save_Access = true;
								cpu_Seq_Access = false;
							}
						}
						else
						{
							// when writing, there is no last internal cycle, proceed to the next instruction
							// also check for interrupts
							cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
							cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

							if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
							else { cpu_Decode_TMB(); }

							cpu_Seq_Access = false;
						}
					}
					else
					{
						cpu_Seq_Access = true;
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Multiply_TMB:
				// Multiplication with possibly early termination
				if (cpu_Fetch_Cnt == 0)
				{
					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_TMB();
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
					cpu_Regs[15] += 2;

					cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
					cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

					cpu_Instr_Type = cpu_Multiply_Cycles;

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = true;
				}
				break;

			case cpu_Prefetch_And_Branch_Ex_TMB:
				// Branch from Thumb mode to ARM mode, no interrupt check due to pipeline invalidation
				if (cpu_Fetch_Cnt == 0)
				{
					// start in thumb mode, always branch
					cpu_Take_Branch = true;

					cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);

					cpu_Execute_Internal_Only_TMB();
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);

					cpu_Regs[15] = cpu_Temp_Reg;

					cpu_FlagTset((cpu_Regs[cpu_Base_Reg] & 1) == 1);
					cpu_Thumb_Mode = cpu_FlagTget();

					// Invalidate instruction pipeline
					if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
					else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;

					cpu_Seq_Access = false;
				}
				break;

			case cpu_Prefetch_And_SWI_Undef:
				// This code path is the exception pretech cycle for SWI and undefined instructions
				if (cpu_Fetch_Cnt == 0)
				{
					if (cpu_Thumb_Mode)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);
					}
					else
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);

						// In ARM mode, we might not actually generate an exception if the condition code fails
						cpu_Execute_Internal_Only_ARM();
					}

					cpu_IRQ_Input_Use = cpu_IRQ_Input;
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_Thumb_Mode || cpu_ARM_Cond_Passed)
					{
						if (cpu_Thumb_Mode)
						{
							cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
						}
						else
						{
							cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
						}

						cpu_Seq_Access = false;

						if (cpu_Exception_Type == cpu_SWI_Exc)
						{
							if (TraceCallback) TraceCallback(1); // SWI

							// supervisor mode
							cpu_Swap_Regs(0x13, true, false);

							// R14 becomes return address
							cpu_Regs[14] = cpu_Thumb_Mode ? (cpu_Regs[15] - 2) : (cpu_Regs[15] - 4);

							// take exception vector 0x8
							cpu_Regs[15] = 0x00000008;
						}
						else
						{
							if (TraceCallback) TraceCallback(2); // UDF

							// undefined instruction mode
							cpu_Swap_Regs(0x1B, true, false);

							// R14 becomes return address
							cpu_Regs[14] = cpu_Thumb_Mode ? (cpu_Regs[15] - 2) : (cpu_Regs[15] - 4);

							// take exception vector 0x4
							cpu_Regs[15] = 0x00000004;
						}

						// Normal Interrupts disabled
						cpu_FlagIset(true);

						// switch into ARM mode
						cpu_Thumb_Mode = false;
						cpu_FlagTset(false);

						// Invalidate instruction pipeline
						cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
					}
					else
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);

						cpu_Regs[15] += 4;

						cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
						cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

						cpu_Decode_ARM();
						cpu_Seq_Access = true;

						if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					}

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Prefetch_IRQ:
				// IRQ uses a prefetch cycle not an internal cycle (just like swi and undef)
				if (cpu_Fetch_Cnt == 0)
				{
					if (TraceCallback) TraceCallback(3); // IRQ

					if (cpu_Thumb_Mode)
					{
						cpu_Fetch_Wait = Wait_State_Access_16_Instr(cpu_Regs[15], cpu_Seq_Access);
					}
					else
					{
						cpu_Fetch_Wait = Wait_State_Access_32_Instr(cpu_Regs[15], cpu_Seq_Access);
					}
				}

				cpu_Fetch_Cnt += 1;

				if (cpu_Fetch_Cnt == cpu_Fetch_Wait)
				{
					if (cpu_Thumb_Mode)
					{
						cpu_Instr_TMB_0 = Read_Memory_16(cpu_Regs[15]);
					}
					else
					{
						cpu_Instr_ARM_0 = Read_Memory_32(cpu_Regs[15]);
					}
				
					// IRQ mode
					cpu_Swap_Regs(0x12, true, false);

					// R14 becomes return address + 4
					if (cpu_Thumb_Mode)
					{
						cpu_Regs[14] = cpu_Regs[15];
					}
					else
					{
						cpu_Regs[14] = cpu_Regs[15] - 4;
					}

					// Normal Interrupts disabled
					cpu_FlagIset(true);

					// take exception vector 0x18
					cpu_Regs[15] = 0x00000018;

					// switch into ARM mode
					cpu_Thumb_Mode = false;
					cpu_FlagTset(false);

					// Invalidate instruction pipeline
					cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;

					cpu_Seq_Access = false;

					cpu_Fetch_Cnt = 0;
					cpu_Fetch_Wait = 0;
				}
				break;

			case cpu_Internal_Reset_1:
				cpu_Instr_Type = cpu_Internal_Reset_2;
				cpu_Exception_Type = cpu_Reset_Exc;
				break;

			case cpu_Internal_Reset_2:
				// switch into ARM mode
				cpu_Thumb_Mode = false;
				cpu_FlagTset(false);

				// Invalidate instruction pipeline
				cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
				break;

			case cpu_Internal_And_Branch_4_ARM:
				// This code path comes from an ALU instruction in ARM mode using R15 as the destination register
				// and is the second half of the (implied) branch. No memory access, and no possible cycle save due to branch
				cpu_Execute_Internal_Only_ARM();

				cpu_IRQ_Input_Use = cpu_IRQ_Input;

				cpu_FlagI_Old = cpu_FlagIget();

				// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
				if (cpu_ALU_S_Bit)
				{
					if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
					{
						//Console.WriteLine("using reg swap on bad mode");
					}
					else
					{
						//upper bit of mode must always be set
						cpu_Swap_Regs((cpu_Regs[17] & 0x1F) | 0x10, false, true);
					}

					if (cpu_FlagTget()) { cpu_Thumb_Mode = true; cpu_Clear_Pipeline = true; }
					else { cpu_Thumb_Mode = false; }

					cpu_ALU_S_Bit = false;
				}

				// Invalidate instruction pipeline if necessary
				if (cpu_Clear_Pipeline)
				{
					if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
					else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }
					cpu_Seq_Access = false;
				}
				else
				{
					cpu_Regs[15] += 4;

					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }

					// instructions with internal cycles revert to non-sequential accesses 
					cpu_Seq_Access = false;
				}
				break;

			case cpu_Internal_Can_Save_ARM:
				// Last Internal cycle of an instruction, note that the actual operation was already completed
				// This cycle is interruptable
				// acording to ARM documentation, this cycle can be combined with the following memory access
				// but it appears that the GBA does not do so
				cpu_IRQ_Input_Use = cpu_IRQ_Input;

				if (cpu_Invalidate_Pipeline)
				{
					cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
				}
				else
				{
					// A memory access cycle could be saved here, but the GBA does not seem to implement it
					if (cpu_Internal_Save_Access) { }

					// next instruction was already prefetched, decode it here
					cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
					cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_ARM(); }
				}

				cpu_Internal_Save_Access = false;
				cpu_Invalidate_Pipeline = false;
				break;

			case cpu_Internal_Can_Save_TMB:
				// Last Internal cycle of an instruction, note that the actual operation was already completed
				// This cycle is interruptable
				// acording to ARM documentation, this cycle can be combined with the following memory access
				// but it appears that the GBA does not do so				
				cpu_IRQ_Input_Use = cpu_IRQ_Input;

				if (cpu_Invalidate_Pipeline)
				{
					cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;
				}
				else
				{
					// A memory access cycle could be saved here, but the GBA does not seem to implement it
					if (cpu_Internal_Save_Access) { }

					// next instruction was already prefetched, decode it here
					cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
					cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else { cpu_Decode_TMB(); }
				}

				cpu_Internal_Save_Access = false;
				cpu_Invalidate_Pipeline = false;
				break;

			case cpu_LDM_Glitch_Mode_Execute:
				cpu_LDM_Glitch_Tick();
				break;

			case cpu_Internal_Halted:
				if (cpu_Trigger_Unhalt)
				{
					if (!Halt_Leave)
					{
						Halt_Leave = true;
						Halt_Leave_cd = 2;
						IRQ_Delays = true;
						delays_to_process = true;
					}
				}
				break;

			case cpu_Multiply_Cycles:
				// cycles of the multiply instruction
				// check for IRQs at the end
				cpu_Mul_Cycles_Cnt += 1;

				if (cpu_Mul_Cycles_Cnt == cpu_Mul_Cycles)
				{
					cpu_IRQ_Input_Use = cpu_IRQ_Input;

					if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
					else if (cpu_Thumb_Mode) { cpu_Decode_TMB(); }
					else { cpu_Decode_ARM(); }

					cpu_Mul_Cycles_Cnt = 0;

					// Multiply forces the next access to be non-sequential
					cpu_Seq_Access = false;
				}
				break;

				// Check timing?
			case cpu_Pause_For_DMA:
				if (dma_Held_CPU_Instr >= 42)
				{
					switch (dma_Held_CPU_Instr)
					{
						case cpu_Internal_And_Branch_4_ARM:
							// This code path comes from an ALU instruction in ARM mode using R15 as the destination register
							// and is the second half of the (implied) branch. No memory access, and no possible cycle save due to branch
							cpu_Execute_Internal_Only_ARM();

							cpu_IRQ_Input_Use = cpu_IRQ_Input;

							cpu_FlagI_Old = cpu_FlagIget();

							// if S bit set in the instruction (can only happen in ARM mode) copy SPSR to CPSR
							if (cpu_ALU_S_Bit)
							{
								if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
								{
									//Console.WriteLine("using reg swap on bad mode");
								}
								else
								{
									//upper bit of mode must always be set
									cpu_Swap_Regs((cpu_Regs[17] & 0x1F) | 0x10, false, true);
								}

								if (cpu_FlagTget()) { cpu_Thumb_Mode = true; cpu_Clear_Pipeline = true; }
								else { cpu_Thumb_Mode = false; }

								cpu_ALU_S_Bit = false;
							}

							// Invalidate instruction pipeline if necessary
							if (cpu_Clear_Pipeline)
							{
								if (cpu_Thumb_Mode) { cpu_Instr_Type = cpu_Prefetch_Only_1_TMB; }
								else { cpu_Instr_Type = cpu_Prefetch_Only_1_ARM; }
								cpu_Seq_Access = false;
							}
							else
							{
								cpu_Regs[15] += 4;

								cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
								cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

								if (cpu_IRQ_Input_Use && !cpu_FlagI_Old) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
								else { cpu_Decode_ARM(); }

								// instructions with internal cycles revert to non-sequential accesses 
								cpu_Seq_Access = false;
							}
							break;

						case cpu_Internal_Can_Save_ARM:
							// Last Internal cycle of an instruction, note that the actual operation was already completed
							// This cycle is interruptable
							// acording to ARM documentation, this cycle can be combined with the following memory access
							// but it appears that the GBA does not do so			
							cpu_IRQ_Input_Use = cpu_IRQ_Input;

							if (cpu_Invalidate_Pipeline)
							{
								cpu_Instr_Type = cpu_Prefetch_Only_1_ARM;
							}
							else
							{
								// A memory access cycle could be saved here, but the GBA does not seem to implement it
								if (cpu_Internal_Save_Access) { }

								// next instruction was already prefetched, decode it here
								cpu_Instr_ARM_2 = cpu_Instr_ARM_1;
								cpu_Instr_ARM_1 = cpu_Instr_ARM_0;

								if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
								else { cpu_Decode_ARM(); }
							}

							cpu_Internal_Save_Access = false;
							cpu_Invalidate_Pipeline = false;
							break;

						case cpu_Internal_Can_Save_TMB:
							// Last Internal cycle of an instruction, note that the actual operation was already completed
							// This cycle is interruptable
							// acording to ARM documentation, this cycle can be combined with the following memory access
							// but it appears that the GBA does not do so			
							cpu_IRQ_Input_Use = cpu_IRQ_Input;

							if (cpu_Invalidate_Pipeline)
							{
								cpu_Instr_Type = cpu_Prefetch_Only_1_TMB;
							}
							else
							{
								// A memory access cycle could be saved here, but the GBA does not seem to implement it
								if (cpu_Internal_Save_Access) { }

								// next instruction was already prefetched, decode it here
								cpu_Instr_TMB_2 = cpu_Instr_TMB_1;
								cpu_Instr_TMB_1 = cpu_Instr_TMB_0;

								if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
								else { cpu_Decode_TMB(); }
							}

							cpu_Internal_Save_Access = false;
							cpu_Invalidate_Pipeline = false;
							break;

						case cpu_LDM_Glitch_Mode_Execute:
							cpu_LDM_Glitch_Tick();
							break;

						case cpu_Internal_Halted:
							if (cpu_Trigger_Unhalt)
							{
								if (!Halt_Leave)
								{
									Halt_Leave = true;
									Halt_Leave_cd = 2;
									IRQ_Delays = true;
									delays_to_process = true;
								}
							}
							break;

						case cpu_Multiply_Cycles:
							// cycles of the multiply instruction
							// check for IRQs at the end
							cpu_Mul_Cycles_Cnt += 1;

							if (cpu_Mul_Cycles_Cnt == cpu_Mul_Cycles)
							{
								cpu_IRQ_Input_Use = cpu_IRQ_Input;

								if (cpu_IRQ_Input_Use && !cpu_FlagIget()) { cpu_Instr_Type = cpu_Prefetch_IRQ; }
								else if (cpu_Thumb_Mode) { cpu_Decode_TMB(); }
								else { cpu_Decode_ARM(); }

								cpu_Mul_Cycles_Cnt = 0;

								// Multiply forces the next access to be non-sequential
								cpu_Seq_Access = false;
							}
							break;
					}

					if (cpu_Instr_Type != cpu_Pause_For_DMA)
					{
						// change the DMA held instruction
						dma_Held_CPU_Instr = cpu_Instr_Type;
						cpu_Instr_Type = cpu_Pause_For_DMA;
					}
				}

				if (!cpu_Is_Paused)
				{
					cpu_Instr_Type = dma_Held_CPU_Instr;

					cpu_Restore_IRQ_Clock = true;
				}
				else
				{
					cpu_No_IRQ_Clock = true;
				}
				break;
		}
		#pragma endregion

		CycleCount += 1;
	}
}