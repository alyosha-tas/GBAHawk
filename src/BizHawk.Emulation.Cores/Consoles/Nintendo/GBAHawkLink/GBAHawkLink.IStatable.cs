using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink
	{
		private void SyncState(Serializer ser)
		{
			if (cart_RAMS[0] != null)
			{
				ser.Sync("cart_RAM 0", ref cart_RAMS[0], false);
			}

			if (cart_RAMS[1] != null)
			{
				ser.Sync("cart_RAM 1", ref cart_RAMS[1], false);
			}

			_controllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref Lag_Count);
			ser.Sync("IsLag", ref Is_Lag);

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
