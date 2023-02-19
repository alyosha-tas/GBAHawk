using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public partial class GBAHawk
	{
		private void SyncState(Serializer ser)
		{
			if (cart_RAM != null)
			{
				ser.Sync(nameof(cart_RAM), ref cart_RAM, false);
			}

			// since we need rate of change of angle, need to savestate them
			ser.Sync(nameof(theta), ref theta);
			ser.Sync(nameof(phi), ref phi);
			ser.Sync(nameof(phi_prev), ref phi_prev);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref _lagCount);
			ser.Sync("IsLag", ref _isLag);

			if (ser.IsReader)
			{
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
				LibGBAHawk.GBA_load_state(GBA_Pntr, GBA_core);
			}
			else
			{
				LibGBAHawk.GBA_save_state(GBA_Pntr, GBA_core);
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
			}
		}
	}
}
