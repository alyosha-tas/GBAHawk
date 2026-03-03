using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.NES.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.SubNESHawk
{
	public partial class SubNESHawk : IEmulator
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _nesCore.ControllerDefinition;

		public bool FrameAdvance(IController controller, bool render, bool renderSound)
		{
			_nesCore.Controller = controller;

			//Console.WriteLine("-----------------------FRAME-----------------------");
			if (_nesCore.Tracer.IsEnabled())
			{
				_nesCore.tracecb = _nesCore.MakeTrace;
			}
			else
			{
				_nesCore.tracecb = null;
			}

			LibNESHawk.NES_settracecallback(_nesCore.NES_Pntr, _nesCore.tracecb);

			reset_frame = false;

			if (controller.IsPressed("Power"))
			{
				reset_frame = true;
			}

			reset_cycle = controller.AxisValue("Reset Cycle");
			reset_cycle_int = (uint)Math.Floor(reset_cycle);

			pass_a_frame = LibNESHawk.NES_subframe_advance(_nesCore.NES_Pntr, true, true, reset_frame, reset_cycle_int);

			if (pass_a_frame)
			{
				LibNESHawk.NES_get_video(_nesCore.NES_Pntr, _nesCore._vidbuffer);
				current_cycle = 0;
			}

			_isLag = pass_a_frame;

			if (_isLag)
			{
				_lagCount++;
			}

			reset_frame = false;

			_frame++;

			return pass_a_frame;
		}

		private bool pass_new_input;
		private bool pass_a_frame;
		private bool reset_frame;
		private int current_cycle;
		private float reset_cycle;
		private uint reset_cycle_int;

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.NES;

		public void ResetCounters()
		{
			_frame = 0;
			_lagCount = 0;
			_isLag = false;
		}

		public void Dispose() => _nesCore.Dispose();
	}
}
