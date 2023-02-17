using BizHawk.Emulation.Common;
using System;
using System.Runtime.InteropServices;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug : IEmulator, IVideoProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		public IController Frame_Controller;

		public ushort Acc_X_state;
		public ushort Acc_Y_state;
		public bool VBlank_Rise;
		public bool delays_to_process;
		public bool IRQ_Write_Delay, IRQ_Write_Delay_2, IRQ_Write_Delay_3;

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			//Console.WriteLine("-----------------------FRAME-----------------------");
			Frame_Controller = controller;

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
			GetControllerState(Frame_Controller);

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
				if (Use_MT) { mapper.Mapper_Tick(); }
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
			SendVideoBuffer();

			if (_settings.VBL_sync)
			{
				for (int j = 0; j < 0x40000; j++) { WRAM_vbls[j] = WRAM[j]; }
				for (int j = 0; j < 0x8000; j++) { IWRAM_vbls[j] = IWRAM[j]; }
				for (int j = 0; j < 0x400; j++) { PALRAM_vbls[j] = PALRAM[j]; }
				for (int j = 0; j < 0x18000; j++) { VRAM_vbls[j] = VRAM[j]; }
				for (int j = 0; j < 0x400; j++) { OAM_vbls[j] = OAM[j]; }

				if (cart_RAM != null)
				{
					for (int j = 0; j < cart_RAM.Length; j++) { cart_RAM_vbls[j] = cart_RAM[j]; }
				}
			}
		}

		public void do_single_step()
		{
			if (delays_to_process) { process_delays(); }

			snd_Tick();
			ppu_Tick();
			if (Use_MT) { mapper.Mapper_Tick(); }
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
					key_Delay_cd = 2;
					delays_to_process = true;
				}
			}
		}

		public void process_delays()
		{
			if (IRQ_Write_Delay)
			{
				cpu_IRQ_Input = cpu_Next_IRQ_Input;
				IRQ_Write_Delay = false;

				// check if all delay sources are false
				if (!IRQ_Write_Delay_3 && !IRQ_Write_Delay_2)
				{
					if (!ppu_Delays && !ser_Delay && !key_Delay)
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
					if (!IRQ_Write_Delay_3 && !IRQ_Write_Delay_2 && !IRQ_Write_Delay)
					{
						if (!ppu_Delays && !key_Delay)
						{
							delays_to_process = false;
						}
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
					if (!IRQ_Write_Delay_3 && !IRQ_Write_Delay_2 && !IRQ_Write_Delay)
					{
						if (!ppu_Delays && !ser_Delay)
						{
							delays_to_process = false;
						}
					}
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
						if (dma_Go[0] && dma_Start_VBL[0]) { dma_Run[0] = true; dma_External_Source[0] = true; }
						if (dma_Go[1] && dma_Start_VBL[1]) { dma_Run[1] = true; dma_External_Source[1] = true; }
						if (dma_Go[2] && dma_Start_VBL[2]) { dma_Run[2] = true; dma_External_Source[2] = true; }
						if (dma_Go[3] && dma_Start_VBL[3]) { dma_Run[3] = true; dma_External_Source[3] = true; }					
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
							if (dma_Go[0] && dma_Start_HBL[0]) { dma_Run[0] = true; dma_External_Source[0] = true; }
							if (dma_Go[1] && dma_Start_HBL[1]) { dma_Run[1] = true; dma_External_Source[1] = true; }
							if (dma_Go[2] && dma_Start_HBL[2]) { dma_Run[2] = true; dma_External_Source[2] = true; }
							if (dma_Go[3] && dma_Start_HBL[3]) { dma_Run[3] = true; dma_External_Source[3] = true; }						
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
						// but not parameters A-D
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

								ppu_Rendering_Complete = false;
								ppu_PAL_Rendering_Complete = false;

								for (int i = 0; i < 4; i++)
								{
									ppu_BG_X_Latch[i] = ppu_BG_X[i];
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
					}
					else if (ppu_LYC_Vid_Check_cd == 0)
					{
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
								dma_External_Source[3] = true;
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
				if (!IRQ_Write_Delay_3 && !IRQ_Write_Delay_2 && !IRQ_Write_Delay)
				{
					if (!ppu_Delays && !ser_Delay && !key_Delay)
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

		public int[] frame_buffer;


		public uint[] vid_buffer;


		public int[] GetVideoBuffer()
		{
			return frame_buffer;
		}

		public void SendVideoBuffer()
		{
			for (int j = 0; j < frame_buffer.Length; j++) 
			{ 
				frame_buffer[j] = (int)vid_buffer[j];
				vid_buffer[j] = 0xFFFFFFFF;
			}
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
