using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.N64.Common;

namespace BizHawk.Emulation.Cores.Nintendo.N64Hawk
{
	public partial class N64Hawk
	{
		private void SyncState(Serializer ser)
		{
			ControllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref Lag_Count);
			ser.Sync("IsLag", ref Is_Lag);

			if (ser.IsReader)
			{
				ser.Sync(nameof(N64_core), ref N64_core, false);
				LibN64Hawk.N64_load_state(N64_Pntr, N64_core);
			}
			else
			{
				LibN64Hawk.N64_save_state(N64_Pntr, N64_core);
				ser.Sync(nameof(N64_core), ref N64_core, false);
			}
		}
	}
}
