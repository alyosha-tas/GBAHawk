using BizHawk.Emulation.Common;
using System;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public partial class GBAHawk : IEmulator, ISoundProvider, IVideoProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => current_controller;

		public bool FrameAdvance(IController controller, bool render, bool rendersound)
		{
			ushort cont_result = 0x3FF;

			if (controller.IsPressed("Up"))
			{
				cont_result &= 0xFFBF;
			}
			if (controller.IsPressed("Down"))
			{
				cont_result &= 0xFF7F;
			}
			if (controller.IsPressed("Left"))
			{
				cont_result &= 0xFFDF;
			}
			if (controller.IsPressed("Right"))
			{
				cont_result &= 0xFFEF;
			}
			if (controller.IsPressed("Start"))
			{
				cont_result &= 0xFFF7;
			}
			if (controller.IsPressed("Select"))
			{
				cont_result &= 0xFFFB;
			}
			if (controller.IsPressed("B"))
			{
				cont_result &= 0xFFFD;
			}
			if (controller.IsPressed("A"))
			{
				cont_result &= 0xFFFE;
			}
			if (controller.IsPressed("L"))
			{
				cont_result &= 0xFDFF;
			}
			if (controller.IsPressed("R"))
			{
				cont_result &= 0xFEFF;
			}

			if (Tracer.IsEnabled())
			{
				tracecb = MakeTrace;
			}
			else
			{
				tracecb = null;
			}
			
			LibGBAHawk.GBA_settracecallback(GBA_Pntr, tracecb);

			if (controller.IsPressed("Power"))
			{
				HardReset();
			}

			LibGBAHawk.GBA_frame_advance(GBA_Pntr, cont_result, true, true);

			LibGBAHawk.GBA_get_video(GBA_Pntr, _vidbuffer);

			_frame++;

			return true;
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GBA;

		public bool DeterministicEmulation => true;

		public void ResetCounters()
		{
			_frame = 0;
			_lagCount = 0;
			_isLag = false;
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
