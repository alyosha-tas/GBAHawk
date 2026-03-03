using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.NES.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.NESHawk
{
	public partial class NESHawk : IEmulator, ISoundProvider, IVideoProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public IController Controller;

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
			
			LibNESHawk.NES_settracecallback(NES_Pntr, tracecb);

			if (controller.IsPressed("Power"))
			{
				HardReset();
			}

			if (controller.IsPressed("Reset"))
			{
				SoftReset();
			}

			Is_Lag = LibNESHawk.NES_frame_advance(NES_Pntr, true, true);

			LibNESHawk.NES_get_video(NES_Pntr, _vidbuffer);

			if (Is_Lag) { Lag_Count++; }

			_frame++;

			return true;
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.NES;

		public void ResetCounters()
		{
			_frame = 0;
			Lag_Count = 0;
			Is_Lag = false;
		}

		public void Dispose()
		{
			if (NES_Pntr != IntPtr.Zero)
			{
				LibNESHawk.NES_destroy(NES_Pntr);
				NES_Pntr = IntPtr.Zero;
			}

			DisposeSound();
		}

		public BlipBuffer blip_buff = new BlipBuffer(25000);

		public int[] Aud = new int[25000];
		public uint num_samp;

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
			uint f_clock = LibNESHawk.NES_get_audio(NES_Pntr, Aud, ref num_samp);

			for (int i = 0; i < num_samp; i++)
			{
				blip_buff.AddDelta((uint)Aud[i * 2], 2*Aud[i * 2 + 1]);
			}

			//Console.WriteLine(num_samp_L + " " + num_samp_R + " " + f_clock);

			blip_buff.EndFrame(f_clock);

			nsamp = blip_buff.SamplesAvailable();
			samples = new short[nsamp * 2];

			if (nsamp != 0)
			{
				blip_buff.ReadSamplesLeft(samples, nsamp);
				blip_buff.ReadSamplesRight(samples, nsamp);
			}
		}

		public void DiscardSamples()
		{
			blip_buff.Clear();
		}

		public void DisposeSound()
		{
			blip_buff.Clear();
			blip_buff.Dispose();
			blip_buff = null;
		}

		public int[] _vidbuffer = new int[256 * 240];

		public int[] GetVideoBuffer()
		{
			return _vidbuffer;
		}

		public int BackgroundColor => unchecked((int)0xFF000000);

		public int VirtualWidth => (int)(BufferWidth * 1.146);
		public int VirtualHeight => BufferHeight;
		public int BufferWidth => 256;
		public int BufferHeight => 240;
		public int VsyncNumerator => 1789773 * 2;
		public int VsyncDenominator => 59561;
	}
}
