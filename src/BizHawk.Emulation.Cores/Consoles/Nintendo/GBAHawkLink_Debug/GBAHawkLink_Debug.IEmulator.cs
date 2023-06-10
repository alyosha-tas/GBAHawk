using System;

using BizHawk.Emulation.Common;
using static BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug.GBAHawkLink_Debug;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug
{
	public partial class GBAHawkLink_Debug : IEmulator, IVideoProvider, ISoundProvider
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _controllerDeck.Definition;

		public bool FrameAdvance(IController controller, bool render, bool renderSound)
		{
			//Console.WriteLine("-----------------------FRAME-----------------------");

			if (_tracer.IsEnabled())
			{
				L.TraceCallback = s => _tracer.Put(s);
			}
			else
			{
				L.TraceCallback = null;
			}

			if (controller.IsPressed("P1 Power"))
			{
				L.HardReset();
			}
			if (controller.IsPressed("P2 Power"))
			{
				R.HardReset();
			}

			bool cablediscosignalNew = controller.IsPressed("Toggle Cable");
			if (cablediscosignalNew && !_cablediscosignal)
			{
				_cableconnected ^= true;
				Console.WriteLine("Cable connect status to {0}", _cableconnected);
			}

			_cablediscosignal = cablediscosignalNew;

			_islag = true;

			GetControllerState(controller);

			do_frame_fill = false;
			do_frame();

			FillVideoBuffer();

			_islag = L.Is_Lag & R.Is_Lag;

			if (_islag)
			{
				_lagcount++;
			}

			_frame++;

			return true;
		}

		public void do_frame()
		{			
			L.do_controller_check();
			R.do_controller_check();

			L.VBlank_Rise = false;

			// advance one full frame
			while (!L.VBlank_Rise)
			{
				L.do_single_step();
				R.do_single_step();

				if (_cableconnected)
				{

				}
			}
		}

		public void GetControllerState(IController controller)
		{
			InputCallbacks.Call();
			L.controller_state = _controllerDeck.ReadPort1(controller);
			R.controller_state = _controllerDeck.ReadPort2(controller);
		}

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GBAL;

		public bool DeterministicEmulation { get; set; }

		public void ResetCounters()
		{
			_frame = 0;
			_lagcount = 0;
			_islag = false;
		}

		public void Dispose()
		{
			L.Dispose();
			R.Dispose();
		}

		public int[] _vidbuffer = new int[240 * 2 * 160];

		public int[] GetVideoBuffer() => _vidbuffer;

		public void FillVideoBuffer()
		{
			// combine the 2 video buffers from the instances
			for (int i = 0; i < 160; i++)
			{
				for (int j = 0; j < 240; j++)
				{
					_vidbuffer[i * 480 + j] = L.vid_buffer[i * 240 + j];
					_vidbuffer[i * 480 + j + 240] = R.vid_buffer[i * 240 + j];
				}
			}
		}

		public int VirtualWidth => 240 * 2;
		public int VirtualHeight => 160;
		public int BufferWidth => 240 * 2;
		public int BufferHeight => 160;

		public int BackgroundColor => unchecked((int)0xFF000000);
		public int VsyncNumerator => 262144;
		public int VsyncDenominator => 4389;

		public static readonly uint[] color_palette_BW = { 0xFFFFFFFF , 0xFFAAAAAA, 0xFF555555, 0xFF000000 };
		public static readonly uint[] color_palette_Gr = { 0xFFA4C505, 0xFF88A905, 0xFF1D551D, 0xFF052505 };

		public uint[] color_palette = new uint[4];

		public bool CanProvideAsync => false;

		public void SetSyncMode(SyncSoundMode mode)
		{
			if (mode != SyncSoundMode.Sync)
			{
				throw new InvalidOperationException("Only Sync mode is supported_");
			}
		}

		public SyncSoundMode SyncMode => SyncSoundMode.Sync;

		public void GetSamplesSync(out short[] samples, out int nsamp)
		{
			L.GetSamplesSync(out var tempSampL, out var nsampL);
			R.GetSamplesSync(out var tempSampR, out var nsampR);

			if (linkSettings.AudioSet == GBAHawkLink_Debug_Settings.AudioSrc.Left)
			{
				samples = tempSampL;
				nsamp = nsampL;
			}
			else if (linkSettings.AudioSet == GBAHawkLink_Debug_Settings.AudioSrc.Right)
			{
				samples = tempSampR;
				nsamp = nsampR;
			}
			else
			{
				samples = new short[0];
				nsamp = 0;
			}
		}

		public void GetSamplesAsync(short[] samples)
		{
			throw new NotSupportedException("Async is not available");
		}

		public void DiscardSamples()
		{
			L.DiscardSamples();
			R.DiscardSamples();
		}

		public void DisposeSound()
		{
			L.DisposeSound();
			R.DisposeSound();
		}
	}
}
