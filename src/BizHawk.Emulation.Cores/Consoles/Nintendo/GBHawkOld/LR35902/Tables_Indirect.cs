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

		private void LD_8_IND(ushort dest_l, ushort dest_h, ushort src)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						WR, dest_l, dest_h, src,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void LD_8_IND_FF(ushort dest, ushort src_l, ushort src_h)
		{
			cur_instr = new ushort[]
						{IDLE,					
						IDLE,					
						IDLE,
						RD, W, src_l, src_h,
						INC16, src_l, src_h,
						IDLE,
						ASGN, Z , 0xFF,
						RD, dest, W, Z,
						IDLE,					
						IDLE,
						HALT_CHK,
						OP };
		}

		private void LD_16_IND_FF(ushort dest, ushort src_l, ushort src_h)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						RD, W, src_l, src_h,
						IDLE,
						INC16, src_l, src_h,
						IDLE,
						RD, Z, src_l, src_h,
						IDLE,
						INC16, src_l, src_h,
						IDLE,
						RD, dest, W, Z,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void LD_FF_IND_16(ushort dest_l, ushort dest_h, ushort src)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						RD, W, dest_l, dest_h,
						IDLE,
						INC16, dest_l, dest_h,
						IDLE,
						RD, Z, dest_l, dest_h,
						IDLE,
						INC16, dest_l, dest_h,
						IDLE,
						WR, W, Z, src,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}
	}
}
