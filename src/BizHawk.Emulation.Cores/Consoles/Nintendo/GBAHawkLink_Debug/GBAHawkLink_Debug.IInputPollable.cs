using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug
{
	public partial class GBAHawkLink_Debug : IInputPollable
	{
		public int LagCount
		{
			get => Lag_Count;
			set => Lag_Count = value;
		}

		public bool IsLagFrame
		{
			get => Is_Lag;
			set => Is_Lag = value;
		}

		public IInputCallbackSystem InputCallbacks { get; } = new InputCallbackSystem();

		public bool Is_Lag;
		private int Lag_Count;
	}
}
