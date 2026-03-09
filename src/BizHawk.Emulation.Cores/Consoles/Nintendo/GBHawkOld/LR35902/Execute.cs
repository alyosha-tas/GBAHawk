namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class LR35902
	{
		public ulong TotalExecutedCycles;
		public ulong instruction_start;

		private int EI_pending;
		public bool interrupts_enabled;

		// variables for executing instructions
		public int instr_pntr = 0;
		public ushort[] cur_instr = new ushort [60];
		public ushort[] instr_table = new ushort[256 * 2 * 60 + 60 * 10];
		public bool CB_prefix;
		public bool halted;
		public bool stopped;
		public bool jammed;
		public int LY;

		// unsaved variables
		public bool checker;
		private byte interrupt_src_reg, interrupt_enable_reg, buttons_pressed;

	}
}