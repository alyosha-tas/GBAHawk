using BizHawk.Common;

using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	public partial class GBHawkLink
	{
		private void SyncState(Serializer ser)
		{
			_controllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref Lag_Count);
			ser.Sync("IsLag", ref Is_Lag);

			ser.Sync(nameof(Current_sync_on_vbl), ref Current_sync_on_vbl, false);

			if (ser.IsReader)
			{
				ser.Sync(nameof(GB_core), ref GB_core, false);
				LibGBHawkLink.GBLink_load_state(GBLink_Pntr, GB_core);
			}
			else
			{
				LibGBHawkLink.GBLink_save_state(GBLink_Pntr, GB_core);
				ser.Sync(nameof(GB_core), ref GB_core, false);
			}
		}
	}
}
