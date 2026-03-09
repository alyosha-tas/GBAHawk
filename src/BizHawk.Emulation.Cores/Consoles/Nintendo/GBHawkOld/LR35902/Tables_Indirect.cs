namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class LR35902
	{
		private void INT_OP_IND(ushort operation, ushort src_l, ushort src_h)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						RD, Z, src_l, src_h,
						IDLE,
						operation, Z,
						IDLE,
						WR, src_l, src_h, Z,
						IDLE,					
						IDLE,
						HALT_CHK,
						OP };
		}

		private void BIT_OP_IND(ushort operation, ushort bit, ushort src_l, ushort src_h)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,	
						RD, Z, src_l, src_h,
						IDLE,
						operation, bit, Z,
						IDLE,
						WR, src_l, src_h, Z,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void BIT_TE_IND(ushort operation, ushort bit, ushort src_l, ushort src_h)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						RD, Z, src_l, src_h,
						IDLE,
						operation, bit, Z,
						HALT_CHK,
						OP };
		}

		private void REG_OP_IND_INC(ushort operation, ushort dest, ushort src_l, ushort src_h)
		{
			cur_instr = new ushort[]
						{IDLE,					
						IDLE,					
						IDLE,
						RD, Z, src_l, src_h,
						operation, dest, Z,
						INC16, src_l, src_h,
						HALT_CHK,
						OP };
		}

		private void REG_OP_IND(ushort operation, ushort dest, ushort src_l, ushort src_h)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						RD, Z, src_l, src_h,
						IDLE,
						operation, dest, Z,
						HALT_CHK,
						OP };
		}
	}
}
