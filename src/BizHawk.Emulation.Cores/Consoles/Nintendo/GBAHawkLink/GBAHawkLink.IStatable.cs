using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink
	{
		private void SyncState(Serializer ser)
		{
			_controllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref Lag_Count);
			ser.Sync("IsLag", ref Is_Lag);

			ser.Sync(nameof(GBP_Mode_Enabled_L), ref GBP_Mode_Enabled_L);
			ser.Sync(nameof(GBP_Screen_Detection_L), ref GBP_Screen_Detection_L);
			ser.Sync(nameof(GBP_Screen_Count_L), ref GBP_Screen_Count_L);

			ser.Sync(nameof(GBP_Mode_Enabled_R), ref GBP_Mode_Enabled_R);
			ser.Sync(nameof(GBP_Screen_Detection_R), ref GBP_Screen_Detection_R);
			ser.Sync(nameof(GBP_Screen_Count_R), ref GBP_Screen_Count_R);

			if (ser.IsReader)
			{
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
				LibGBAHawkLink.GBALink_load_state(GBA_Pntr, GBA_core);
			}
			else
			{
				LibGBAHawkLink.GBALink_save_state(GBA_Pntr, GBA_core);
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
			}
		}
	}
}
