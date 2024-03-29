﻿using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug : IInputPollable
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

		internal bool Is_Lag = true;
		private int Lag_Count;
	}
}
