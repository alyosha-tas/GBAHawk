using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBA
{
	public partial class SubGBAHawk
	{
		private void SyncState(Serializer ser)
		{
			_controllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref Lag_Count);
			ser.Sync("IsLag", ref Is_Lag);

			ser.Sync(nameof(GBP_Mode_Enabled), ref GBP_Mode_Enabled);
			ser.Sync(nameof(GBP_Screen_Detection), ref GBP_Screen_Detection);
			ser.Sync(nameof(GBP_Screen_Count), ref GBP_Screen_Count);

			if (ser.IsReader)
			{
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
				LibSubGBAHawk.GBA_load_state(GBA_Pntr, GBA_core);
			}
			else
			{
				LibSubGBAHawk.GBA_save_state(GBA_Pntr, GBA_core);
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
			}
		}
	}
}
