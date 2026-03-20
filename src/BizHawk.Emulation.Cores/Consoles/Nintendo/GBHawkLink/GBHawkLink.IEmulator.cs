using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using System;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	public partial class GBHawkLink : IEmulator, ISoundProvider, IVideoProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		// not stated used to load data to cores
		bool[] do_Resets = new bool[4];
		bool[] do_Renders = new bool[4];
		bool[] do_Sounds = new bool[4];
		byte[] f_cntrls = new byte[4];
		ushort[] f_accxs = new ushort[4];
		ushort[] f_accys = new ushort[4];

		bool[] get_console_audios = new bool[4];

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			//Update the color palette if a setting changed

			for (int i = 0; i < Num_ROMS; i++)
			{
				switch (i)
				{
					case 0: LibGBHawkLink.GBLink_load_Palette(GBLink_Pntr, Settings.A_Palette == GBLinkSettings.PaletteType.BW, (uint)i); break;
					case 1: LibGBHawkLink.GBLink_load_Palette(GBLink_Pntr, Settings.B_Palette == GBLinkSettings.PaletteType.BW, (uint)i); break;
					case 2: LibGBHawkLink.GBLink_load_Palette(GBLink_Pntr, Settings.C_Palette == GBLinkSettings.PaletteType.BW, (uint)i); break;
					case 3: LibGBHawkLink.GBLink_load_Palette(GBLink_Pntr, Settings.D_Palette == GBLinkSettings.PaletteType.BW, (uint)i); break;
				}
			}

			if (Tracer.IsEnabled())
			{
				tracecb = MakeTrace;
			}
			else
			{
				tracecb = null;
			}

			uint tracer_core = (uint)Settings.TraceSet;

			LibGBHawkLink.GBLink_settracecallback(GBLink_Pntr, tracecb, tracer_core);

			InputCallbacks.Call();

			for (int i = 0; i < Num_ROMS; i++)
			{
				f_cntrls[i] = _controllerDeck.ReadPort(controller, i);
				(f_accxs[i], f_accys[i]) = _controllerDeck.ReadAcc(controller, i);

				do_Renders[i] = true;
				do_Sounds[i] = true;
			}

			do_Resets[0] = controller.IsPressed("P1 Power");
			do_Resets[1] = controller.IsPressed("P2 Power");
			do_Resets[2] = controller.IsPressed("P3 Power");
			do_Resets[3] = controller.IsPressed("P4 Power");

			if (Num_ROMS == 2)
			{
				bool cablediscosignalNew = controller.IsPressed("Toggle Cable");
				if (cablediscosignalNew && !_cablediscosignal)
				{
					_cableconnected = !_cableconnected;
					Console.WriteLine("Cable connect status to {0}", _cableconnected);

					LibGBHawkLink.GBLink_change_linking(GBLink_Pntr, _cableconnected, 0);
				}

				_cablediscosignal = cablediscosignalNew;
			}
			else if (Num_ROMS == 3)
			{
				if (controller.IsPressed("Toggle Cable AC") | controller.IsPressed("Toggle Cable BC") | controller.IsPressed("Toggle Cable AB"))
				{
					// if any connection exists, disconnect it
					// otherwise connect in order of precedence
					// only one event can happen per frame, either a connection or disconnection
					if (_cableconnected_AC | _cableconnected_BC | _cableconnected_AB)
					{
						LibGBHawkLink.GBLink_change_linking(GBLink_Pntr, false, 1);
					}
					else if (controller.IsPressed("Toggle Cable AC"))
					{
						LibGBHawkLink.GBLink_change_linking(GBLink_Pntr, true, 4);
					}
					else if (controller.IsPressed("Toggle Cable BC"))
					{
						LibGBHawkLink.GBLink_change_linking(GBLink_Pntr, true, 3);
					}
					else if (controller.IsPressed("Toggle Cable AB"))
					{
						LibGBHawkLink.GBLink_change_linking(GBLink_Pntr, true, 2);
					}

					Console.WriteLine("Cable connect status:");
					Console.WriteLine("AC: " + _cableconnected_AC);
					Console.WriteLine("BC: " + _cableconnected_BC);
					Console.WriteLine("AB: " + _cableconnected_AB);
				}
			}

			Is_Lag = LibGBHawkLink.GBLink_frame_advance(GBLink_Pntr, f_cntrls, f_accxs, f_accys, do_Renders, do_Sounds, do_Resets);

			if (Is_Lag) { Lag_Count++; }

			for (uint i = 0; i < Num_ROMS; i++)
			{
				LibGBHawkLink.GBLink_get_video(GBLink_Pntr, video_buffers[i], i);
			}

			get_video_from_Sources();

			_frame++;

			return true;
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GBL;

		public void ResetCounters()
		{
			_frame = 0;
			Lag_Count = 0;
			Is_Lag = false;
		}

		public void Dispose()
		{
			if (GBLink_Pntr != IntPtr.Zero)
			{
				LibGBHawkLink.GBLink_destroy(GBLink_Pntr);
				GBLink_Pntr = IntPtr.Zero;
			}

			DisposeSound();
		}

		public BlipBuffer blip_L_0 = new BlipBuffer(15000);
		public BlipBuffer blip_R_0 = new BlipBuffer(15000);
		public BlipBuffer blip_L_1 = new BlipBuffer(15000);
		public BlipBuffer blip_R_1 = new BlipBuffer(15000);

		public int[] Aud_L_0 = new int[15000];
		public int[] Aud_R_0 = new int[15000];
		public uint num_samp_L_0;
		public uint num_samp_R_0;

		public int[] Aud_L_1 = new int[15000];
		public int[] Aud_R_1 = new int[15000];
		public uint num_samp_L_1;
		public uint num_samp_R_1;

		const int blipbuffsize = 15000;

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
			nsamp = 0;
			samples = Array.Empty<short>();

			get_console_audios[0] = Settings.A_AudioSet;
			get_console_audios[1] = Settings.B_AudioSet;
			get_console_audios[2] = Settings.C_AudioSet;
			get_console_audios[3] = Settings.D_AudioSet;

			uint f_clock = LibGBHawkLink.GBLink_get_audio(GBLink_Pntr, Aud_L_0, ref num_samp_L_0, Aud_R_0, ref num_samp_R_0,
																	   Aud_L_1, ref num_samp_L_1, Aud_R_1, ref num_samp_R_1, get_console_audios);

			int counts = 0;

			for (int i = 0; i < 4; i++)
			{
				if (get_console_audios[i]) { counts++; }
			}

			if (counts == 0)
			{
				nsamp = 0;
				samples = Array.Empty<short>();
			}
			else if (counts == 1)
			{
				// everything stored in '0' entries
				for (int i = 0; i < num_samp_L_0; i++)
				{
					blip_L_0.AddDelta((uint)Aud_L_0[i * 2], Aud_L_0[i * 2 + 1]);
				}

				for (int i = 0; i < num_samp_R_0; i++)
				{
					blip_R_0.AddDelta((uint)Aud_R_0[i * 2], Aud_R_0[i * 2 + 1]);
				}

				//Console.WriteLine(num_samp_L + " " + num_samp_R + " " + f_clock);

				blip_L_0.EndFrame(f_clock);
				blip_R_0.EndFrame(f_clock);

				nsamp = blip_L_0.SamplesAvailable();
				samples = new short[nsamp * 2];

				if (nsamp != 0)
				{
					blip_L_0.ReadSamplesLeft(samples, nsamp);
					blip_R_0.ReadSamplesRight(samples, nsamp);
				}
			}
			else
			{
				// map everything in '0' to left audio and '1' to right audio
				for (int i = 0; i < num_samp_L_0; i++)
				{
					blip_L_0.AddDelta((uint)Aud_L_0[i * 2], Aud_L_0[i * 2 + 1]);
				}

				for (int i = 0; i < num_samp_R_0; i++)
				{
					blip_R_0.AddDelta((uint)Aud_R_0[i * 2], Aud_R_0[i * 2 + 1]);
				}

				for (int i = 0; i < num_samp_L_1; i++)
				{
					blip_L_1.AddDelta((uint)Aud_L_1[i * 2], Aud_L_1[i * 2 + 1]);
				}

				for (int i = 0; i < num_samp_R_1; i++)
				{
					blip_R_1.AddDelta((uint)Aud_R_1[i * 2], Aud_R_1[i * 2 + 1]);
				}

				//Console.WriteLine(num_samp_L + " " + num_samp_R + " " + f_clock);

				blip_L_0.EndFrame(f_clock);
				blip_R_0.EndFrame(f_clock);
				blip_L_1.EndFrame(f_clock);
				blip_R_1.EndFrame(f_clock);

				int nsamp_0 = blip_L_0.SamplesAvailable();
				int nsamp_1 = blip_L_1.SamplesAvailable();

				nsamp = nsamp_0;

				if (nsamp_1 > nsamp_0)
				{
					nsamp = nsamp_1;
				}


				short[] samples_0 = new short[nsamp * 2];
				short[] samples_1 = new short[nsamp * 2];

				samples = new short[nsamp * 2];

				if (nsamp != 0)
				{
					blip_L_0.ReadSamplesLeft(samples_0, nsamp);
					blip_R_0.ReadSamplesRight(samples_0, nsamp);

					blip_L_1.ReadSamplesLeft(samples_1, nsamp);
					blip_R_1.ReadSamplesRight(samples_1, nsamp);

					for (int i = 0; i < nsamp * 2; i++)
					{
						samples[i] = (short)(samples_0[i] + samples_1[i]);
					}
				}
			}
		}

		public void DiscardSamples()
		{
			blip_L_0.Clear();
			blip_R_0.Clear();
			blip_L_1.Clear();
			blip_R_1.Clear();
		}

		public void DisposeSound()
		{
			blip_L_0.Clear();
			blip_R_0.Clear();
			blip_L_0.Dispose();
			blip_R_0.Dispose();
			blip_L_0 = null;
			blip_R_0 = null;

			blip_L_1.Clear();
			blip_R_1.Clear();
			blip_L_1.Dispose();
			blip_R_1.Dispose();
			blip_L_1 = null;
			blip_R_1 = null;
		}

		public int[][] video_buffers = new int[4][];

		public int[] _vidbuffer;

		public int[] GetVideoBuffer()
		{
			return _vidbuffer;
		}

		public void get_video_from_Sources()
		{
			if (Is_OneScreenMode)
			{
				if (Settings.A_VideoSet == GBLinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 160 * 144; i++)
					{
						_vidbuffer[i] = video_buffers[0][i];
					}
				}
				else if (Settings.B_VideoSet == GBLinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 160 * 144; i++)
					{
						_vidbuffer[i] = video_buffers[1][i];
					}
				}
				else if (Settings.C_VideoSet == GBLinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 160 * 144; i++)
					{
						_vidbuffer[i] = video_buffers[2][i];
					}
				}
				else if (Settings.D_VideoSet == GBLinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 160 * 144; i++)
					{
						_vidbuffer[i] = video_buffers[3][i];
					}
				}
				else
				{
					for (int i = 0; i < 160 * 144; i++)
					{
						_vidbuffer[i] = (int)0x000000;
					}
				}
			}
			else if (Num_ROMS == 2)
			{
				for (int i = 0; i < 144; i++)
				{
					for (int j = 0; j < 160; j++)
					{
						if (Settings.A_VideoSet == GBLinkSettings.VideoSrc.Enable)
						{
							_vidbuffer[i * 322 + j] = video_buffers[0][i * 160 + j];
						}
						else
						{
							_vidbuffer[i * 322 + j] = 0;
						}

						if (Settings.B_VideoSet == GBLinkSettings.VideoSrc.Enable)
						{
							_vidbuffer[i * 322 + 162 + j] = video_buffers[1][i * 160 + j];
						}
						else
						{
							_vidbuffer[i * 322 + 162 + j] = 0;
						}
					}

					_vidbuffer[322 * i + 160] = (int)0xFAFAFA;
					_vidbuffer[322 * i + 161] = (int)0xFAFAFA;
				}
			}
			else
			{
				if (Num_ROMS == 3)
				{
					for (int i = 0; i < 144; i++)
					{
						for (int j = 0; j < 160; j++)
						{
							if (Settings.A_VideoSet == GBLinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 322 + j] = video_buffers[0][i * 160 + j];
							}
							else
							{
								_vidbuffer[i * 322 + j] = 0;
							}

							if (Settings.B_VideoSet == GBLinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 322 + 162 + j] = video_buffers[1][i * 160 + j];
							}
							else
							{
								_vidbuffer[i * 322 + 162 + j] = 0;
							}

							if (Settings.C_VideoSet == GBLinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[(i + 146) * 322 + j + 81] = video_buffers[2][i * 160 + j];
							}
							else
							{
								_vidbuffer[(i + 146) * 322 + j + 81] = 0;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < 144; i++)
					{
						for (int j = 0; j < 160; j++)
						{
							if (Settings.A_VideoSet == GBLinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 322 + j] = video_buffers[0][i * 160 + j];
							}
							else
							{
								_vidbuffer[i * 322 + j] = 0;
							}

							if (Settings.B_VideoSet == GBLinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 322 + 162 + j] = video_buffers[1][i * 160 + j];
							}
							else
							{
								_vidbuffer[i * 322 + 162 + j] = 0;
							}

							if (Settings.C_VideoSet == GBLinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[(i + 146) * 322 + j] = video_buffers[2][i * 160 + j];
							}
							else
							{
								_vidbuffer[(i + 146) * 322 + j] = 0;
							}

							if (Settings.D_VideoSet == GBLinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[(i + 146) * 322 + j + 162] = video_buffers[3][i * 160 + j];
							}
							else
							{
								_vidbuffer[(i + 146) * 322 + j + 162] = 0;
							}
						}
					}
				}

				for (int i = 0; i < 144; i++)
				{
					_vidbuffer[322 * i + 160] = (int)0xFAFAFA;
					_vidbuffer[322 * i + 161] = (int)0xFAFAFA;
				}

				for (int i = 0; i < 322; i++)
				{
					_vidbuffer[322 * 144 + i] = (int)0xFAFAFA;
					_vidbuffer[322 * 145 + i] = (int)0xFAFAFA;
				}

				if (Num_ROMS == 4)
				{
					for (int i = 0; i < 144; i++)
					{
						_vidbuffer[322 * i + 322 * 146 + 160] = (int)0xFAFAFA;
						_vidbuffer[322 * i + 322 * 146 + 161] = (int)0xFAFAFA;
					}
				}
			}
		}

		public int VirtualWidth => Actual_Width;
		public int VirtualHeight => Actual_Height;
		public int BufferWidth => Actual_Width;
		public int BufferHeight => Actual_Height;
		public int BackgroundColor => unchecked((int)0xFF000000);
		public int VsyncNumerator => 262144;
		public int VsyncDenominator => 4389;
	}
}
