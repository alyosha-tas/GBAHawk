using BizHawk.Common;

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

			if (ser.IsReader)
			{
				ser.Sync(nameof(GB_core), ref GB_core, false);
				LibGBHawkLink.GBLink_load_state(GB_Pntr, GB_core);
			}
			else
			{
				LibGBHawkLink.GBLink_save_state(GB_Pntr, GB_core);
				ser.Sync(nameof(GB_core), ref GB_core, false);
			}
		}
	}
}
