﻿using System;
using System.Windows.Forms;

namespace BizHawk.Client.GBAHawk
{
	public partial class HexColorsForm : Form
	{
		private readonly HexEditor _hexEditor;

		public HexColorsForm(HexEditor hexEditor)
		{
			_hexEditor = hexEditor;
			InitializeComponent();
		}

		private void HexColors_Form_Load(object sender, EventArgs e)
		{
			HexBackgrnd.BackColor = _hexEditor.Colors.Background;
			HexForegrnd.BackColor = _hexEditor.Colors.Foreground;
			HexMenubar.BackColor = _hexEditor.Colors.MenuBar;
			HexHighlight.BackColor = _hexEditor.Colors.Highlight;
		}

		private void HexBackground_Click(object sender, MouseEventArgs e)
		{
			if (colorDialog1.ShowDialog().IsOk())
			{
				_hexEditor.Colors.Background = colorDialog1.Color;
				_hexEditor.Header.BackColor = colorDialog1.Color;
				_hexEditor.MemoryViewerBox.BackColor = _hexEditor.Colors.Background;
				HexBackgrnd.BackColor = colorDialog1.Color;
			}
		}

		private void HexForeground_Click(object sender, MouseEventArgs e)
		{
			if (colorDialog1.ShowDialog().IsOk())
			{
				_hexEditor.Colors.Foreground = colorDialog1.Color;
				_hexEditor.Header.ForeColor = colorDialog1.Color;
				_hexEditor.MemoryViewerBox.ForeColor = _hexEditor.Colors.Foreground;
				HexForegrnd.BackColor = colorDialog1.Color;
			}
		}

		private void HexMenuBar_Click(object sender, MouseEventArgs e)
		{
			if (colorDialog1.ShowDialog().IsOk())
			{
				_hexEditor.Colors.MenuBar = colorDialog1.Color;
				_hexEditor.HexMenuStrip.BackColor = _hexEditor.Colors.MenuBar;
				HexMenubar.BackColor = colorDialog1.Color;
			}
		}

		private void HexHighlight_Click(object sender, MouseEventArgs e)
		{
			if (colorDialog1.ShowDialog().IsOk())
			{
				_hexEditor.Colors.Highlight = colorDialog1.Color;
				HexHighlight.BackColor = colorDialog1.Color;
			}
		}
	}
}
