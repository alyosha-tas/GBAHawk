namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class LR35902
	{
		// this contains the vectors of instruction operations
		// NOTE: This list is NOT confirmed accurate for each individual cycle

		private void INC_16(ushort src_l, ushort src_h)
		{
			cur_instr = new[]
						{IDLE,
						IDLE,
						IDLE,
						INC16, src_l, src_h,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void DEC_16(ushort src_l, ushort src_h)
		{
			cur_instr = new[]
						{IDLE,
						IDLE,
						IDLE,
						DEC16, src_l, src_h,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}

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

		private void JR_COND(ushort cond)
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						RD, W, PCl, PCh,
						INC16, PCl, PCh,
						COND_CHECK, cond, (ushort)0,							
						IDLE,
						ASGN, Z, 0,
						IDLE,
						ADDS, PCl, PCh, W, Z,
						HALT_CHK,
						OP };
		}

		private void JP_COND(ushort cond)
		{
			cur_instr = new[]
						{IDLE,
						IDLE,
						IDLE,
						RD, W, PCl, PCh,
						IDLE,
						INC16, PCl, PCh,
						IDLE,
						RD, Z, PCl, PCh,
						INC16, PCl, PCh,
						COND_CHECK, cond, (ushort)1,
						IDLE,
						TR, PCl, W,
						IDLE,
						TR, PCh, Z,
						HALT_CHK,
						OP };
		}

		private void RET_()
		{
			cur_instr = new[]
						{IDLE,
						IDLE,
						IDLE,
						RD, PCl, SPl, SPh,
						IDLE,
						INC16, SPl, SPh,
						IDLE,
						RD, PCh, SPl, SPh,
						IDLE,
						INC16, SPl, SPh,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void RETI_()
		{
			cur_instr = new[]
						{IDLE,
						IDLE,
						IDLE,
						RD, PCl, SPl, SPh,
						IDLE,
						INC16, SPl, SPh,
						IDLE,
						RD, PCh, SPl, SPh,
						IDLE,
						INC16, SPl, SPh,
						IDLE,
						EI_RETI,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}


		private void RET_COND(ushort cond)
		{
			cur_instr = new[]
						{IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						COND_CHECK, cond, (ushort)2,
						IDLE,
						RD, PCl, SPl, SPh,
						IDLE,
						INC16, SPl, SPh,
						IDLE,
						RD, PCh, SPl, SPh,
						IDLE,
						INC16, SPl, SPh,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void CALL_COND(ushort cond)
		{
			cur_instr = new[]
						{IDLE,
						IDLE,
						IDLE,
						RD, W, PCl, PCh,
						INC16, PCl, PCh,
						IDLE,							
						IDLE,
						RD, Z, PCl, PCh,
						INC16, PCl, PCh,
						COND_CHECK, cond, (ushort)3,
						DEC16, SPl, SPh,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						WR, SPl, SPh, PCh,
						IDLE,							
						IDLE,
						DEC16, SPl, SPh,
						WR, SPl, SPh, PCl,				
						TR, PCl, W,
						TR, PCh, Z,
						HALT_CHK,
						OP };
		}

		private void INT_OP(ushort operation, ushort src)
		{
			cur_instr = new[]
						{operation, src,
						IDLE,
						HALT_CHK,
						OP };
		}

		private void BIT_OP(ushort operation, ushort bit, ushort src)
		{
			cur_instr = new[]
						{operation, bit, src,
						IDLE,
						HALT_CHK,
						OP };
		}
	}
}
