using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class GBHawkOld : IInputPollable
	{
		public int LagCount
		{
			get => _lagcount;
			set => _lagcount = value;
		}

		public bool IsLagFrame
		{
			get => _islag;
			set => _islag = value;
		}

		public IInputCallbackSystem InputCallbacks { get; } = new InputCallbackSystem();

		internal bool _islag = true;
		private int _lagcount;
	}
}
