using BizHawk.Emulation.Common;
using System;
using System.Runtime.InteropServices;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug : IEmulator, IVideoProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		public uint PALRAM_32W_Addr, VRAM_32W_Addr;
		public ushort PALRAM_32W_Value, VRAM_32W_Value;

		public ushort Acc_X_state;
		public ushort Acc_Y_state;
		public byte Solar_state;
		public bool VBlank_Rise;
		public bool delays_to_process;
		public bool IRQ_Write_Delay, IRQ_Write_Delay_2, IRQ_Write_Delay_3;

		public bool VRAM_32_Check, PALRAM_32_Check;
		public bool VRAM_32_Delay, PALRAM_32_Delay;

		public bool IRQ_Delays, Misc_Delays;

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			//Console.WriteLine("-----------------------FRAME-----------------------");
			for (int j = 0; j < vid_buffer.Length; j++)
			{
				vid_buffer[j] = unchecked((int)0xFFFFFFFF);
			}

			if (_tracer.IsEnabled())
			{
				TraceCallback = s => _tracer.Put(s);
			}
			else
			{
				TraceCallback = null;
			}

			if (controller.IsPressed("P1 Power"))
			{
				HardReset();
			}

			// update the controller state on VBlank
			GetControllerState(controller);

			// as long as not in stop mode, vblank will occur and the controller will be checked
			if (VBlank_Rise || stopped)
			{
				// check if controller state caused interrupt
				do_controller_check();
			}

			Is_Lag = true;

			VBlank_Rise = false;

			do_frame();

			if (Is_Lag)
			{
				Lag_Count++;
			}

			Frame_Count++;

			return true;
		}

		public void do_frame()
		{
			while (!VBlank_Rise)
			{
				if (delays_to_process) { process_delays(); }

				snd_Tick();
				ppu_Tick();
				ser_Tick();
				tim_Tick();
				pre_Tick();
				dma_Tick();
				cpu_Tick();

				CycleCount++;
			}
		}

		public void On_VBlank()
		{
			// send the image on VBlank
		}

		public void do_single_step()
		{
			if (delays_to_process) { process_delays(); }

			snd_Tick();
			ppu_Tick();
			ser_Tick();
			tim_Tick();
			pre_Tick();
			dma_Tick();
			cpu_Tick();

			CycleCount++;
		}

		public void do_controller_check()
		{
			if ((key_CTRL & 0x4000) == 0x4000)
			{
				if ((key_CTRL & 0x8000) == 0x8000)
				{
					if ((key_CTRL & ~controller_state & 0x3FF) == (key_CTRL & 0x3FF))
					{
						// doesn't trigger an interrupt if no keys are selected. (see joypad.gba test rom)
						if ((key_CTRL & 0x3FF) != 0)
						{
							INT_Flags |= 0x1000;

							key_Delay = true;
							Misc_Delays = true;
							key_Delay_cd = 2;
							delays_to_process = true;
						}					
					}
				}
				else
				{
					if ((key_CTRL & ~controller_state & 0x3FF) != 0)
					{
						// doesn't trigger an interrupt if all keys are selected. (see megaman and bass)
						if ((key_CTRL & 0x3FF) != 0x3FF)
						{
							INT_Flags |= 0x1000;

							key_Delay = true;
							Misc_Delays = true;
							key_Delay_cd = 2;
							delays_to_process = true;
						}
					}
				}
			}
		}

		// only on writes, it is possible to trigger an interrupt with and mode and no keys selected or pressed
		public void do_controller_check_glitch()
		{
			if ((key_CTRL & 0xC3FF) == 0xC000)
			{
				if ((controller_state & 0x3FF) == 0x3FF)
				{
					INT_Flags |= 0x1000;

					key_Delay = true;
					Misc_Delays = true;
					key_Delay_cd = 2;
					delays_to_process = true;
				}
			}
		}

		public void process_delays()
		{
			if (IRQ_Delays)
			{
				if (IRQ_Write_Delay)
				{
					cpu_IRQ_Input = cpu_Next_IRQ_Input;
					IRQ_Write_Delay = false;

					// check if all delay sources are false
					if (!IRQ_Write_Delay_3 && !IRQ_Write_Delay_2)
					{
						IRQ_Delays = false;
						
						if (!ppu_Delays && !Misc_Delays && !ppu_Sprite_Delays)
						{
							delays_to_process = false;
						}
					}
				}

				if (IRQ_Write_Delay_2)
				{
					cpu_Next_IRQ_Input = cpu_Next_IRQ_Input_2;
					IRQ_Write_Delay = true;
					IRQ_Write_Delay_2 = false;
				}

				if (IRQ_Write_Delay_3)
				{
					cpu_Next_IRQ_Input_2 = cpu_Next_IRQ_Input_3;
					IRQ_Write_Delay_2 = true;
					IRQ_Write_Delay_3 = false;

					// in any case, if the flags and enable registers no longer have any bits in common, the cpu can no longer be unhalted
					if ((INT_EN & INT_Flags & 0x3FFF) == 0)
					{
						cpu_Trigger_Unhalt = false;
					}
					else
					{
						cpu_Trigger_Unhalt = true;
					}
				}
			}

			if (Misc_Delays)
			{
				if (VRAM_32_Delay)
				{
					if (!VRAM_32_Check)
					{
						// always write first 16 bits when not blocked
						if (!ppu_VRAM_Access)
						{
							// Forced Align
							VRAM_32W_Addr &= 0xFFFFFFFC;

							if ((VRAM_32W_Addr & 0x00010000) == 0x00010000)
							{
								VRAM[VRAM_32W_Addr & 0x17FFF] = (byte)(VRAM_32W_Value & 0xFF);
								VRAM[(VRAM_32W_Addr & 0x17FFF) + 1] = (byte)((VRAM_32W_Value >> 8) & 0xFF);
							}
							else
							{
								VRAM[VRAM_32W_Addr & 0xFFFF] = (byte)(VRAM_32W_Value & 0xFF);
								VRAM[(VRAM_32W_Addr & 0xFFFF) + 1] = (byte)((VRAM_32W_Value >> 8) & 0xFF);
							}
						}
					}
					else
					{
						VRAM_32_Delay = false;

						// check if all delay sources are false
						if (!key_Delay && !ser_Delay && !PALRAM_32_Delay)
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

							PALRAM[PALRAM_32W_Addr & 0x3FF] = (byte)(PALRAM_32W_Value & 0xFF);
							PALRAM[(PALRAM_32W_Addr & 0x3FF) + 1] = (byte)((PALRAM_32W_Value >> 8) & 0xFF);
						}
					}
					else
					{
						PALRAM_32_Delay = false;

						// check if all delay sources are false
						if (!key_Delay && !ser_Delay && !VRAM_32_Delay)
						{
							Misc_Delays = false;
						}
					}
				}

				if (ser_Delay)
				{
					ser_Delay_cd--;

					if (ser_Delay_cd == 0)
					{
						// trigger IRQ
						if (((INT_EN & INT_Flags & 0x80) == 0x80))
						{
							cpu_Trigger_Unhalt = true;
							if (INT_Master_On) { cpu_IRQ_Input = true; }
						}

						ser_Delay = false;

						// check if all delay sources are false
						if (!key_Delay && !PALRAM_32_Delay && !VRAM_32_Delay)
						{
							Misc_Delays = false;
						}
					}
				}

				if (key_Delay)
				{
					key_Delay_cd--;

					if (key_Delay_cd == 0)
					{
						// trigger IRQ
						if (((INT_EN & INT_Flags & 0x1000) == 0x1000))
						{
							cpu_Trigger_Unhalt = true;
							if (INT_Master_On) { cpu_IRQ_Input = true; }
						}

						key_Delay = false;

						// check if all delay sources are false
						if (!ser_Delay && !PALRAM_32_Delay && !VRAM_32_Delay)
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

					if (ppu_VBL_IRQ_cd == 2)
					{
						if ((ppu_STAT & 0x8) == 0x8) { INT_Flags |= 0x1; }				
					}
					else if (ppu_VBL_IRQ_cd == 1)
					{
						// trigger any DMAs with VBlank as a start condition
						if (dma_Go[0] && dma_Start_VBL[0]) { dma_Run[0] = true; }
						if (dma_Go[1] && dma_Start_VBL[1]) { dma_Run[1] = true; }
						if (dma_Go[2] && dma_Start_VBL[2]) { dma_Run[2] = true; }
						if (dma_Go[3] && dma_Start_VBL[3]) { dma_Run[3] = true; }					
					}
					else if (ppu_VBL_IRQ_cd == 0)
					{
						if (((INT_EN & INT_Flags & 0x1) == 0x1)) 
						{
							cpu_Trigger_Unhalt = true;
							if (INT_Master_On) { cpu_IRQ_Input = true; }			
						}

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

					if (ppu_HBL_IRQ_cd == 2)
					{
						// trigger HBL IRQ
						if ((ppu_STAT & 0x10) == 0x10) { INT_Flags |= 0x2; }							
					}	
					else if (ppu_HBL_IRQ_cd == 1)
					{
						// trigger any DMAs with HBlank as a start condition
						// but not if in vblank
						if (ppu_LY < 160)
						{
							if (dma_Go[0] && dma_Start_HBL[0]) { dma_Run[0] = true; }
							if (dma_Go[1] && dma_Start_HBL[1]) { dma_Run[1] = true; }
							if (dma_Go[2] && dma_Start_HBL[2]) { dma_Run[2] = true; }
							if (dma_Go[3] && dma_Start_HBL[3]) { dma_Run[3] = true; }						
						}
					}
					else if (ppu_HBL_IRQ_cd == 0)
					{
						if (((INT_EN & INT_Flags & 0x2) == 0x2))
						{
							cpu_Trigger_Unhalt = true;
							if (INT_Master_On) { cpu_IRQ_Input = true; }
						}

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

					if (ppu_LYC_IRQ_cd == 2)
					{
						if ((ppu_STAT & 0x20) == 0x20) { INT_Flags |= 0x4; }					
					}
					else if (ppu_LYC_IRQ_cd == 0)
					{
						if (((INT_EN & INT_Flags & 0x4) == 0x4))
						{
							cpu_Trigger_Unhalt = true;
							if (INT_Master_On) { cpu_IRQ_Input = true; }
						}

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
						if ((ppu_LY < 160) && !ppu_Forced_Blank)
						{
							if (ppu_BG_Mode > 0)
							{
								ppu_BG_Ref_X_Latch[2] = ppu_BG_Ref_X[2];
								ppu_BG_Ref_Y_Latch[2] = ppu_BG_Ref_Y[2];

								ppu_BG_Ref_X_Latch[3] = ppu_BG_Ref_X[3];
								ppu_BG_Ref_Y_Latch[3] = ppu_BG_Ref_Y[3];

								ppu_Convert_Offset_to_float(2);
								ppu_Convert_Offset_to_float(3);
							}

							ppu_BG_Mosaic_X_Mod = ppu_BG_Mosaic_X;

							ppu_Rendering_Complete = false;
							ppu_PAL_Rendering_Complete = false;

							for (int i = 0; i < 4; i++)
							{
								ppu_BG_X_Latch[i] = (ushort)(ppu_BG_X[i] & 0xFFF8);
								ppu_BG_Y_Latch[i] = ppu_BG_Y[i];

								ppu_Fetch_Count[i] = 0;

								ppu_Scroll_Cycle[i] = 0;

								ppu_Pixel_Color[i] = 0;

								ppu_BG_Has_Pixel[i] = false;
							}

							if (ppu_BG_Mode <= 1)
							{
								ppu_BG_Start_Time[0] = (ushort)(32 - 4 * (ppu_BG_X[0] & 0x7));
								ppu_BG_Start_Time[1] = (ushort)(32 - 4 * (ppu_BG_X[1] & 0x7));

								ppu_BG_Rendering_Complete[0] = !ppu_BG_On[0];
								ppu_BG_Rendering_Complete[1] = !ppu_BG_On[1];

								if (ppu_BG_Mode == 0)
								{
									ppu_BG_Start_Time[2] = (ushort)(32 - 4 * (ppu_BG_X[2] & 0x7));
									ppu_BG_Start_Time[3] = (ushort)(32 - 4 * (ppu_BG_X[3] & 0x7));

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
					else if (ppu_LYC_Vid_Check_cd == 0)
					{
						// Latch C-D rotation scaling parameters here.
						// Seems like X - parameters are not latched but Y - parameters are not?
						ppu_BG_Rot_C_Latch[2] = ppu_BG_Rot_C[2];
						ppu_BG_Rot_C_Latch[3] = ppu_BG_Rot_C[3];
						ppu_BG_Rot_D_Latch[2] = ppu_BG_Rot_D[2];
						ppu_BG_Rot_D_Latch[3] = ppu_BG_Rot_D[3];

						if ((ppu_LY < 160) && !ppu_Forced_Blank)
						{
							if (ppu_BG_Mode > 0)
							{
								ppu_Convert_Rotation_to_float_CD(2);
								ppu_Convert_Rotation_to_float_CD(3);
							}
						}

						// video capture DMA, check timing
						if (dma_Go[3] && dma_Start_Snd_Vid[3])
						{
							// only starts on scanline 2
							if (ppu_LY == 2)
							{
								dma_Video_DMA_Start = true;
							}

							if ((ppu_LY >= 2) && (ppu_LY < 162) && dma_Video_DMA_Start)
							{
								dma_Run[3] = true;
							}

							if (ppu_LY == 162)
							{
								dma_Video_DMA_Start = false;
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

					ppu_Sprite_Eval_Finished = true;

					if ((ppu_LY < 159) || (ppu_LY == 227))
					{
						ppu_Sprite_Eval_Finished = !ppu_OBJ_On;

						ppu_Sprite_LY_Check = (byte)(ppu_LY + 1);

						if (ppu_LY == 227) { ppu_Sprite_LY_Check = 0; }
					}

					// reset obj window detection for the scanline
					for (int i = ppu_Sprite_ofst_eval; i < (240 + ppu_Sprite_ofst_eval); i++)
					{
						ppu_Sprite_Pixels[i] = 0;
						ppu_Sprite_Priority[i] = 0;
						ppu_Sprite_Pixel_Occupied[i] = false;
						ppu_Sprite_Semi_Transparent[i] = false;
						ppu_Sprite_Object_Window[i] = false;
					}

					ppu_Sprite_Render_Cycle = 0;

					ppu_Sprite_Delays = false;

					if (!ppu_Delays && !Misc_Delays && !IRQ_Delays)
					{
						delays_to_process = false;
					}
				}
			}
		}

		public void GetControllerState(IController controller)
		{
			InputCallbacks.Call();
			controller_state = _controllerDeck.ReadPort1(controller);
			(Acc_X_state, Acc_Y_state) = _controllerDeck.ReadAcc1(controller);
			Solar_state = _controllerDeck.ReadSolar1(controller);
		}

		public int Frame => Frame_Count;

		public string SystemId => VSystemID.Raw.GBA;

		public bool DeterministicEmulation { get; set; }

		public void ResetCounters()
		{
			Frame_Count = 0;
			Lag_Count = 0;
			Is_Lag = false;
		}

		public void Dispose()
		{
			Marshal.FreeHGlobal(Mem_Domains.vram);
			Marshal.FreeHGlobal(Mem_Domains.oam);
			Marshal.FreeHGlobal(Mem_Domains.mmio);
			Marshal.FreeHGlobal(Mem_Domains.palram);

			DisposeSound();
		}

		public int[] vid_buffer;

		public int[] GetVideoBuffer()
		{			
			return vid_buffer;
		}

		public int VirtualWidth => 240;
		public int VirtualHeight => 160;
		public int BufferWidth => 240;
		public int BufferHeight => 160;
		public int BackgroundColor => unchecked((int)0xFF000000);
		public int VsyncNumerator => 262144;
		public int VsyncDenominator => 4389;
	}
}
