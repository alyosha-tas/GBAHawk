using System;
using System.Collections.Generic;
using System.Windows.Forms;
using BizHawk.Client.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Client.EmuHawk
{
	public partial class VirtualPadDiscManager : UserControl, IVirtualPadControl
	{
		private readonly InputManager _inputManager;

		public VirtualPadDiscManager(
			InputManager inputManager,
			IEmulator ownerEmulator,
			string name,
			IReadOnlyList<string> buttonNames)
		{
			_inputManager = inputManager;
			_ownerEmulator = ownerEmulator;
			Name = name;
			InitializeComponent();
			btnOpen.InputManager = _inputManager;
			btnClose.InputManager = _inputManager;
			btnOpen.Name = buttonNames[0];
			btnClose.Name = buttonNames[1];
			_discSelectName = buttonNames[2];

			// these need to follow InitializeComponent call
			UpdateCoreAssociation();
			UpdateValues();
		}

		private readonly string _discSelectName;
		private readonly object _ownerEmulator;

		private void UpdateCoreAssociation()
		{
			return;
		}


		public void Clear()
		{
		}

		public void UpdateValues()
		{
			return;
		}

		public void Set(IController controller)
		{
			//controller.AxisValue("Disc Select")
		}

		public bool ReadOnly { get; set; }

		private void lvDiscs_SelectedIndexChanged(object sender, EventArgs e)
		{
			// emergency measure: if no selection, set no disc
			_inputManager.StickyXorAdapter.SetAxis(_discSelectName, lvDiscs.SelectedIndices.Count == 0 ? 0 : lvDiscs.SelectedIndices[0]);
		}

		private void btnClose_Click(object sender, EventArgs e)
		{
			if (lblTimeZero.Visible)
			{
				btnOpen.Checked = !btnClose.Checked;
				UpdateValues();
			}
		}

		private void btnOpen_Click(object sender, EventArgs e)
		{
			if (lblTimeZero.Visible)
			{
				btnClose.Checked = !btnOpen.Checked;
				UpdateValues();
			}
		}
	}
}
