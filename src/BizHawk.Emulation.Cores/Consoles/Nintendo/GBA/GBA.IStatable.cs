using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public partial class GBA
	{
		private void SyncState(Serializer ser)
		{
			ser.BeginSection("GBA");

			if (cart_RAM != null)
			{
				ser.Sync(nameof(cart_RAM), ref cart_RAM, false);
			}

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref _lagCount);
			ser.Sync("IsLag", ref _isLag);

			ser.EndSection();

			if (ser.IsReader)
			{
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
				LibGBA.GBA_load_state(GBA_Pntr, GBA_core);
			}
			else
			{
				LibGBA.GBA_save_state(GBA_Pntr, GBA_core);
				ser.Sync(nameof(GBA_core), ref GBA_core, false);
			}
		}
	}
}
