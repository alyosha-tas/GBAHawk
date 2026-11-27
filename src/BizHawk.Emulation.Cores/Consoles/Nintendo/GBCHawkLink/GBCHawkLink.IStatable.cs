using BizHawk.Common;

using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBCLink
{
	public partial class GBCHawkLink
	{
		private void SyncState(Serializer ser)
		{
			_controllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref Lag_Count);
			ser.Sync("IsLag", ref Is_Lag);

			ser.Sync(nameof(Current_sync_on_vbl), ref Current_sync_on_vbl, false);
			ser.Sync(nameof(_cableconnected), ref _cableconnected);
			ser.Sync(nameof(_cablediscosignal), ref _cablediscosignal);

			ser.Sync(nameof(_cableconnected_AC), ref _cableconnected_AC);
			ser.Sync(nameof(_cableconnected_BC), ref _cableconnected_BC);
			ser.Sync(nameof(_cableconnected_AB), ref _cableconnected_AB);

			ser.Sync(nameof(_cableconnected_UD), ref _cableconnected_UD);
			ser.Sync(nameof(_cableconnected_LR), ref _cableconnected_LR);
			ser.Sync(nameof(_cableconnected_X), ref _cableconnected_X);
			ser.Sync(nameof(_cableconnected_4x), ref _cableconnected_4x);

			if (ser.IsReader)
			{
				ser.Sync(nameof(GBC_core), ref GBC_core, false);
				LibGBCHawkLink.GBCLink_load_state(GBCLink_Pntr, GBC_core);
			}
			else
			{
				LibGBCHawkLink.GBCLink_save_state(GBCLink_Pntr, GBC_core);
				ser.Sync(nameof(GBC_core), ref GBC_core, false);
			}
		}
	}
}
