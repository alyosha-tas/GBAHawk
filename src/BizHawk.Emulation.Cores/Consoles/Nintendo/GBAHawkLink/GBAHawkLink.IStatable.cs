using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;

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

			ser.Sync(nameof(GBP_Mode_Enabled_A), ref GBP_Mode_Enabled_A);
			ser.Sync(nameof(GBP_Screen_Detection_A), ref GBP_Screen_Detection_A);
			ser.Sync(nameof(GBP_Screen_Count_A), ref GBP_Screen_Count_A);

			ser.Sync(nameof(GBP_Mode_Enabled_B), ref GBP_Mode_Enabled_B);
			ser.Sync(nameof(GBP_Screen_Detection_B), ref GBP_Screen_Detection_B);
			ser.Sync(nameof(GBP_Screen_Count_B), ref GBP_Screen_Count_B);

			ser.Sync(nameof(GBP_Mode_Enabled_C), ref GBP_Mode_Enabled_C);
			ser.Sync(nameof(GBP_Screen_Detection_C), ref GBP_Screen_Detection_C);
			ser.Sync(nameof(GBP_Screen_Count_C), ref GBP_Screen_Count_C);

			ser.Sync(nameof(GBP_Mode_Enabled_D), ref GBP_Mode_Enabled_D);
			ser.Sync(nameof(GBP_Screen_Detection_D), ref GBP_Screen_Detection_D);
			ser.Sync(nameof(GBP_Screen_Count_D), ref GBP_Screen_Count_D);

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
