using System;
using System.Collections.Generic;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		// Instruction types
		public const ushort cpu_Internal_And_Prefetch_ARM = 0;
		public const ushort cpu_Internal_And_Prefetch_2_ARM = 1;
		public const ushort cpu_Internal_And_Prefetch_3_ARM = 2;
		public const ushort cpu_Internal_And_Branch_1_ARM = 3;
		public const ushort cpu_Internal_And_Branch_2_ARM = 4;
		public const ushort cpu_Internal_And_Branch_3_ARM = 5;
		public const ushort cpu_Prefetch_Only_1_ARM = 6;
		public const ushort cpu_Prefetch_Only_2_ARM = 7;
		public const ushort cpu_Prefetch_And_Load_Store_ARM = 8;
		public const ushort cpu_Load_Store_Word_ARM = 9;
		public const ushort cpu_Load_Store_Half_ARM = 10;
		public const ushort cpu_Load_Store_Byte_ARM = 11;
		public const ushort cpu_Multi_Load_Store_ARM = 12;
		public const ushort cpu_Multiply_ARM = 13;
		public const ushort cpu_Prefetch_Swap_ARM = 14;
		public const ushort cpu_Swap_ARM = 15;
		public const ushort cpu_Prefetch_And_Branch_Ex_ARM = 16;

		public const ushort cpu_Internal_And_Prefetch_TMB = 20;
		public const ushort cpu_Internal_And_Prefetch_2_TMB = 21;
		public const ushort cpu_Internal_And_Branch_1_TMB = 22;
		public const ushort cpu_Prefetch_Only_1_TMB = 23;
		public const ushort cpu_Prefetch_Only_2_TMB = 24;
		public const ushort cpu_Prefetch_And_Load_Store_TMB = 25;
		public const ushort cpu_Load_Store_Word_TMB = 26;
		public const ushort cpu_Load_Store_Half_TMB = 27;
		public const ushort cpu_Load_Store_Byte_TMB = 28;
		public const ushort cpu_Multi_Load_Store_TMB = 29;
		public const ushort cpu_Multiply_TMB = 30;
		public const ushort cpu_Prefetch_And_Branch_Ex_TMB = 31;

		// SWI and undefined opcode cycles, same for both Thumb and ARM
		public const ushort cpu_Prefetch_And_SWI_Undef = 38;
		public const ushort cpu_Prefetch_IRQ = 39;
		public const ushort cpu_Internal_Reset_1 = 40;
		public const ushort cpu_Internal_Reset_2 = 41;

		// These operations are all internal cycles, they can take place while a DMA is occuring
		public const ushort cpu_Internal_And_Branch_4_ARM = 42;
		public const ushort cpu_Internal_Can_Save_ARM = 43;
		public const ushort cpu_Internal_Can_Save_TMB = 44;
		public const ushort cpu_Internal_Halted = 46;
		public const ushort cpu_Multiply_Cycles = 47;
		public const ushort cpu_Pause_For_DMA = 48;

		// Instruction Operations ARM
		public const ushort cpu_ARM_AND = 10;
		public const ushort cpu_ARM_EOR = 11;
		public const ushort cpu_ARM_SUB = 12;
		public const ushort cpu_ARM_RSB = 13;
		public const ushort cpu_ARM_ADD = 14;
		public const ushort cpu_ARM_ADC = 15;
		public const ushort cpu_ARM_SBC = 16;
		public const ushort cpu_ARM_RSC = 17;
		public const ushort cpu_ARM_TST = 18;
		public const ushort cpu_ARM_TEQ = 19;
		public const ushort cpu_ARM_CMP = 20;
		public const ushort cpu_ARM_CMN = 21;
		public const ushort cpu_ARM_ORR = 22;
		public const ushort cpu_ARM_MOV = 23;
		public const ushort cpu_ARM_BIC = 24;
		public const ushort cpu_ARM_MVN = 25;
		public const ushort cpu_ARM_MSR = 26;
		public const ushort cpu_ARM_MRS = 27;
		public const ushort cpu_ARM_Bx = 28;
		public const ushort cpu_ARM_MUL = 29;
		public const ushort cpu_ARM_MUL_UL = 30;
		public const ushort cpu_ARM_MUL_SL = 31;
		public const ushort cpu_ARM_Swap = 32;
		public const ushort cpu_ARM_Imm_LS = 70;
		public const ushort cpu_ARM_Reg_LS = 80;
		public const ushort cpu_ARM_Multi_1 = 90;
		public const ushort cpu_ARM_Branch = 100;
		public const ushort cpu_ARM_Cond_Check_Only = 110;

		// Instruction Operations Thumb
		public const ushort cpu_Thumb_Shift = 5;
		public const ushort cpu_Thumb_Add_Sub_Reg = 6;
		public const ushort cpu_Thumb_AND = 10;
		public const ushort cpu_Thumb_EOR = 11;
		public const ushort cpu_Thumb_LSL = 12;
		public const ushort cpu_Thumb_LSR = 13;
		public const ushort cpu_Thumb_ASR = 14;
		public const ushort cpu_Thumb_ADC = 15;
		public const ushort cpu_Thumb_SBC = 16;
		public const ushort cpu_Thumb_ROR = 17;
		public const ushort cpu_Thumb_TST = 18;
		public const ushort cpu_Thumb_NEG = 19;
		public const ushort cpu_Thumb_CMP = 20;
		public const ushort cpu_Thumb_CMN = 21;
		public const ushort cpu_Thumb_ORR = 22;
		public const ushort cpu_Thumb_MUL = 23;
		public const ushort cpu_Thumb_BIC = 24;
		public const ushort cpu_Thumb_MVN = 25;

		public const ushort cpu_Thumb_High_Add = 30;
		public const ushort cpu_Thumb_High_Cmp = 31;
		public const ushort cpu_Thumb_High_Bx = 32;
		public const ushort cpu_Thumb_High_MOV = 33;
		public const ushort cpu_Thumb_ALU_Imm = 34;
		public const ushort cpu_Thumb_PC_Rel_LS = 35;
		public const ushort cpu_Thumb_Rel_LS = 36;
		public const ushort cpu_Thumb_Imm_LS = 37;
		public const ushort cpu_Thumb_Half_LS = 38;
		public const ushort cpu_Thumb_SP_REL_LS = 39;
		public const ushort cpu_Thumb_Add_SP_PC = 40;
		public const ushort cpu_Thumb_Add_Sub_Stack = 41;
		public const ushort cpu_Thumb_Push_Pop = 50;
		public const ushort cpu_Thumb_Multi_1 = 60;
		public const ushort cpu_Thumb_Branch = 100;
		public const ushort cpu_Thumb_Branch_Cond = 101;
		public const ushort cpu_Thumb_Branch_Link_1 = 102;
		public const ushort cpu_Thumb_Branch_Link_2 = 103;

		// Exception Types
		public const ushort cpu_IRQ_Exc = 0;
		public const ushort cpu_SWI_Exc = 1;
		public const ushort cpu_Undef_Exc = 2;
		public const ushort cpu_Reset_Exc = 3;

		// ALU related
		public const ulong cpu_Carry_Compare = 0x100000000;
		public const ulong cpu_Long_Neg_Compare = 0x8000000000000000;

		public const uint cpu_Neg_Compare = 0x80000000;
		public const uint cpu_Cast_Int = 0xFFFFFFFF;

		// Masks
		public const uint cpu_Unalloc_Mask = 0x0FFFFF00;
		public const uint cpu_User_Mask = 0xF0000000;
		public const uint cpu_Priv_Mask = 0x000000DF;
		public const uint cpu_State_Mask = 0x00000020;
	}
}