using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Cores.Nintendo.GBCHawk;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBCHawk
{
	public partial class SubGBCHawk : IEmulator
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _gbcCore.ControllerDefinition;

		public bool FrameAdvance(IController controller, bool render, bool renderSound)
		{
			_gbcCore.Controller = controller;

			// update the controller state on VBlank
			_gbcCore.GetControllerState(controller);

			//Console.WriteLine("-----------------------FRAME-----------------------");
			if (_gbcCore.Tracer.IsEnabled())
			{
				_gbcCore.tracecb = _gbcCore.MakeTrace;
			}
			else
			{
				_gbcCore.tracecb = null;
			}

			LibGBCHawk.GBC_settracecallback(_gbcCore.GBC_Pntr, _gbcCore.tracecb);

			reset_frame = false;

			if (controller.IsPressed("P1 Power"))
			{
				reset_frame = true;
			}

			input_cycle = controller.AxisValue("Input Cycle");
			input_cycle_int = (uint)Math.Floor(input_cycle);

			pass_a_frame = false;

			pass_a_frame = LibGBCHawk.GBC_subframe_advance(_gbcCore.GBC_Pntr, _gbcCore.controller_state, _gbcCore.Acc_X_state, _gbcCore.Acc_Y_state, true, true, reset_frame, input_cycle_int);

			if (pass_a_frame)
			{
				LibGBCHawk.GBC_get_video(_gbcCore.GBC_Pntr, _gbcCore._vidbuffer);
			}

			_isLag = pass_a_frame;

			if (_isLag)
			{
				_lagCount++;
			}

			_frame++;

			return pass_a_frame;
		}

		private bool pass_new_input;
		private bool pass_a_frame;
		private bool reset_frame;
		private float input_cycle;
		private uint input_cycle_int;

		public int Frame => _frame;

		public string SystemId => VSystemID.Raw.GB;

		public void ResetCounters()
		{
			_frame = 0;
			_lagCount = 0;
			_isLag = false;
		}

		public void Dispose() => _gbcCore.Dispose();
	}
}
