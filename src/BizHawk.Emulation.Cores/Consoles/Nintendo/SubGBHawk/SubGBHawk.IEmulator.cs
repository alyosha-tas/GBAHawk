using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBHawk
{
	public partial class SubGBHawk : IEmulator
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _gbCore.ControllerDefinition;

		public bool FrameAdvance(IController controller, bool render, bool renderSound)
		{
			_gbCore.Controller = controller;

			//Console.WriteLine("-----------------------FRAME-----------------------");
			if (_gbCore.Tracer.IsEnabled())
			{
				_gbCore.tracecb = _gbCore.MakeTrace;
			}
			else
			{
				_gbCore.tracecb = null;
			}

			LibGBHawk.GB_settracecallback(_gbCore.GB_Pntr, _gbCore.tracecb);

			reset_frame = false;

			if (controller.IsPressed("Power"))
			{
				reset_frame = true;
			}

			reset_cycle = controller.AxisValue("Reset Cycle");
			reset_cycle_int = (uint)Math.Floor(reset_cycle);

			pass_a_frame = LibGBHawk.GB_subframe_advance(_gbCore.GB_Pntr, true, true, reset_frame, reset_cycle_int);

			if (pass_a_frame)
			{
				LibGBHawk.GB_get_video(_gbCore.GB_Pntr, _gbCore._vidbuffer);
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

		public string SystemId => VSystemID.Raw.GB;

		public void ResetCounters()
		{
			_frame = 0;
			_lagCount = 0;
			_isLag = false;
		}

		public void Dispose() => _gbCore.Dispose();
	}
}
