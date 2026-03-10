namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class LR35902
	{
		// this contains the vectors of instruction operations
		// NOTE: This list is NOT confirmed accurate for each individual cycle

		private void REG_OP(ushort operation, ushort dest, ushort src)
		{
			cur_instr = new[]
						{operation, dest, src,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void HALT_()
		{
			cur_instr = new ushort[]
						{HALT_FUNC,
						IDLE,
						IDLE,
						OP_G,
						HALT_CHK,
						HALT_CHK_2,
						HALT, 0};
		}

		private void INT_OP(ushort operation, ushort src)
		{
			cur_instr = new[]
						{operation, src,
						IDLE,
						HALT_CHK,
						OP };
		}

	}
}
