using System;
using System.Drawing;
using System.Windows.Forms;

using BizHawk.Client.Common;
using BizHawk.Bizware.BizwareGL;

namespace BizHawk.Client.EmuHawk
{
	/// <summary>
	/// Thinly wraps a BizwareGL.GraphicsControl for EmuHawk's needs
	/// </summary>
	public class PresentationPanel
	{
		private readonly Config _config;

		public PresentationPanel(
			Config config,
			IGL gl,
			MouseEventHandler onClick,
			MouseEventHandler onMove,
			MouseEventHandler onWheel)
		{
			_config = config;

			GraphicsControl = new GraphicsControl(gl)
			{
				Dock = DockStyle.Fill,
				BackColor = Color.Black
			};

			// pass through these events to the form. we might need a more scalable solution for mousedown etc. for zapper and whatnot.
			// http://stackoverflow.com/questions/547172/pass-through-mouse-events-to-parent-control (HTTRANSPARENT)
			GraphicsControl.MouseClick += onClick;
			GraphicsControl.MouseMove += onMove;
			GraphicsControl.MouseWheel += onWheel;
		}

		private bool _isDisposed;
		public void Dispose()
		{
			if (_isDisposed) return;
			_isDisposed = true;
			GraphicsControl.Dispose();
		}

		//graphics resources
		public GraphicsControl GraphicsControl;

		public Control Control => GraphicsControl;
		public static implicit operator Control(PresentationPanel self) { return self.GraphicsControl; }

		public bool Resized { get; set; }

		public Size NativeSize => GraphicsControl.ClientSize;
	}
}
