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
				ser.Sync(nameof(NES_core), ref NES_core, false);
				LibN64Hawk.N64_load_state(NES_Pntr, NES_core);
			}
			else
			{
				LibN64Hawk.N64_save_state(NES_Pntr, NES_core);
				ser.Sync(nameof(NES_core), ref NES_core, false);
			}
		}
	}
}
