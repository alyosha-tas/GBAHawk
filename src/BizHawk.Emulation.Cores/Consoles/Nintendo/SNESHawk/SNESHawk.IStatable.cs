using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.SNES.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SNESHawk
{
	public partial class SNESHawk
	{
		private void SyncState(Serializer ser)
		{
			ControllerDeck.SyncState(ser);

			ser.Sync("Frame", ref _frame);
			ser.Sync("LagCount", ref Lag_Count);
			ser.Sync("IsLag", ref Is_Lag);

			if (ser.IsReader)
			{
				ser.Sync(nameof(NES_core), ref NES_core, false);
				LibSNESHawk.SNES_load_state(NES_Pntr, NES_core);
			}
			else
			{
				LibSNESHawk.SNES_save_state(NES_Pntr, NES_core);
				ser.Sync(nameof(NES_core), ref NES_core, false);
			}
		}
	}
}
