using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBCHawk
{
	public partial class GBCHawk
	{
		private void SyncState(Serializer ser)
		{
			_controllerDeck.SyncState(ser);

			ser.Sync("Lag", ref Lag_Count);
			ser.Sync("Frame", ref _frame);
			ser.Sync("IsLag", ref Is_Lag);

			ser.Sync(nameof(Current_sync_on_vbl), ref Current_sync_on_vbl);
			ser.Sync(nameof(Is_GB_in_GBC), ref Is_GB_in_GBC);
			ser.Sync(nameof(Is_GBC), ref Is_GBC);
			ser.Sync(nameof(Use_MT), ref Use_MT);

			if (ser.IsReader)
			{
				ser.Sync(nameof(GBC_core), ref GBC_core, false);
				LibGBCHawk.GBC_load_state(GBC_Pntr, GBC_core);
			}
			else
			{
				LibGBCHawk.GBC_save_state(GBC_Pntr, GBC_core);
				ser.Sync(nameof(GBC_core), ref GBC_core, false);
			}
		}
	}
}
