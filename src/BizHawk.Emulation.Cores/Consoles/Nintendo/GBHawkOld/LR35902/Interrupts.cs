namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class LR35902
	{
		private void INTERRUPT_()
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						DEC16, SPl, SPh,
						IDLE,
						WR, SPl, SPh, PCh,
						ASGN, PCh, 0,
						DEC16, SPl, SPh,
						INT_GET, 1, W,
						WR, SPl, SPh, PCl,
						IDLE,
						IRQ_CLEAR,
						TR, PCl, W,
						OP };
		}

		private void INTERRUPT_GBC_NOP()
		{
			cur_instr = new ushort[]
						{IDLE,
						IDLE,
						IDLE,
						HDMA_UPD,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						IDLE,
						DEC16, SPl, SPh,
						IDLE,
						WR, SPl, SPh, PCh,
						ASGN, PCh, 0,
						DEC16, SPl, SPh,
						INT_GET, 1, W,
						WR, SPl, SPh, PCl,
						IDLE,
						IRQ_CLEAR,
						TR, PCl, W,
						OP };
		}

		public ushort int_src;
		public byte int_clear;
		public int stop_time;
		public bool stop_check;


		private void ResetInterrupts()
		{


			int_src = 5;
			int_clear = 0;
		}
	}
}