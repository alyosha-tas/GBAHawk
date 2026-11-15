using BizHawk.Common.NumberExtensions;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using System;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink : IEmulator, ISoundProvider, IVideoProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		public IController Controller;

		bool[] do_Resets = new bool[4];
		bool[] do_Renders = new bool[4];
		bool[] do_Sounds = new bool[4];
		byte[] f_solars = new byte[4];
		ushort[] f_cntrls = new ushort[4];
		ushort[] f_accxs = new ushort[4];
		ushort[] f_accys = new ushort[4];

		bool[] get_console_audios = new bool[4];

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			Controller = controller;
			
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

			// update the controller state on VBlank
			InputCallbacks.Call();

			for (int i = 0; i < Num_ROMS; i++)
			{
				f_cntrls[i] = _controllerDeck.ReadPort(controller, i);
				(f_accxs[i], f_accys[i]) = _controllerDeck.ReadAcc(controller, i);

				do_Renders[i] = true;
				do_Sounds[i] = true;
			}

			GetController_GBP_Solar(controller);

			f_solars[0] = Solar_state_A;
			f_solars[1] = Solar_state_B;
			f_solars[2] = Solar_state_C;
			f_solars[3] = Solar_state_D;

			do_Resets[0] = controller.IsPressed("P1 Power");
			do_Resets[1] = controller.IsPressed("P2 Power");
			do_Resets[2] = controller.IsPressed("P3 Power");
			do_Resets[3] = controller.IsPressed("P4 Power");

			Is_Lag = LibGBAHawkLink.GBALink_frame_advance(GBA_Pntr, f_cntrls, f_accxs, f_accys, f_solars, do_Renders, do_Sounds, do_Resets);

			if (Is_Lag) { Lag_Count++; }

			for (uint i = 0; i < Num_ROMS; i++)
			{
				LibGBAHawkLink.GBALink_get_video(GBA_Pntr, video_buffers[i], i);
			}

			_frame++;

			// Detect GBP via image
			if (SyncSettings.Use_GBP_A) { GBP_Screen_Detection_A = GBACommonFunctions.Check_Video_GBP(video_buffers[0]); }
			if (SyncSettings.Use_GBP_B) { GBP_Screen_Detection_B = GBACommonFunctions.Check_Video_GBP(video_buffers[1]); }
			if (Num_ROMS > 2)
			{
				if (SyncSettings.Use_GBP_C) { GBP_Screen_Detection_C = GBACommonFunctions.Check_Video_GBP(video_buffers[2]); }

				if (Num_ROMS > 3)
				{
					if (SyncSettings.Use_GBP_D) { GBP_Screen_Detection_D = GBACommonFunctions.Check_Video_GBP(video_buffers[3]); }
				}
			}

			return true;
		}

		public void GetController_GBP_Solar(IController controller)
		{
			if (SyncSettings.Use_Discrete_Solar_A)
			{
				// only change state if a key is being pressed, lowest key has highest priority
				if (controller.IsPressed("P1 S1"))
				{
					Solar_state_A = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel1_A / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P1 S2"))
				{
					Solar_state_A = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel2_A / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P1 S3"))
				{
					Solar_state_A = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel3_A / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P1 S4"))
				{
					Solar_state_A = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel4_A / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P1 S5"))
				{
					Solar_state_A = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel5_A / 100 * 0xA0) + 0x50));
				}
			}
			else
			{
				Solar_state_A = _controllerDeck.ReadSolar(controller, 0);
			}

			if (SyncSettings.Use_Discrete_Solar_B)
			{
				// only change state if a key is being pressed, lowest key has highest priority
				if (controller.IsPressed("P2 S1"))
				{
					Solar_state_B = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel1_B / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P2 S2"))
				{
					Solar_state_B = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel2_B / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P2 S3"))
				{
					Solar_state_B = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel3_B / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P2 S4"))
				{
					Solar_state_B = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel4_B / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P2 S5"))
				{
					Solar_state_B = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel5_B / 100 * 0xA0) + 0x50));
				}
			}
			else
			{
				Solar_state_B = _controllerDeck.ReadSolar(controller, 1);
			}

			if (SyncSettings.Use_Discrete_Solar_C)
			{
				// only change state if a key is being pressed, lowest key has highest priority
				if (controller.IsPressed("P3 S1"))
				{
					Solar_state_C = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel1_C / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P3 S2"))
				{
					Solar_state_C = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel2_C / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P3 S3"))
				{
					Solar_state_C = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel3_C / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P3 S4"))
				{
					Solar_state_C = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel4_C / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P3 S5"))
				{
					Solar_state_C = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel5_C / 100 * 0xA0) + 0x50));
				}
			}
			else if (Num_ROMS > 2)
			{
				
				Solar_state_C = _controllerDeck.ReadSolar(controller, 2);
			}

			if (SyncSettings.Use_Discrete_Solar_D)
			{
				// only change state if a key is being pressed, lowest key has highest priority
				if (controller.IsPressed("P4 S1"))
				{
					Solar_state_D = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel1_D / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P4 S2"))
				{
					Solar_state_D = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel2_D / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P4 S3"))
				{
					Solar_state_D = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel3_D / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P4 S4"))
				{
					Solar_state_D = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel4_D / 100 * 0xA0) + 0x50));
				}
				else if (controller.IsPressed("P4 S5"))
				{
					Solar_state_D = (byte)(0x140 - ((int)Math.Floor((float)SyncSettings.LightLevel5_D / 100 * 0xA0) + 0x50));
				}
			}
			else if (Num_ROMS > 3)
			{
				Solar_state_D = _controllerDeck.ReadSolar(controller, 3);
			}

			// override the input state from the GBP directly
			if (GBP_Screen_Detection_A)
			{
				if (GBP_Screen_Count_A < 2)
				{
					f_cntrls[0] = 0x3FF;

					GBP_Screen_Count_A += 1;
				}
				else
				{
					f_cntrls[0] = 0x30F;

					GBP_Mode_Enabled_A = true;

					//Console.WriteLine("GBP Rumble mode enabled");
					LibGBAHawkLink.GBALink_Set_GBP_Enable(GBA_Pntr, 0);

					GBP_Screen_Count_A = 0;
				}
			}

			if (GBP_Screen_Detection_B)
			{
				if (GBP_Screen_Count_B < 2)
				{
					f_cntrls[1] = 0x3FF;

					GBP_Screen_Count_B += 1;
				}
				else
				{
					f_cntrls[1] = 0x30F;

					GBP_Mode_Enabled_B = true;

					//Console.WriteLine("GBP Rumble mode enabled");
					LibGBAHawkLink.GBALink_Set_GBP_Enable(GBA_Pntr, 1);

					GBP_Screen_Count_B = 0;
				}
			}

			if (GBP_Screen_Detection_C)
			{
				if (GBP_Screen_Count_C < 2)
				{
					f_cntrls[2] = 0x3FF;

					GBP_Screen_Count_C += 1;
				}
				else
				{
					f_cntrls[2] = 0x30F;

					GBP_Mode_Enabled_C = true;

					//Console.WriteLine("GBP Rumble mode enabled");
					LibGBAHawkLink.GBALink_Set_GBP_Enable(GBA_Pntr, 2);

					GBP_Screen_Count_C = 0;
				}
			}

			if (GBP_Screen_Detection_D)
			{
				if (GBP_Screen_Count_D < 2)
				{
					f_cntrls[3] = 0x3FF;

					GBP_Screen_Count_D += 1;
				}
				else
				{
					f_cntrls[3] = 0x30F;

					GBP_Mode_Enabled_D = true;

					//Console.WriteLine("GBP Rumble mode enabled");
					LibGBAHawkLink.GBALink_Set_GBP_Enable(GBA_Pntr, 3);

					GBP_Screen_Count_D = 0;
				}
			}
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GBAL;

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

		public void GetSamplesSync(out short[] samples, out int nsamp)
		{
			nsamp = 0;
			samples = Array.Empty<short>();

			get_console_audios[0] = Settings.A_AudioSet;
			get_console_audios[1] = Settings.B_AudioSet;
			get_console_audios[2] = Settings.C_AudioSet;
			get_console_audios[3] = Settings.D_AudioSet;

			uint f_clock = LibGBAHawkLink.GBALink_get_audio(GBA_Pntr, Aud_L_0, ref num_samp_L_0, Aud_R_0, ref num_samp_R_0,
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

				//Console.WriteLine(num_samp_L_0 + " " + num_samp_R_0 + " " + f_clock);

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
			if (Is_OneScreenMode)
			{
				if (Settings.A_VideoSet == GBALinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 240*160; i++)
					{
						_vidbuffer[i] = video_buffers[0][i];
					}
				}
				else if (Settings.B_VideoSet == GBALinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 240 * 160; i++)
					{
						_vidbuffer[i] = video_buffers[1][i];
					}
				}
				else if (Settings.C_VideoSet == GBALinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 240 * 160; i++)
					{
						_vidbuffer[i] = video_buffers[2][i];
					}
				}
				else if (Settings.D_VideoSet == GBALinkSettings.VideoSrc.Enable)
				{
					for (int i = 0; i < 240 * 160; i++)
					{
						_vidbuffer[i] = video_buffers[3][i];
					}
				}
				else
				{
					for (int i = 0; i < 240 * 160; i++)
					{
						_vidbuffer[i] = (int)0x000000;
					}
				}
			}
			else if (Num_ROMS == 2)
			{
				for (int i = 0; i < 160; i++)
				{
					for (int j = 0; j < 240; j++)
					{
						if (Settings.A_VideoSet == GBALinkSettings.VideoSrc.Enable)
						{
							_vidbuffer[i * 482 + j] = video_buffers[0][i * 240 + j];
						}
						else
						{
							_vidbuffer[i * 482 + j] = 0;
						}

						if (Settings.B_VideoSet == GBALinkSettings.VideoSrc.Enable)
						{
							_vidbuffer[i * 482 + 242 + j] = video_buffers[1][i * 240 + j];
						}
						else
						{
							_vidbuffer[i * 482 + 242 + j] = 0;
						}
					}

					_vidbuffer[482 * i + 240] = (int)0xFAFAFA;
					_vidbuffer[482 * i + 241] = (int)0xFAFAFA;
				}
			}
			else
			{
				if (Num_ROMS == 3)
				{
					for (int i = 0; i < 160; i++)
					{
						for (int j = 0; j < 240; j++)
						{
							if (Settings.A_VideoSet == GBALinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 482 + j] = video_buffers[0][i * 240 + j];
							}
							else
							{
								_vidbuffer[i * 482 + j] = 0;
							}

							if (Settings.B_VideoSet == GBALinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 482 + 242 + j] = video_buffers[1][i * 240 + j];
							}
							else
							{
								_vidbuffer[i * 482 + 242 + j] = 0;
							}

							if (Settings.C_VideoSet == GBALinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[(i + 162) * 482 + j + 121] = video_buffers[2][i * 240 + j];
							}
							else
							{
								_vidbuffer[(i + 162) * 482 + j + 121] = 0;
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < 160; i++)
					{
						for (int j = 0; j < 240; j++)
						{
							if (Settings.A_VideoSet == GBALinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 482 + j] = video_buffers[0][i * 240 + j];
							}
							else
							{
								_vidbuffer[i * 482 + j] = 0;
							}

							if (Settings.B_VideoSet == GBALinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[i * 482 + 242 + j] = video_buffers[1][i * 240 + j];
							}
							else
							{
								_vidbuffer[i * 482 + 242 + j] = 0;
							}

							if (Settings.C_VideoSet == GBALinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[(i + 162) * 482 + j] = video_buffers[2][i * 240 + j];
							}
							else
							{
								_vidbuffer[(i + 162) * 482 + j] = 0;
							}

							if (Settings.D_VideoSet == GBALinkSettings.VideoSrc.Enable)
							{
								_vidbuffer[(i + 162) * 482 + j + 242] = video_buffers[3][i * 240 + j];
							}
							else
							{
								_vidbuffer[(i + 162) * 482 + j + 242] = 0;
							}
						}
					}
				}

				for (int i = 0; i < 160; i++)
				{
					_vidbuffer[482 * i + 240] = (int)0xFAFAFA;
					_vidbuffer[482 * i + 241] = (int)0xFAFAFA;
				}

				for (int i = 0; i < 482; i++)
				{
					_vidbuffer[482 * 160 + i] = (int)0xFAFAFA;
					_vidbuffer[482 * 161 + i] = (int)0xFAFAFA;
				}

				if (Num_ROMS == 4)
				{
					for (int i = 0; i < 160; i++)
					{
						_vidbuffer[482 * i + 482 * 162 + 240] = (int)0xFAFAFA;
						_vidbuffer[482 * i + 482 * 162 + 241] = (int)0xFAFAFA;
					}
				}
			}

			return _vidbuffer;
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
