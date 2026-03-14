using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawk
{
	public partial class GBHawk
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
				ser.Sync(nameof(GB_core), ref GB_core, false);
				LibGBHawk.GB_load_state(GB_Pntr, GB_core);
			}
			else
			{
				LibGBHawk.GB_save_state(GB_Pntr, GB_core);
				ser.Sync(nameof(GB_core), ref GB_core, false);
			}
		}
	}
}
