using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Cores.Nintendo.GBHawk;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBHawk
{
	public partial class SubGBHawk : IEmulator
	{
		public IEmulatorServiceProvider ServiceProvider { get; }

		public ControllerDefinition ControllerDefinition => _gbCore.ControllerDefinition;

		public bool FrameAdvance(IController controller, bool render, bool renderSound)
		{
			//Update the color palette if a setting changed
			if (_gbCore.Settings.Palette == GBHawk.GBHawk.GBHawkSettings.PaletteType.BW)
			{
				LibGBHawk.GB_load_Palette(_gbCore.GB_Pntr, true);
			}
			else
			{
				LibGBHawk.GB_load_Palette(_gbCore.GB_Pntr, false);
			}

			_gbCore.Controller = controller;

			// update the controller state on VBlank
			_gbCore.GetControllerState(controller);

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

			if (controller.IsPressed("P1 Power"))
			{
				reset_frame = true;
			}

			input_cycle = controller.AxisValue("Input Cycle");
			input_cycle_int = (uint)Math.Floor(input_cycle);

			pass_a_frame = false;

			pass_a_frame = LibGBHawk.GB_subframe_advance(_gbCore.GB_Pntr, _gbCore.controller_state, _gbCore.Acc_X_state, _gbCore.Acc_Y_state, true, true, reset_frame, input_cycle_int);

			if (pass_a_frame)
			{
				LibGBHawk.GB_get_video(_gbCore.GB_Pntr, _gbCore._vidbuffer);
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

		public void Dispose() => _gbCore.Dispose();
	}
}
