using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;

using System;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBA
{
	public partial class SubGBAHawk : IEmulator, ISoundProvider, IVideoProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		public IController Controller;

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			Controller = controller;
			
			// update the controller state on VBlank
			GetControllerState(controller);

			if (Tracer.IsEnabled())
			{
				tracecb = MakeTrace;
			}
			else
			{
				tracecb = null;
			}

			LibGBAHawk.GBA_settracecallback(GBA_Pntr, tracecb);

			Is_Lag = LibGBAHawk.GBA_subframe_advance(GBA_Pntr, controller_state, Acc_X_state, Acc_Y_state, Solar_state, 
														true, true, controller.IsPressed("P1 Power"), (uint)controller.AxisValue("Reset Cycle"));

			LibGBAHawk.GBA_get_video(GBA_Pntr, _vidbuffer);

			if (Is_Lag) { Lag_Count++; }

			_frame++;

			// Detect GBP via image
			if (SyncSettings.Use_GBP)
			{
				GBP_Screen_Detection = GBACommonFunctions.Check_Video_GBP(_vidbuffer);
			}

			return true;
		}

		public void GetControllerState(IController controller)
		{
			InputCallbacks.Call();
			controller_state = _controllerDeck.ReadPort1(controller);
			(Acc_X_state, Acc_Y_state) = _controllerDeck.ReadAcc1(controller);
			Solar_state = _controllerDeck.ReadSolar1(controller);

			// override the input state from the GBP directly
			if (GBP_Screen_Detection)
			{
				if (GBP_Screen_Count < 2)
				{
					controller_state = 0x3FF;

					GBP_Screen_Count += 1;
				}
				else
				{
					controller_state = 0x3F0;

					GBP_Mode_Enabled = true;

					//Console.WriteLine("GBP Rumble mode enabled");
					LibGBAHawk.GBA_Set_GBP_Enable(GBA_Pntr);

					GBP_Screen_Count = 0;
				}
			}
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GBA;

		public bool DeterministicEmulation => true;

		public void ResetCounters()
		{
			_frame = 0;
			Lag_Count = 0;
			Is_Lag = false;
		}

		public void Dispose()
		{
			if (GBA_Pntr != IntPtr.Zero)
			{
				LibGBAHawk.GBA_destroy(GBA_Pntr);
				GBA_Pntr = IntPtr.Zero;
			}

			DisposeSound();
		}

		public BlipBuffer blip_L = new BlipBuffer(25000);
		public BlipBuffer blip_R = new BlipBuffer(25000);

		public int[] Aud_L = new int[25000];
		public int[] Aud_R = new int[25000];
		public uint num_samp_L;
		public uint num_samp_R;

		const int blipbuffsize = 9000;

		public bool CanProvideAsync => false;

		public void SetSyncMode(SyncSoundMode mode)
		{
			if (mode != SyncSoundMode.Sync)
			{
				throw new NotSupportedException("Only sync mode is supported");
			}
		}

		public void GetSamplesAsync(short[] samples)
		{
			throw new NotSupportedException("Async not supported");
		}

		public SyncSoundMode SyncMode => SyncSoundMode.Sync;

		public void GetSamplesSync(out short[] samples, out int nsamp)
		{
			uint f_clock = LibGBAHawk.GBA_get_audio(GBA_Pntr, Aud_L, ref num_samp_L, Aud_R, ref num_samp_R);

			for (int i = 0; i < num_samp_L; i++)
			{
				blip_L.AddDelta((uint)Aud_L[i * 2], Aud_L[i * 2 + 1]);
			}

			for (int i = 0; i < num_samp_R; i++)
			{
				blip_R.AddDelta((uint)Aud_R[i * 2], Aud_R[i * 2 + 1]);
			}

			//Console.WriteLine(num_samp_L + " " + num_samp_R + " " + f_clock);

			blip_L.EndFrame(f_clock);
			blip_R.EndFrame(f_clock);

			nsamp = blip_L.SamplesAvailable();
			samples = new short[nsamp * 2];

			if (nsamp != 0)
			{
				blip_L.ReadSamplesLeft(samples, nsamp);
				blip_R.ReadSamplesRight(samples, nsamp);
			}
		}

		public void DiscardSamples()
		{
			blip_L.Clear();
			blip_R.Clear();
		}

		public void DisposeSound()
		{
			blip_L.Clear();
			blip_R.Clear();
			blip_L.Dispose();
			blip_R.Dispose();
			blip_L = null;
			blip_R = null;
		}

		public int[] _vidbuffer = new int[240 * 160];

		public int[] GetVideoBuffer()
		{
			return _vidbuffer;
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
