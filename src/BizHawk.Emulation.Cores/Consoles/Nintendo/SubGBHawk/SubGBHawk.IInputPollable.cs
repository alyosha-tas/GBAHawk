using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBHawk
{
	public partial class SubGBHawk : IInputPollable
	{
		public int LagCount
		{
			get => _lagCount;
			set => _lagCount = value;
		}

		public bool IsLagFrame
		{
			get => _isLag;
			set => _isLag = value;
		}

		public IInputCallbackSystem InputCallbacks => _gbCore.InputCallbacks;

		private bool _isLag = true;
		private int _lagCount;
	}
}
