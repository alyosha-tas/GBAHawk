using BizHawk.Common.NumberExtensions;
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
		public bool controller_was_checked;
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

			Is_Lag = true;

			controller_was_checked = false;

			VBlank_Rise = false;

			do_frame(controller);

			// if the game is halted but controller interrupts are on, check for interrupts
			// if the game is stopped, any button press will un-stop even if interrupts are off
			if (stopped && !controller_was_checked)
			{
				// update the controller state on VBlank
				GetControllerState(controller);

				do_controller_check();
			}

			if (Is_Lag)
			{
				Lag_Count++;
			}

			Frame_Count++;

			return true;
		}

		public void do_frame(IController controller)
		{
			while (!VBlank_Rise)
			{
				if (delays_to_process) { process_delays(); }

				snd_Tick();
				ppu_Tick();
				if (Use_MT) { mapper.Mapper_Tick(); }
				dma_Tick();
				pre_Tick();
				ser_Tick();
				tim_Tick();
				cpu_Tick();

				CycleCount++;
			}
		}

		public void On_VBlank()
		{
			Is_Lag = false;

			controller_was_checked = true;

			// update the controller state on VBlank
			GetControllerState(Frame_Controller);

			// check if controller state caused interrupt
			do_controller_check();

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
			dma_Tick();
			pre_Tick();
			ser_Tick();
			tim_Tick();
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

							if ((INT_EN & 0x1000) == 0x1000)
							{
								if (INT_Master_On) { cpu_IRQ_Input = true; }
							}
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

							if ((INT_EN & 0x1000) == 0x1000)
							{
								if (INT_Master_On) { cpu_IRQ_Input = true; }
							}
						}
					}
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
					if (!ppu_Delays)
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
			}

			if (ppu_Delays)
			{
				if (ppu_VBL_IRQ_cd > 0)
				{
					ppu_VBL_IRQ_cd -= 1;

					if (ppu_VBL_IRQ_cd == 0)
					{
						INT_Flags |= 0x1;
						if (((INT_EN & 0x1) == 0x1) && INT_Master_On) { cpu_IRQ_Input = true; }

						// check for any additional ppu delays
						if ((ppu_HBL_IRQ_cd == 0) && (ppu_LYC_IRQ_cd == 0) && (ppu_HBL_Check_cd == 0) && (ppu_LYC_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				if (ppu_HBL_IRQ_cd > 0)
				{
					ppu_HBL_IRQ_cd -= 1;

					if (ppu_HBL_IRQ_cd == 0)
					{
						INT_Flags |= 0x2;
						if (((INT_EN & 0x2) == 0x2) && INT_Master_On) { cpu_IRQ_Input = true; }

						// check for any additional ppu delays
						if ((ppu_VBL_IRQ_cd == 0) && (ppu_LYC_IRQ_cd == 0) && (ppu_HBL_Check_cd == 0) && (ppu_LYC_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				if (ppu_LYC_IRQ_cd > 0)
				{
					ppu_LYC_IRQ_cd -= 1;

					if (ppu_LYC_IRQ_cd == 0)
					{
						INT_Flags |= 0x4;
						if (((INT_EN & 0x4) == 0x4) && INT_Master_On) { cpu_IRQ_Input = true; }

						// check for any additional ppu delays
						if ((ppu_VBL_IRQ_cd == 0) && (ppu_HBL_IRQ_cd == 0) && (ppu_HBL_Check_cd == 0) && (ppu_LYC_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				if (ppu_HBL_Check_cd > 0)
				{
					ppu_HBL_Check_cd -= 1;

					if (ppu_HBL_Check_cd == 0)
					{
						// Enter HBlank
						ppu_STAT |= 2;

						// trigger HBL IRQ
						if ((ppu_STAT & 0x10) == 0x10)
						{
							ppu_HBL_IRQ_cd = 3;
							ppu_Delays = true;
							delays_to_process = true;
						}

						// check for any additional ppu delays
						if ((ppu_VBL_IRQ_cd == 0) && (ppu_HBL_IRQ_cd == 0) && (ppu_LYC_IRQ_cd == 0) && (ppu_LYC_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				if (ppu_LYC_Check_cd > 0)
				{
					ppu_LYC_Check_cd -= 1;

					if (ppu_LYC_Check_cd == 0)
					{
						if (ppu_LY == ppu_LYC)
						{
							if ((ppu_STAT & 0x20) == 0x20)
							{
								ppu_LYC_IRQ_cd = 3;
								ppu_Delays = true;
								delays_to_process = true;
							}

							// set the flag bit
							ppu_STAT |= 4;
						}

						// check for any additional ppu delays
						if ((ppu_VBL_IRQ_cd == 0) && (ppu_HBL_IRQ_cd == 0) && (ppu_LYC_IRQ_cd == 0) && (ppu_HBL_Check_cd == 0))
						{
							ppu_Delays = false;
						}
					}
				}

				// check if all delay sources are false
				if (!IRQ_Write_Delay_3 && !IRQ_Write_Delay_2 && !IRQ_Write_Delay)
				{
					if (!ppu_Delays)
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
