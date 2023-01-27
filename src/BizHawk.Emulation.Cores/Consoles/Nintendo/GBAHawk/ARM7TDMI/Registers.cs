using System;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk
{
#pragma warning disable CS0675 // Bitwise-or operator used on a sign-extended operand


	public partial class GBAHawk
	{
		// register contents are saved and copied out during mode switches

		uint cpu_user_R8, cpu_user_R9, cpu_user_R10, cpu_user_R11, cpu_user_R12, cpu_user_R13, cpu_user_R14;
		uint cpu_spr_R13, cpu_spr_R14, cpu_spr_S;
		uint cpu_abort_R13, cpu_abort_R14, cpu_abort_S;
		uint cpu_undf_R13, cpu_undf_R14, cpu_undf_S;
		uint cpu_intr_R13, cpu_intr_R14, cpu_intr_S;
		uint cpu_fiq_R8, cpu_fiq_R9, cpu_fiq_R10, cpu_fiq_R11, cpu_fiq_R12, cpu_fiq_R13, cpu_fiq_R14, cpu_fiq_S;

		public uint[] cpu_Regs = new uint[18];

		public bool cpu_FlagN
		{
			get => (cpu_Regs[16] & 0x80000000) != 0;
			set => cpu_Regs[16] = (uint)((cpu_Regs[16] & ~0x80000000) | (value ? 0x80000000 : 0x00000000));
		}

		public bool cpu_FlagZ
		{
			get => (cpu_Regs[16] & 0x40000000) != 0;
			set => cpu_Regs[16] = (uint)((cpu_Regs[16] & ~0x40000000) | (value ? 0x40000000 : 0x00000000));
		}

		public bool cpu_FlagC
		{
			get => (cpu_Regs[16] & 0x20000000) != 0;
			set => cpu_Regs[16] = (uint)((cpu_Regs[16] & ~0x20000000) | (value ? 0x20000000 : 0x00000000));
		}

		public bool cpu_FlagV
		{
			get => (cpu_Regs[16] & 0x10000000) != 0;
			set => cpu_Regs[16] = (uint)((cpu_Regs[16] & ~0x10000000) | (value ? 0x10000000 : 0x00000000));
		}

		public bool cpu_FlagI
		{
			get => (cpu_Regs[16] & 0x00000080) != 0;
			set => cpu_Regs[16] = (uint)((cpu_Regs[16] & ~0x00000080) | (value ? 0x00000080 : 0x00000000));
		}

		public bool cpu_FlagF
		{
			get => (cpu_Regs[16] & 0x00000040) != 0;
			set => cpu_Regs[16] = (uint)((cpu_Regs[16] & ~0x00000040) | (value ? 0x00000040 : 0x00000000));
		}

		public bool cpu_FlagT
		{
			get => (cpu_Regs[16] & 0x00000020) != 0;
			set => cpu_Regs[16] = (uint)((cpu_Regs[16] & ~0x00000020) | (value ? 0x00000020 : 0x00000000));
		}

		public void ResetRegisters()
		{
			cpu_user_R8 = cpu_user_R9 = cpu_user_R10 = cpu_user_R11 = cpu_user_R12 = cpu_user_R13 = cpu_user_R14 = 0;
			cpu_spr_R13 = cpu_spr_R14 = cpu_spr_S = 0;
			cpu_abort_R13 = cpu_abort_R14 = cpu_abort_S = 0;
			cpu_undf_R13 = cpu_undf_R14 = cpu_undf_S = 0;
			cpu_intr_R13 = cpu_intr_R14 = cpu_intr_S = 0;
			cpu_fiq_R8 = cpu_fiq_R9 = cpu_fiq_R10 = cpu_fiq_R11 = cpu_fiq_R12 = cpu_fiq_R13 = cpu_fiq_R14 = cpu_fiq_S = 0;

			for (int i=0; i < 18; i++)
			{
				cpu_Regs[i] = 0;
			}

			// The processor starts in ARM, supervisor mode, with interrupts disabled
			cpu_Regs[16] = 0x13;

			cpu_FlagI = true;
			cpu_FlagF = true;

		}

		// NOTTE: system and user have same regs
		public void cpu_Swap_Regs(uint New_State, bool C_to_S, bool S_to_C)
		{
			uint cpu_s_to_c_reg = cpu_Regs[17];

			// user and system
			if (((cpu_Regs[16] & 0x1F) == 0x10) || ((cpu_Regs[16] & 0x1F) == 0x1F))
			{
				cpu_user_R13 = cpu_Regs[13];
				cpu_user_R14 = cpu_Regs[14];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					// nothing to do
				}
				else if (New_State ==0x11)		// FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// FIQ
			if ((cpu_Regs[16] & 0x1F) == 0x11)
			{
				cpu_fiq_R8 = cpu_Regs[8];
				cpu_fiq_R9 = cpu_Regs[9];
				cpu_fiq_R10 = cpu_Regs[10];
				cpu_fiq_R11 = cpu_Regs[11];
				cpu_fiq_R12 = cpu_Regs[12];
				cpu_fiq_R13 = cpu_Regs[13];
				cpu_fiq_R14 = cpu_Regs[14];
				cpu_fiq_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					// nothing to do
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[8] = cpu_user_R8;
					cpu_Regs[9] = cpu_user_R9;
					cpu_Regs[10] = cpu_user_R10;
					cpu_Regs[11] = cpu_user_R11;
					cpu_Regs[12] = cpu_user_R12;

					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// IRQ
			if ((cpu_Regs[16] & 0x1F) == 0x12)
			{
				cpu_intr_R13 = cpu_Regs[13];
				cpu_intr_R14 = cpu_Regs[14];
				cpu_intr_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))		// user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;			
				}
				else if (New_State == 0x11)		// FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					// nothing to do
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// Supervisor
			if ((cpu_Regs[16] & 0x1F) == 0x13)
			{
				cpu_spr_R13 = cpu_Regs[13];
				cpu_spr_R14 = cpu_Regs[14];
				cpu_spr_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					// nothing to so
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// Abort
			if ((cpu_Regs[16] & 0x1F) == 0x17)
			{
				cpu_abort_R13 = cpu_Regs[13];
				cpu_abort_R14 = cpu_Regs[14];
				cpu_abort_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					// nothing to so
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					cpu_Regs[13] = cpu_undf_R13;
					cpu_Regs[14] = cpu_undf_R14;
					cpu_Regs[17] = cpu_undf_S;
				}
			}

			// Undefined Instruction
			if ((cpu_Regs[16] & 0x1F) == 0x1B)
			{
				cpu_undf_R13 = cpu_Regs[13];
				cpu_undf_R14 = cpu_Regs[14];
				cpu_undf_S = cpu_Regs[17];

				if ((New_State == 0x10) || (New_State == 0x1F))     // user and system
				{
					cpu_Regs[13] = cpu_user_R13;
					cpu_Regs[14] = cpu_user_R14;
				}
				else if (New_State == 0x11)     // FIQ
				{
					cpu_user_R8 = cpu_Regs[8];
					cpu_user_R9 = cpu_Regs[9];
					cpu_user_R10 = cpu_Regs[10];
					cpu_user_R11 = cpu_Regs[11];
					cpu_user_R12 = cpu_Regs[12];

					cpu_Regs[8] = cpu_fiq_R8;
					cpu_Regs[9] = cpu_fiq_R9;
					cpu_Regs[10] = cpu_fiq_R10;
					cpu_Regs[11] = cpu_fiq_R11;
					cpu_Regs[12] = cpu_fiq_R12;
					cpu_Regs[13] = cpu_fiq_R13;
					cpu_Regs[14] = cpu_fiq_R14;
					cpu_Regs[17] = cpu_fiq_S;
				}
				else if (New_State == 0x12)     // IRQ
				{
					cpu_Regs[13] = cpu_intr_R13;
					cpu_Regs[14] = cpu_intr_R14;
					cpu_Regs[17] = cpu_intr_S;
				}
				else if (New_State == 0x13)     // Supervisor
				{
					cpu_Regs[13] = cpu_spr_R13;
					cpu_Regs[14] = cpu_spr_R14;
					cpu_Regs[17] = cpu_spr_S;
				}
				else if (New_State == 0x17)     // Abort
				{
					cpu_Regs[13] = cpu_abort_R13;
					cpu_Regs[14] = cpu_abort_R14;
					cpu_Regs[17] = cpu_abort_S;
				}
				else if (New_State == 0x1B)     // Undefined Instruction
				{
					// nothing to so
				}
			}

			if (C_to_S) { cpu_Regs[17] = cpu_Regs[16]; }

			if (S_to_C) { cpu_Regs[16] = cpu_s_to_c_reg; }

			cpu_Regs[16] &= 0xFFFFFFE0;
			cpu_Regs[16] |= New_State;
		}
	}

#pragma warning restore CS0675 // Bitwise-or operator used on a sign-extended operand
}