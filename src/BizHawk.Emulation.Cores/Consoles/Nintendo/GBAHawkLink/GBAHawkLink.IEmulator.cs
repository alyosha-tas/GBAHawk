using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using System;
using System.Text;
using static BizHawk.Emulation.Cores.Nintendo.GBHawkLink.GBHawkLink;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink : IEmulator, ISoundProvider, IVideoProvider
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

			uint tracer_core = (uint)Settings.TraceSet;

			LibGBAHawkLink.GBALink_settracecallback(GBA_Pntr, tracecb, tracer_core);

			bool L_Reset = false;
			bool R_Reset = false;

			if (controller.IsPressed("P1 Power"))
			{
				L_Reset = true;
			}
			if (controller.IsPressed("P2 Power"))
			{
				R_Reset = true;
			}

			Is_Lag = LibGBAHawkLink.GBALink_frame_advance(GBA_Pntr, controller_state_1, Acc_X_state_1, Acc_Y_state_1, Solar_state_1, true, true,
																	controller_state_2, Acc_X_state_2, Acc_Y_state_2, Solar_state_2, true, true,
																	L_Reset, R_Reset);

			if (Is_Lag) { Lag_Count++; }

			LibGBAHawkLink.GBALink_get_video(GBA_Pntr, _vidbuffer_L, 0);
			LibGBAHawkLink.GBALink_get_video(GBA_Pntr, _vidbuffer_R, 1);

			for (int i = 0; i < 160; i++)
			{
				for (int j = 0; j < 240; j++)
				{
					_vidbuffer[482 * i + j] = _vidbuffer_L[240 *i + j];
				}

				_vidbuffer[482 * i + 240] = (int)0xFAFAFA;
				_vidbuffer[482 * i + 241] = (int)0xFAFAFA;

				for (int j = 0; j < 240; j++)
				{
					_vidbuffer[482 * i + 242 + j] = _vidbuffer_R[240 * i + j];
				}
			}

			_frame++;

			// Detect GBP via image
			if (SyncSettings.Use_GBP_L)
			{
				//GBP_Screen_Detection_L = GBACommonFunctions.Check_Video_GBP(_vidbuffer_L);
			}

			// Detect GBP via image
			if (SyncSettings.Use_GBP_R)
			{
				//GBP_Screen_Detection_R = GBACommonFunctions.Check_Video_GBP(_vidbuffer_R);
			}

			return true;
		}

		public void GetControllerState(IController controller)
		{
			InputCallbacks.Call();
			controller_state_1 = _controllerDeck.ReadPort1(controller);
			(Acc_X_state_1, Acc_Y_state_1) = _controllerDeck.ReadAcc1(controller);
			Solar_state_1 = _controllerDeck.ReadSolar1(controller);

			controller_state_2 = _controllerDeck.ReadPort2(controller);
			(Acc_X_state_2, Acc_Y_state_2) = _controllerDeck.ReadAcc2(controller);
			Solar_state_2 = _controllerDeck.ReadSolar2(controller);

			// override the input state from the GBP directly
			if (GBP_Screen_Detection_L)
			{
				if (GBP_Screen_Count_L < 2)
				{
					controller_state_1 = 0x3FF;

					GBP_Screen_Count_L += 1;
				}
				else
				{
					controller_state_1 = 0x30F;

					GBP_Mode_Enabled_L = true;

					//Console.WriteLine("GBP Rumble mode enabled");
					LibGBAHawkLink.GBALink_Set_GBP_Enable(GBA_Pntr, 0);

					GBP_Screen_Count_L = 0;
				}
			}

			// override the input state from the GBP directly
			if (GBP_Screen_Detection_R)
			{
				if (GBP_Screen_Count_R < 2)
				{
					controller_state_2 = 0x3FF;

					GBP_Screen_Count_R += 1;
				}
				else
				{
					controller_state_2 = 0x30F;

					GBP_Mode_Enabled_R = true;

					//Console.WriteLine("GBP Rumble mode enabled");
					LibGBAHawkLink.GBALink_Set_GBP_Enable(GBA_Pntr, 1);

					GBP_Screen_Count_R = 0;
				}
			}
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GBAL;

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
				LibGBAHawkLink.GBALink_destroy(GBA_Pntr);
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
			uint audio_core = (uint)Settings.AudioSet;

			uint f_clock = LibGBAHawkLink.GBALink_get_audio(GBA_Pntr, Aud_L, ref num_samp_L, Aud_R, ref num_samp_R, audio_core);

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

		public int[] _vidbuffer_L = new int[240 * 160];
		public int[] _vidbuffer_R = new int[240 * 160];

		public int[] _vidbuffer = new int[482 * 160];
		public int[] GetVideoBuffer()
		{
			if (Settings.VideoSet == GBALinkSettings.VideoSrc.Both)
			{
				return _vidbuffer;
			}
			else if (Settings.VideoSet == GBALinkSettings.VideoSrc.Left)
			{
				return _vidbuffer_L;
			}
			else
			{
				return _vidbuffer_R;
			}
		}

		public int VirtualWidth => (Settings.VideoSet == GBALinkSettings.VideoSrc.Both) ? 482 : 240;
		public int VirtualHeight => 160;
		public int BufferWidth => (Settings.VideoSet == GBALinkSettings.VideoSrc.Both) ? 482 : 240;
		public int BufferHeight => 160;
		public int BackgroundColor => unchecked((int)0xFF000000);
		public int VsyncNumerator => 262144;
		public int VsyncDenominator => 4389;
	}
}
