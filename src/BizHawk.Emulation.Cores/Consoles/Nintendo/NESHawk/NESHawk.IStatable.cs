using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.NES.Common;

namespace BizHawk.Emulation.Cores.Nintendo.NESHawk
{
	public partial class NESHawk
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
				LibNESHawk.NES_load_state(NES_Pntr, NES_core);
			}
			else
			{
				LibNESHawk.NES_save_state(NES_Pntr, NES_core);
				ser.Sync(nameof(NES_core), ref NES_core, false);
			}
		}
	}
}
