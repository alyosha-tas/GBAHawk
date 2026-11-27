using BizHawk.Common.NumberExtensions;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBCHawk
{
	public partial class GBCHawk : IEmulator, IVideoProvider, ISoundProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		public byte controller_state;
		public ushort Acc_X_state, Acc_Y_state;

		public bool Current_sync_on_vbl;

		public IController Controller;

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			Current_sync_on_vbl = Settings.VBL_sync;

			LibGBCHawk.GBC_Sync_Domain_VBL(GBC_Pntr, Settings.VBL_sync);

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

			LibGBCHawk.GBC_settracecallback(GBC_Pntr, tracecb);

			if (controller.IsPressed("P1 Power"))
			{
				HardReset();
			}

			Is_Lag = LibGBCHawk.GBC_frame_advance(GBC_Pntr, controller_state, Acc_X_state, Acc_Y_state, true, true);

			LibGBCHawk.GBC_get_video(GBC_Pntr, _vidbuffer);

			if (Is_Lag) { Lag_Count++; }

			_frame++;

			return true;
		}

		public void GetControllerState(IController controller)
		{
			InputCallbacks.Call();
			controller_state = _controllerDeck.ReadPort1(controller);
			(Acc_X_state, Acc_Y_state) = _controllerDeck.ReadAcc1(controller);
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GB;

		public void ResetCounters()
		{
			_frame = 0;
			Lag_Count = 0;
			Is_Lag = false;
		}

		public void Dispose()
		{
			if (GBC_Pntr != IntPtr.Zero)
			{
				LibGBCHawk.GBC_destroy(GBC_Pntr);
				GBC_Pntr = IntPtr.Zero;
			}

			DisposeSound();
		}

		public BlipBuffer blip_L = new BlipBuffer(15000);
		public BlipBuffer blip_R = new BlipBuffer(15000);

		public int[] Aud_L = new int[15000];
		public int[] Aud_R = new int[15000];
		public uint num_samp_L;
		public uint num_samp_R;

		public void GetSamplesSync(out short[] samples, out int nsamp)
		{
			uint f_clock = LibGBCHawk.GBC_get_audio(GBC_Pntr, Aud_L, ref num_samp_L, Aud_R, ref num_samp_R);

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

		public int[] _vidbuffer = new int[160 * 144];

		public int[] GetVideoBuffer()
		{
			return _vidbuffer;
		}

		public int VirtualWidth => 160;
		public int VirtualHeight => 144;
		public int BufferWidth => 160;
		public int BufferHeight => 144;
		public int BackgroundColor => unchecked((int)0xFF000000);
		public int VsyncNumerator => 262144;
		public int VsyncDenominator => 4389;
	}
}
