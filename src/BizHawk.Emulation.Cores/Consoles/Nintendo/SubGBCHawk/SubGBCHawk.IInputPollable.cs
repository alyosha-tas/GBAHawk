using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBCHawk
{
	public partial class SubGBCHawk : IInputPollable
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

		public IInputCallbackSystem InputCallbacks => _gbcCore.InputCallbacks;

		private bool _isLag = true;
		private int _lagCount;
	}
}
