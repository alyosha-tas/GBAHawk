using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public partial class GBAHawk : IInputPollable
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

		public int Lag_Count = 0;
		public bool Is_Lag = false;
	}
}
