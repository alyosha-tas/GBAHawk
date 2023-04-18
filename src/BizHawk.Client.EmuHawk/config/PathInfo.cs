using System;
using System.Windows.Forms;

namespace BizHawk.Client.GBAHawk
{
	public partial class PathInfo : Form
	{
		public PathInfo()
		{
			InitializeComponent();
		}

		private void Ok_Click(object sender, EventArgs e)
		{
			Close();
		}
	}
}
