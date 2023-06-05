using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBA
{
	public partial class SubGBAHawk
	{
		private void SyncState(Serializer ser)
		{
			if (cart_RAM != null)
			{
				ser.Sync(nameof(cart_RAM), ref cart_RAM, false);
			}

			_controllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref _lagCount);
			ser.Sync("IsLag", ref _isLag);

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
