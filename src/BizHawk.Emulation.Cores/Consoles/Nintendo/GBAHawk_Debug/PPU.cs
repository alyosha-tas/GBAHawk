using BizHawk.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
/*
	PPU Emulation
	NOTES: 

	Does turning on interrupts in the STAT register trigger interrupts if conditions are met? For now assume no.

	TODO: check timing on window y - parameter check
*/

#pragma warning disable CS0675 // Bitwise-or operator used on a sign-extended operand

	public partial class GBAHawk_Debug
	{		
		public static readonly int[] ppu_OBJ_Sizes_X = {8, 16, 8, 8, 
														16, 32, 8, 8,
														32, 32, 16, 8,
														64, 64, 32, 8};

		public static readonly int[] ppu_OBJ_Sizes_Y = {8, 8, 16, 8,
														16, 8, 32, 8,
														32, 16, 32, 8,
														64, 32, 64, 8};

		public double[] ppu_Fract_Parts = new double[256];

		public ushort[] ppu_BG_CTRL = new ushort[4];
		public ushort[] ppu_BG_X = new ushort[4];
		public ushort[] ppu_BG_Y = new ushort[4];
		public ushort[] ppu_BG_X_Latch = new ushort[4];
		public ushort[] ppu_BG_Y_Latch = new ushort[4];

		public uint[] ppu_BG_Ref_X = new uint[4];
		public uint[] ppu_BG_Ref_Y = new uint[4];

		public int[] ppu_BG_On_Update_Time = new int[4];

		public ushort[] ppu_BG_Rot_A = new ushort[4];
		public ushort[] ppu_BG_Rot_B = new ushort[4];
		public ushort[] ppu_BG_Rot_C = new ushort[4];
		public ushort[] ppu_BG_Rot_D = new ushort[4];

		public ushort[] ppu_ROT_REF_LY = new ushort[4];

		public bool[] ppu_BG_On = new bool[4];
		public bool[] ppu_BG_On_New = new bool[4];

		public bool[] ppu_BG_Ref_X_Change = new bool[4];
		public bool[] ppu_BG_Ref_LY_Change = new bool[4];

		public int ppu_BG_Mode, ppu_Display_Frame;
		public int ppu_X_RS, ppu_Y_RS;
		public int ppu_Forced_Blank_Time;
		public int ppu_OBJ_On_Time;

		public int ppu_VBL_IRQ_cd, ppu_HBL_IRQ_cd, ppu_LYC_IRQ_cd, ppu_Sprite_cd;

		public int ppu_LYC_Vid_Check_cd;

		public ushort ppu_CTRL, ppu_Green_Swap, ppu_Cycle, ppu_Display_Cycle, ppu_Sprite_Eval_Time;
		public ushort ppu_WIN_Hor_0, ppu_WIN_Hor_1, ppu_WIN_Vert_0, ppu_WIN_Vert_1;
		public ushort ppu_WIN_In, ppu_WIN_Out, ppu_Mosaic, ppu_Special_FX, ppu_Alpha, ppu_Bright;

		public byte ppu_STAT, ppu_LY, ppu_LYC;

		public bool ppu_HBL_Free, ppu_OBJ_Dim, ppu_Forced_Blank, ppu_Any_Window_On;
		public bool ppu_OBJ_On, ppu_WIN0_On, ppu_WIN1_On, ppu_OBJ_WIN;
		public bool ppu_WIN0_Active, ppu_WIN1_Active;

		public bool ppu_In_VBlank;
		public bool ppu_Delays;
		public bool ppu_Sprite_Delays;
		public bool ppu_Do_Green_Swap;

		public bool ppu_VRAM_In_Use, ppu_VRAM_High_In_Use, ppu_PALRAM_In_Use;

		public bool ppu_VRAM_Access, ppu_VRAM_High_Access;
		public bool ppu_PALRAM_Access;
		public bool ppu_OAM_Access;

		// Sprite Evaluation
		public uint[] ppu_Sprite_Pixels = new uint[240 * 2];
		public int[] ppu_Sprite_Priority = new int[240 * 2];

		public bool[] ppu_Sprite_Pixel_Occupied = new bool[240 * 2];
		public bool[] ppu_Sprite_Semi_Transparent = new bool[240 * 2];
		public bool[] ppu_Sprite_Object_Window = new bool[240 * 2];
		public bool[] ppu_Sprite_Is_Mosaic = new bool[240 * 2];

		public uint ppu_Cur_Sprite_X;
		public uint ppu_Cur_Sprite_Y;
		public uint ppu_Cur_Sprite_Y_Temp;

		public int ppu_Current_Sprite;
		public int ppu_Process_Sprite;
		public int ppu_Process_Sprite_Temp;
		public int ppu_Sprite_ofst_eval;
		public int ppu_Sprite_ofst_draw;
		public int ppu_Sprite_X_Pos, ppu_Sprite_Y_Pos;
		public int ppu_Sprite_X_Pos_Temp, ppu_Sprite_Y_Pos_Temp;
		public int ppu_Sprite_X_Size, ppu_Sprite_Y_Size;
		public int ppu_Sprite_X_Size_Temp, ppu_Sprite_Y_Size_Temp;
		public int ppu_Sprite_Render_Cycle;
		public int ppu_Fetch_OAM_A_D_Cnt;
		public int ppu_Fetch_Sprite_VRAM_Cnt;
		public int ppu_Sprite_VRAM_Mod;
		public int ppu_Sprite_X_Scale;
		public int ppu_Sprite_Size_X_Ofst;
		public int ppu_Sprite_Size_Y_Ofst;
		public int ppu_Sprite_Size_X_Ofst_Temp;
		public int ppu_Sprite_Size_Y_Ofst_Temp;
		public int ppu_Sprite_Mode;
		public int ppu_Sprite_Next_Fetch;
		public int ppu_Param_Pick;
		public int ppu_Sprite_Mosaic_Y_Counter;
		public int ppu_Sprite_Mosaic_Y_Compare;

		public ushort ppu_Sprite_Attr_0, ppu_Sprite_Attr_1, ppu_Sprite_Attr_2;
		public ushort ppu_Sprite_Attr_0_Temp, ppu_Sprite_Attr_1_Temp;

		public ushort ppu_Sprite_A_Latch, ppu_Sprite_B_Latch, ppu_Sprite_C_Latch, ppu_Sprite_D_Latch;

		public byte ppu_Sprite_LY_Check;

		public bool ppu_Rot_Scale;
		public bool ppu_Rot_Scale_Temp;
		public bool ppu_Fetch_OAM_0, ppu_Fetch_OAM_2, ppu_Fetch_OAM_A_D;
		public bool ppu_Fetch_Sprite_VRAM;
		public bool ppu_New_Sprite, ppu_Sprite_Eval_Finished;
		public bool ppu_Sprite_Mosaic;

		// latched sprite pixel parameters
		public uint ppu_Sprite_Pixel_Latch;

		public int ppu_Sprite_Priority_Latch;

		public bool ppu_Sprite_Semi_Transparent_Latch;
		public bool ppu_Sprite_Mosaic_Latch;
		public bool ppu_Sprite_Pixel_Occupied_Latch;

		// BG rendering
		public int[] ppu_Fetch_Count = new int[4];
		public int[] ppu_Scroll_Cycle = new int[4];

		public int[] ppu_Pixel_Color = new int[4];
		public int[] ppu_Pixel_Color_2 = new int[4];
		public int[] ppu_Pixel_Color_1 = new int[4];
		public int[] ppu_Pixel_Color_M = new int[4];
		public int[] ppu_Pixel_Color_R = new int[4];
		public int[] ppu_Tile_Addr = new int[4];
		public int[] ppu_Y_Flip_Ofst = new int[4];

		public ushort[] ppu_BG_Start_Time = new ushort[4];

		public byte[] ppu_BG_Effect_Byte = new byte[4];
		public byte[] ppu_BG_Effect_Byte_New = new byte[4];

		public bool[] ppu_BG_Rendering_Complete = new bool[4];

		public bool[] ppu_BG_Has_Pixel = new bool[4];
		public bool[] ppu_BG_Has_Pixel_2 = new bool[4];
		public bool[] ppu_BG_Has_Pixel_1 = new bool[4];
		public bool[] ppu_BG_Has_Pixel_M = new bool[4];
		public bool[] ppu_BG_Has_Pixel_R = new bool[4];

		public uint ppu_BG_Pixel_F;
		public uint ppu_BG_Pixel_S;
		public uint ppu_Final_Pixel;
		public uint ppu_Blend_Pixel;

		public ushort ppu_BG_Mosaic_X_Mod;

		public bool ppu_Brighten_Final_Pixel;
		public bool ppu_Blend_Final_Pixel;

		// Palette fetches for BG's are true unless they happen from mode 3 or 5, where the color is directly encoded in VRAM
		// When this occurs, the first BG pixel will always be from BG 2, and the second from backdrop
		public bool ppu_Fetch_BG;

		public bool ppu_Fetch_Target_1;
		public bool ppu_Fetch_Target_2;

		public bool ppu_Rendering_Complete;
		public bool ppu_PAL_Rendering_Complete;

		public ushort ppu_VRAM_Open_Bus;

		public uint ppu_Base_LY_2, ppu_Base_LY_3;

		public ulong ppu_Current_Ref_X_2, ppu_Current_Ref_Y_2;
		public ulong ppu_Current_Ref_X_3, ppu_Current_Ref_Y_3;

		// Derived values, not stated, reloaded with savestate
		public ushort[] ppu_ROT_OBJ_X = new ushort[128];
		public ushort[] ppu_ROT_OBJ_Y = new ushort[128];

		public ushort[] ppu_MOS_OBJ_X = new ushort[0x200];

		public ushort[] ppu_MOS_BG_Y = new ushort[0x200];

		public int[] BG_Scale_X = new int[4];
		public int[] BG_Scale_Y = new int[4];
		public int[] BG_Num_Tiles = new int[4];
		public int[] ppu_BG_Char_Base = new int[4];
		public int[] ppu_BG_Screen_Base = new int[4];

		public byte[] ppu_BG_Priority = new byte[4];
		public byte[] ppu_BG_Screen_Size = new byte[4];

		public bool[] ppu_BG_Mosaic = new bool[4];
		public bool[] ppu_BG_Pal_Size = new bool[4];
		public bool[] ppu_BG_Overflow = new bool[4];
		public bool[] ppu_WIN0_BG_En = new bool[4];
		public bool[] ppu_WIN1_BG_En = new bool[4];
		public bool[] ppu_OBJ_BG_En = new bool[4];
		public bool[] ppu_OUT_BG_En = new bool[4];

		public ulong ppu_F_Rot_A_2, ppu_F_Rot_B_2, ppu_F_Rot_C_2, ppu_F_Rot_D_2;
		public ulong ppu_F_Rot_A_3, ppu_F_Rot_B_3, ppu_F_Rot_C_3, ppu_F_Rot_D_3;

		public int ppu_SFX_mode, ppu_SFX_BRT_Num;
		public int ppu_SFX_Alpha_Num_1, ppu_SFX_Alpha_Num_2;

		public ushort ppu_OBJ_Mosaic_X, ppu_OBJ_Mosaic_Y;
		public ushort ppu_BG_Mosaic_X, ppu_BG_Mosaic_Y;

		public byte ppu_WIN0_Left, ppu_WIN1_Left, ppu_WIN0_Right, ppu_WIN1_Right;
		public byte ppu_WIN0_Top, ppu_WIN1_Top, ppu_WIN0_Bot, ppu_WIN1_Bot;

		public bool ppu_WIN0_OBJ_En, ppu_WIN1_OBJ_En, ppu_OUT_OBJ_En, ppu_OBJ_OBJ_En;
		public bool ppu_WIN0_Color_En, ppu_WIN1_Color_En, ppu_OUT_Color_En, ppu_OBJ_Color_En;

		public bool ppu_SFX_OBJ_Target_1, ppu_SFX_OBJ_Target_2;

		// volatile variables used every cycle in rendering, not stated
		public bool[] BG_Go = new bool[4];

		public bool[] BG_Go_Disp = new bool[4];

		public byte ppu_Read_Reg_8(uint addr)
		{
			byte ret = 0;

			switch (addr)
			{
				case 0x00: ret = (byte)(ppu_CTRL & 0xFF); break;
				case 0x01: ret = (byte)((ppu_CTRL & 0xFF00) >> 8); break;
				case 0x02: ret = (byte)(ppu_Green_Swap & 0xFF); break;
				case 0x03: ret = (byte)((ppu_Green_Swap & 0xFF00) >> 8); break;
				case 0x04: ret = ppu_STAT; break;
				case 0x05: ret = ppu_LYC; break;
				case 0x06: ret = ppu_LY; break;
				case 0x07: ret = 0; break;

				case 0x08: ret = (byte)(ppu_BG_CTRL[0] & 0xFF); break;
				case 0x09: ret = (byte)((ppu_BG_CTRL[0] & 0xFF00) >> 8); break;
				case 0x0A: ret = (byte)(ppu_BG_CTRL[1] & 0xFF); break;
				case 0x0B: ret = (byte)((ppu_BG_CTRL[1] & 0xFF00) >> 8); break;
				case 0x0C: ret = (byte)(ppu_BG_CTRL[2] & 0xFF); break;
				case 0x0D: ret = (byte)((ppu_BG_CTRL[2] & 0xFF00) >> 8); break;
				case 0x0E: ret = (byte)(ppu_BG_CTRL[3] & 0xFF); break;
				case 0x0F: ret = (byte)((ppu_BG_CTRL[3] & 0xFF00) >> 8); break;

				case 0x48: ret = (byte)(ppu_WIN_In & 0xFF); break;
				case 0x49: ret = (byte)((ppu_WIN_In & 0xFF00) >> 8); break;
				case 0x4A: ret = (byte)(ppu_WIN_Out & 0xFF); break;
				case 0x4B: ret = (byte)((ppu_WIN_Out & 0xFF00) >> 8); break;

				case 0x50: ret = (byte)(ppu_Special_FX & 0xFF); break;
				case 0x51: ret = (byte)((ppu_Special_FX & 0xFF00) >> 8); break;
				case 0x52: ret = (byte)(ppu_Alpha & 0xFF); break;
				case 0x53: ret = (byte)((ppu_Alpha & 0xFF00) >> 8); break;

				default: ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		public ushort ppu_Read_Reg_16(uint addr)
		{
			ushort ret = 0;

			switch (addr)
			{
				case 0x00: ret = ppu_CTRL; break;
				case 0x02: ret = ppu_Green_Swap; break;
				case 0x04: ret = (ushort)((ppu_LYC << 8) | ppu_STAT); break;
				case 0x06: ret = ppu_LY; break;

				case 0x08: ret = ppu_BG_CTRL[0]; break;
				case 0x0A: ret = ppu_BG_CTRL[1]; break;
				case 0x0C: ret = ppu_BG_CTRL[2]; break;
				case 0x0E: ret = ppu_BG_CTRL[3]; break;

				case 0x48: ret = ppu_WIN_In; break;
				case 0x4A: ret = ppu_WIN_Out; break;

				case 0x50: ret = ppu_Special_FX; break;
				case 0x52: ret = ppu_Alpha; break;

				default: ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		public uint ppu_Read_Reg_32(uint addr)
		{
			uint ret = 0;

			switch (addr)
			{
				case 0x00: ret = (uint)((ppu_Green_Swap << 16) | ppu_CTRL); break;
				case 0x04: ret = (uint)((ppu_LY << 16) | (ppu_LYC << 8) | ppu_STAT); break;

				case 0x08: ret = (uint)((ppu_BG_CTRL[1] << 16) | ppu_BG_CTRL[0]); break;
				case 0x0C: ret = (uint)((ppu_BG_CTRL[3] << 16) | ppu_BG_CTRL[2]); break;

				case 0x48: ret = (uint)((ppu_WIN_Out << 16) | ppu_WIN_In); break;

				case 0x50: ret = (uint)((ppu_Alpha << 16) | ppu_Special_FX); break;

				default: ret = cpu_Last_Bus_Value; break;
			}

			return ret;
		}

		public void ppu_Write_Reg_8(uint addr, byte value)
		{
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write((ushort)((ppu_CTRL & 0xFF00) | value)); break;
				case 0x01: ppu_CTRL_Write((ushort)((ppu_CTRL & 0x00FF) | (value << 8))); break;
				case 0x02: ppu_Green_Swap = (ushort)((ppu_Green_Swap & 0xFF00) | value); ppu_Do_Green_Swap = (ppu_Green_Swap & 1) == 1; break;
				case 0x03: ppu_Green_Swap = (ushort)((ppu_Green_Swap & 0x00FF) | (value << 8)); break;
				case 0x04: ppu_STAT_Write(value); break;
				case 0x05: ppu_Update_LYC(value); break;
				case 0x06: // No Effect on LY
				case 0x07: // No Effect on LY
				case 0x08: ppu_BG_CTRL[0] = (ushort)((ppu_BG_CTRL[0] & 0xDF00) | value); ppu_BG_CTRL_Write(0); break;
				case 0x09: ppu_BG_CTRL[0] = (ushort)((ppu_BG_CTRL[0] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(0); break;
				case 0x0A: ppu_BG_CTRL[1] = (ushort)((ppu_BG_CTRL[1] & 0xDF00) | value); ppu_BG_CTRL_Write(1); break;
				case 0x0B: ppu_BG_CTRL[1] = (ushort)((ppu_BG_CTRL[1] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = (ushort)((ppu_BG_CTRL[2] & 0xFF00) | value); ppu_BG_CTRL_Write(2); break;
				case 0x0D: ppu_BG_CTRL[2] = (ushort)((ppu_BG_CTRL[2] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(2); break;
				case 0x0E: ppu_BG_CTRL[3] = (ushort)((ppu_BG_CTRL[3] & 0xFF00) | value); ppu_BG_CTRL_Write(3); break;
				case 0x0F: ppu_BG_CTRL[3] = (ushort)((ppu_BG_CTRL[3] & 0x00FF) | (value << 8)); ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X[0] = (ushort)((ppu_BG_X[0] & 0xFF00) | value); break;
				case 0x11: ppu_BG_X[0] = (ushort)((ppu_BG_X[0] & 0x00FF) | (value << 8)); break;
				case 0x12: ppu_BG_Y[0] = (ushort)((ppu_BG_Y[0] & 0xFF00) | value); break;
				case 0x13: ppu_BG_Y[0] = (ushort)((ppu_BG_Y[0] & 0x00FF) | (value << 8)); break;
				case 0x14: ppu_BG_X[1] = (ushort)((ppu_BG_X[1] & 0xFF00) | value); break;
				case 0x15: ppu_BG_X[1] = (ushort)((ppu_BG_X[1] & 0x00FF) | (value << 8)); break;
				case 0x16: ppu_BG_Y[1] = (ushort)((ppu_BG_Y[1] & 0xFF00) | value); break;
				case 0x17: ppu_BG_Y[1] = (ushort)((ppu_BG_Y[1] & 0x00FF) | (value << 8)); break;
				case 0x18: ppu_BG_X[2] = (ushort)((ppu_BG_X[2] & 0xFF00) | value); break;
				case 0x19: ppu_BG_X[2] = (ushort)((ppu_BG_X[2] & 0x00FF) | (value << 8)); break;
				case 0x1A: ppu_BG_Y[2] = (ushort)((ppu_BG_Y[2] & 0xFF00) | value); break;
				case 0x1B: ppu_BG_Y[2] = (ushort)((ppu_BG_Y[2] & 0x00FF) | (value << 8)); break;
				case 0x1C: ppu_BG_X[3] = (ushort)((ppu_BG_X[3] & 0xFF00) | value); break;
				case 0x1D: ppu_BG_X[3] = (ushort)((ppu_BG_X[3] & 0x00FF) | (value << 8)); break;
				case 0x1E: ppu_BG_Y[3] = (ushort)((ppu_BG_Y[3] & 0xFF00) | value); break;
				case 0x1F: ppu_BG_Y[3] = (ushort)((ppu_BG_Y[3] & 0x00FF) | (value << 8)); break;

				case 0x20: ppu_BG_Rot_A[2] = (ushort)((ppu_BG_Rot_A[2] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x21: ppu_BG_Rot_A[2] = (ushort)((ppu_BG_Rot_A[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x22: ppu_BG_Rot_B[2] = (ushort)((ppu_BG_Rot_B[2] & 0xFF00) | value); break;
				case 0x23: ppu_BG_Rot_B[2] = (ushort)((ppu_BG_Rot_B[2] & 0x00FF) | (value << 8)); break;
				case 0x24: ppu_BG_Rot_C[2] = (ushort)((ppu_BG_Rot_C[2] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x25: ppu_BG_Rot_C[2] = (ushort)((ppu_BG_Rot_C[2] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x26: ppu_BG_Rot_D[2] = (ushort)((ppu_BG_Rot_D[2] & 0xFF00) | value); break;
				case 0x27: ppu_BG_Rot_D[2] = (ushort)((ppu_BG_Rot_D[2] & 0x00FF) | (value << 8)); break;
				case 0x28: ppu_BG_Ref_X[2] = (uint)((ppu_BG_Ref_X[2] & 0xFFFFFF00) | value); ppu_ROT_REF_X_Update(2); break;
				case 0x29: ppu_BG_Ref_X[2] = (uint)((ppu_BG_Ref_X[2] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_X_Update(2); break;
				case 0x2A: ppu_BG_Ref_X[2] = (uint)((ppu_BG_Ref_X[2] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_X_Update(2); break;
				case 0x2B: ppu_BG_Ref_X[2] = (uint)((ppu_BG_Ref_X[2] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_X_Update(2); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (uint)((ppu_BG_Ref_Y[2] & 0xFFFFFF00) | value); ppu_ROT_REF_LY_Update(2); break;
				case 0x2D: ppu_BG_Ref_Y[2] = (uint)((ppu_BG_Ref_Y[2] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_LY_Update(2); break;
				case 0x2E: ppu_BG_Ref_Y[2] = (uint)((ppu_BG_Ref_Y[2] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(2); break;
				case 0x2F: ppu_BG_Ref_Y[2] = (uint)((ppu_BG_Ref_Y[2] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = (ushort)((ppu_BG_Rot_A[3] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x31: ppu_BG_Rot_A[3] = (ushort)((ppu_BG_Rot_A[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x32: ppu_BG_Rot_B[3] = (ushort)((ppu_BG_Rot_B[3] & 0xFF00) | value); break;
				case 0x33: ppu_BG_Rot_B[3] = (ushort)((ppu_BG_Rot_B[3] & 0x00FF) | (value << 8)); break;
				case 0x34: ppu_BG_Rot_C[3] = (ushort)((ppu_BG_Rot_C[3] & 0xFF00) | value); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x35: ppu_BG_Rot_C[3] = (ushort)((ppu_BG_Rot_C[3] & 0x00FF) | (value << 8)); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x36: ppu_BG_Rot_D[3] = (ushort)((ppu_BG_Rot_D[3] & 0xFF00) | value); break;
				case 0x37: ppu_BG_Rot_D[3] = (ushort)((ppu_BG_Rot_D[3] & 0x00FF) | (value << 8)); break;
				case 0x38: ppu_BG_Ref_X[3] = (uint)((ppu_BG_Ref_X[3] & 0xFFFFFF00) | value); ppu_ROT_REF_X_Update(3); break;
				case 0x39: ppu_BG_Ref_X[3] = (uint)((ppu_BG_Ref_X[3] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_X_Update(3); break;
				case 0x3A: ppu_BG_Ref_X[3] = (uint)((ppu_BG_Ref_X[3] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_X_Update(3); break;
				case 0x3B: ppu_BG_Ref_X[3] = (uint)((ppu_BG_Ref_X[3] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_X_Update(3); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (uint)((ppu_BG_Ref_Y[3] & 0xFFFFFF00) | value); ppu_ROT_REF_LY_Update(3); break;
				case 0x3D: ppu_BG_Ref_Y[3] = (uint)((ppu_BG_Ref_Y[3] & 0xFFFF00FF) | (value << 8)); ppu_ROT_REF_LY_Update(3); break;
				case 0x3E: ppu_BG_Ref_Y[3] = (uint)((ppu_BG_Ref_Y[3] & 0xFF00FFFF) | (value << 16)); ppu_ROT_REF_LY_Update(3); break;
				case 0x3F: ppu_BG_Ref_Y[3] = (uint)((ppu_BG_Ref_Y[3] & 0x00FFFFFF) | ((value & 0xF) << 24)); ppu_ROT_REF_LY_Update(3); break;

				case 0x40: ppu_WIN_Hor_0 = (ushort)((ppu_WIN_Hor_0 & 0xFF00) | value); ppu_Calc_Win0(); break;
				case 0x41: ppu_WIN_Hor_0 = (ushort)((ppu_WIN_Hor_0 & 0x00FF) | (value << 8)); ppu_Calc_Win0(); break;
				case 0x42: ppu_WIN_Hor_1 = (ushort)((ppu_WIN_Hor_1 & 0xFF00) | value); ppu_Calc_Win1(); break;
				case 0x43: ppu_WIN_Hor_1 = (ushort)((ppu_WIN_Hor_1 & 0x00FF) | (value << 8)); ppu_Calc_Win1(); break;
				case 0x44: ppu_WIN_Vert_0 = (ushort)((ppu_WIN_Vert_0 & 0xFF00) | value); ppu_Calc_Win0(); break;
				case 0x45: ppu_WIN_Vert_0 = (ushort)((ppu_WIN_Vert_0 & 0x00FF) | (value << 8)); ppu_Calc_Win0(); break;
				case 0x46: ppu_WIN_Vert_1 = (ushort)((ppu_WIN_Vert_1 & 0xFF00) | value); ppu_Calc_Win1(); break;
				case 0x47: ppu_WIN_Vert_1 = (ushort)((ppu_WIN_Vert_1 & 0x00FF) | (value << 8)); ppu_Calc_Win1(); break;
				case 0x48: ppu_Update_Win_In((ushort)((ppu_WIN_In & 0xFF00) | value)); break;
				case 0x49: ppu_Update_Win_In((ushort)((ppu_WIN_In & 0x00FF) | (value << 8))); break;
				case 0x4A: ppu_Update_Win_Out((ushort)((ppu_WIN_Out & 0xFF00) | value)); break;
				case 0x4B: ppu_Update_Win_Out((ushort)((ppu_WIN_Out & 0x00FF) | (value << 8))); break;
				case 0x4C: ppu_Update_Mosaic((ushort)((ppu_Mosaic & 0xFF00) | value)); break;
				case 0x4D: ppu_Update_Mosaic((ushort)((ppu_Mosaic & 0x00FF) | (value << 8))); break;

				case 0x50: ppu_Update_Special_FX((ushort)((ppu_Special_FX & 0xFF00) | value)); break;
				case 0x51: ppu_Update_Special_FX((ushort)((ppu_Special_FX & 0x00FF) | (value << 8))); break;
				case 0x52: ppu_Update_Alpha((ushort)((ppu_Alpha & 0xFF00) | value)); break;
				case 0x53: ppu_Update_Alpha((ushort)((ppu_Alpha & 0x00FF) | (value << 8))); break;
				case 0x54: ppu_Update_Bright((ushort)((ppu_Bright & 0xFF00) | value)); break;
				case 0x55: ppu_Update_Bright((ushort)((ppu_Bright & 0x00FF) | (value << 8))); break;
			}
		}

		public void ppu_Write_Reg_16(uint addr, ushort value)
		{
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write(value); break;
				case 0x02: ppu_Green_Swap = value; ppu_Do_Green_Swap = (ppu_Green_Swap & 1) == 1; break;
				case 0x04: ppu_STAT_Write((byte)value); ppu_Update_LYC((byte)(value >> 8)); break;
				case 0x06: // No Effect on LY
				case 0x08: ppu_BG_CTRL[0] = (ushort)(value & 0xDFFF); ppu_BG_CTRL_Write(0); break;
				case 0x0A: ppu_BG_CTRL[1] = (ushort)(value & 0xDFFF); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = value; ppu_BG_CTRL_Write(2); break;
				case 0x0E: ppu_BG_CTRL[3] = value; ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X[0] = value; break;
				case 0x12: ppu_BG_Y[0] = value; break;
				case 0x14: ppu_BG_X[1] = value; break;
				case 0x16: ppu_BG_Y[1] = value; break;
				case 0x18: ppu_BG_X[2] = value; break;
				case 0x1A: ppu_BG_Y[2] = value; break;
				case 0x1C: ppu_BG_X[3] = value; break;
				case 0x1E: ppu_BG_Y[3] = value; break;

				case 0x20: ppu_BG_Rot_A[2] = value; ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x22: ppu_BG_Rot_B[2] = value; break;
				case 0x24: ppu_BG_Rot_C[2] = value; ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x26: ppu_BG_Rot_D[2] = value; break;
				case 0x28: ppu_BG_Ref_X[2] = (uint)((ppu_BG_Ref_X[2] & 0xFFFF0000) | value); ppu_ROT_REF_X_Update(2); break;
				case 0x2A: ppu_BG_Ref_X[2] = (uint)((ppu_BG_Ref_X[2] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_X_Update(2); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (uint)((ppu_BG_Ref_Y[2] & 0xFFFF0000) | value); ppu_ROT_REF_LY_Update(2); break;
				case 0x2E: ppu_BG_Ref_Y[2] = (uint)((ppu_BG_Ref_Y[2] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = value; ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x32: ppu_BG_Rot_B[3] = value; break;
				case 0x34: ppu_BG_Rot_C[3] = value; ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x36: ppu_BG_Rot_D[3] = value; break;
				case 0x38: ppu_BG_Ref_X[3] = (uint)((ppu_BG_Ref_X[3] & 0xFFFF0000) | value); ppu_ROT_REF_X_Update(3); break;
				case 0x3A: ppu_BG_Ref_X[3] = (uint)((ppu_BG_Ref_X[3] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_X_Update(3); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (uint)((ppu_BG_Ref_Y[3] & 0xFFFF0000) | value); ppu_ROT_REF_LY_Update(3); break;
				case 0x3E: ppu_BG_Ref_Y[3] = (uint)((ppu_BG_Ref_Y[3] & 0x0000FFFF) | ((value & 0xFFF) << 16)); ppu_ROT_REF_LY_Update(3); break;

				case 0x40: ppu_WIN_Hor_0 = value; ppu_Calc_Win0(); break;
				case 0x42: ppu_WIN_Hor_1 = value; ppu_Calc_Win1(); break;
				case 0x44: ppu_WIN_Vert_0 = value; ppu_Calc_Win0(); break;
				case 0x46: ppu_WIN_Vert_1 = value; ppu_Calc_Win1(); break;
				case 0x48: ppu_Update_Win_In(value); break;
				case 0x4A: ppu_Update_Win_Out(value); break;
				case 0x4C: ppu_Update_Mosaic(value); break;

				case 0x50: ppu_Update_Special_FX(value); break;
				case 0x52: ppu_Update_Alpha(value); break;
				case 0x54: ppu_Update_Bright(value); break;
			}
		}

		public void ppu_Write_Reg_32(uint addr, uint value)
		{
			switch (addr)
			{
				case 0x00: ppu_CTRL_Write((ushort)(value & 0xFFFF));
						   ppu_Green_Swap = (ushort)((value >> 16) & 0xFFFF); ppu_Do_Green_Swap = (ppu_Green_Swap & 1) == 1; break;
				case 0x04: ppu_STAT_Write((byte)value); ppu_Update_LYC((byte)(value >> 8)); break;
						   /* no effect on LY*/
				case 0x08: ppu_BG_CTRL[0] = (ushort)(value & 0xDFFF); ppu_BG_CTRL_Write(0);
						   ppu_BG_CTRL[1] = (ushort)((value >> 16) & 0xDFFF); ppu_BG_CTRL_Write(1); break;
				case 0x0C: ppu_BG_CTRL[2] = (ushort)(value & 0xFFFF); ppu_BG_CTRL_Write(2);
						   ppu_BG_CTRL[3] = (ushort)((value >> 16) & 0xFFFF); ppu_BG_CTRL_Write(3); break;

				case 0x10: ppu_BG_X[0] = (ushort)(value & 0xFFFF);
						   ppu_BG_Y[0] = (ushort)((value >> 16) & 0xFFFF); break;
				case 0x14: ppu_BG_X[1] = (ushort)(value & 0xFFFF);
						   ppu_BG_Y[1] = (ushort)((value >> 16) & 0xFFFF); break;
				case 0x18: ppu_BG_X[2] = (ushort)(value & 0xFFFF);
						   ppu_BG_Y[2] = (ushort)((value >> 16) & 0xFFFF); break;
				case 0x1C: ppu_BG_X[3] = (ushort)(value & 0xFFFF);
						   ppu_BG_Y[3] = (ushort)((value >> 16) & 0xFFFF); break;

				case 0x20: ppu_BG_Rot_A[2] = (ushort)(value & 0xFFFF);
						   ppu_BG_Rot_B[2] = (ushort)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x24: ppu_BG_Rot_C[2] = (ushort)(value & 0xFFFF);
						   ppu_BG_Rot_D[2] = (ushort)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(2); break;
				case 0x28: ppu_BG_Ref_X[2] = (value & 0xFFFFFFF); ppu_ROT_REF_X_Update(2); break;
				case 0x2C: ppu_BG_Ref_Y[2] = (value & 0xFFFFFFF); ppu_ROT_REF_LY_Update(2); break;

				case 0x30: ppu_BG_Rot_A[3] = (ushort)(value & 0xFFFF);
						   ppu_BG_Rot_B[3] = (ushort)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x34: ppu_BG_Rot_C[3] = (ushort)(value & 0xFFFF);
						   ppu_BG_Rot_D[3] = (ushort)((value >> 16) & 0xFFFF); ppu_Convert_Rotation_to_ulong_AC(3); break;
				case 0x38: ppu_BG_Ref_X[3] = (value & 0xFFFFFFF); ppu_ROT_REF_X_Update(3); break;
				case 0x3C: ppu_BG_Ref_Y[3] = (value & 0xFFFFFFF); ppu_ROT_REF_LY_Update(3); break;

				case 0x40: ppu_WIN_Hor_0 = (ushort)(value & 0xFFFF);
						   ppu_WIN_Hor_1 = (ushort)((value >> 16) & 0xFFFF); ppu_Calc_Win0(); ppu_Calc_Win1(); break;
				case 0x44: ppu_WIN_Vert_0 = (ushort)(value & 0xFFFF);
						   ppu_WIN_Vert_1 = (ushort)((value >> 16) & 0xFFFF); ppu_Calc_Win0(); ppu_Calc_Win1(); break;
				case 0x48: ppu_Update_Win_In((ushort)(value & 0xFFFF));
						   ppu_Update_Win_Out((ushort)((value >> 16) & 0xFFFF)); break;
				case 0x4C: ppu_Update_Mosaic((ushort)(value & 0xFFFF)); 
						   /* no effect*/ break;

				case 0x50: ppu_Update_Special_FX((ushort)(value & 0xFFFF));
						   ppu_Update_Alpha((ushort)((value >> 16) & 0xFFFF)); break;
				case 0x54: ppu_Update_Bright((ushort)(value & 0xFFFF));
						   /* no effect*/ break;
			}
		}

		public void ppu_STAT_Write(byte value)
		{
			ppu_STAT &= 0xC7;
			ppu_STAT |= (byte)(value & 0x38);
		}

		public void ppu_Update_LYC(byte value)
		{
			byte old_LYC = ppu_LYC;

			ppu_LYC = value;

			// check if writing a matching LYC value will trigger an interrupt
			if ((old_LYC != ppu_LY) && (ppu_LY == ppu_LYC))
			{
				ppu_STAT |= 0x04;
				if (ppu_LYC_IRQ_cd == 0)
				{
					ppu_LYC_IRQ_cd = 4;
					ppu_Delays = true;
					delays_to_process = true;
				}
			}

			// does changing to non-matching value clear the flag? for now assume yes
			// also assume it doesn't disable any pending interrupts
			if ((old_LYC == ppu_LY) && (ppu_LY != ppu_LYC))
			{
				ppu_STAT &= 0xFB;
			}
		}

		public void ppu_CTRL_Write(ushort value)
		{
			ppu_BG_Mode = value & 7;
			ppu_Display_Frame = (value >> 4) & 1;

			ppu_HBL_Free = (value & 0x20) == 0x20;
			ppu_OBJ_Dim = (value & 0x40) == 0x40;

			ppu_WIN0_On = (value & 0x2000) == 0x2000;
			ppu_WIN1_On = (value & 0x4000) == 0x4000;
			ppu_OBJ_WIN = (value & 0x8000) == 0x8000;

			// BG's don't turn on immediately, but do turn off immediately
			for (int i = 0; i < 4; i++)
			{
				if ((value & (0x100 << i)) == 0)
				{
					ppu_BG_On[i] = false;
					ppu_BG_On_Update_Time[i] = 0;
				}
				else if (ppu_BG_On_Update_Time[i] == 0)
				{
					// if the change happens before around cycle 40, then the delay is only 2 scanlines not 3
					// see spyro season of ice, talking to the first fairy
					if (ppu_Cycle < 40)
					{
						ppu_BG_On_Update_Time[i] = 2;
					}
					else
					{
						ppu_BG_On_Update_Time[i] = 3;
					}
				}
			}

			// sprites require one scanline to turn on
			if ((value & 0x1000) == 0)
			{
				ppu_OBJ_On = false;
				ppu_OBJ_On_Time = 0;
			}
			else
			{
				ppu_OBJ_On_Time = 2;
			}

			// forced blank timing is the same as BG enable
			if ((value & 0x80) == 0x80)
			{
				ppu_Forced_Blank = true;
				ppu_Forced_Blank_Time = 0;
			}
			else
			{
				if (ppu_Cycle < 40)
				{
					ppu_Forced_Blank_Time = 2;
				}
				else
				{
					ppu_Forced_Blank_Time = 3;
				}
			}

			ppu_CTRL = value;

			if (ppu_HBL_Free) { ppu_Sprite_Eval_Time = 964; }
			else { ppu_Sprite_Eval_Time = 1232; }

			ppu_Any_Window_On = ppu_WIN0_On | ppu_WIN1_On | ppu_OBJ_WIN;

			// ensure BG controls match new mode
			ppu_BG_CTRL_Write(0);
			ppu_BG_CTRL_Write(1);
			ppu_BG_CTRL_Write(2);
			ppu_BG_CTRL_Write(3);

			if (ppu_Forced_Blank)
			{
				ppu_VRAM_Access = false;
				ppu_PALRAM_Access = false;
				ppu_OAM_Access = false;
			}

			//Console.WriteLine(value + " Mode: " + ppu_BG_Mode + " w0: " + ppu_WIN0_On + " w1: " + ppu_WIN0_On + " " + ppu_LY + " " + ppu_Cycle + " " + CycleCount);
		}

		public void ppu_Calc_Win0()
		{
			ppu_WIN0_Right = (byte)(ppu_WIN_Hor_0 & 0xFF);
			ppu_WIN0_Left = (byte)((ppu_WIN_Hor_0 >> 8) & 0xFF);

			ppu_WIN0_Bot = (byte)(ppu_WIN_Vert_0 & 0xFF);
			ppu_WIN0_Top = (byte)((ppu_WIN_Vert_0 >> 8) & 0xFF);

			//Console.WriteLine("W0: " + ppu_WIN0_Top + " " + ppu_WIN0_Bot + " " + ppu_LY + " " + ppu_Cycle);
		}

		public void ppu_Calc_Win1()
		{
			ppu_WIN1_Right = (byte)(ppu_WIN_Hor_1 & 0xFF);
			ppu_WIN1_Left = (byte)((ppu_WIN_Hor_1 >> 8) & 0xFF);

			ppu_WIN1_Bot = (byte)(ppu_WIN_Vert_1 & 0xFF);
			ppu_WIN1_Top = (byte)((ppu_WIN_Vert_1 >> 8) & 0xFF);

			//Console.WriteLine("W1: " + ppu_WIN1_Top + " " + ppu_WIN1_Bot + " " + ppu_LY + " " + ppu_Cycle);
		}

		public void ppu_Update_Win_In(ushort value)
		{
			ppu_WIN_In = (ushort)(value & 0x3F3F);
			
			ppu_WIN0_BG_En[0] = (ppu_WIN_In & 0x1) == 0x1;
			ppu_WIN0_BG_En[1] = (ppu_WIN_In & 0x2) == 0x2;
			ppu_WIN0_BG_En[2] = (ppu_WIN_In & 0x4) == 0x4;
			ppu_WIN0_BG_En[3] = (ppu_WIN_In & 0x8) == 0x8;
			ppu_WIN0_OBJ_En = (ppu_WIN_In & 0x10) == 0x10;
			ppu_WIN0_Color_En = (ppu_WIN_In & 0x20) == 0x20;

			ppu_WIN1_BG_En[0] = (ppu_WIN_In & 0x100) == 0x100;
			ppu_WIN1_BG_En[1] = (ppu_WIN_In & 0x200) == 0x200;
			ppu_WIN1_BG_En[2] = (ppu_WIN_In & 0x400) == 0x400;
			ppu_WIN1_BG_En[3] = (ppu_WIN_In & 0x800) == 0x800;
			ppu_WIN1_OBJ_En = (ppu_WIN_In & 0x1000) == 0x1000;
			ppu_WIN1_Color_En = (ppu_WIN_In & 0x2000) == 0x2000;
		}

		public void ppu_Update_Win_Out(ushort value)
		{
			ppu_WIN_Out = (ushort)(value & 0x3F3F);

			ppu_OUT_BG_En[0] = (ppu_WIN_Out & 0x1) == 0x1;
			ppu_OUT_BG_En[1] = (ppu_WIN_Out & 0x2) == 0x2;
			ppu_OUT_BG_En[2] = (ppu_WIN_Out & 0x4) == 0x4;
			ppu_OUT_BG_En[3] = (ppu_WIN_Out & 0x8) == 0x8;
			ppu_OUT_OBJ_En = (ppu_WIN_Out & 0x10) == 0x10;
			ppu_OUT_Color_En = (ppu_WIN_Out & 0x20) == 0x20;

			ppu_OBJ_BG_En[0] = (ppu_WIN_Out & 0x100) == 0x100;
			ppu_OBJ_BG_En[1] = (ppu_WIN_Out & 0x200) == 0x200;
			ppu_OBJ_BG_En[2] = (ppu_WIN_Out & 0x400) == 0x400;
			ppu_OBJ_BG_En[3] = (ppu_WIN_Out & 0x800) == 0x800;
			ppu_OBJ_OBJ_En = (ppu_WIN_Out & 0x1000) == 0x1000;
			ppu_OBJ_Color_En = (ppu_WIN_Out & 0x2000) == 0x2000;
		}

		public void ppu_ROT_REF_X_Update(int layer)
		{
			if (ppu_BG_On[layer])
			{
				ppu_BG_Ref_X_Change[layer] = true;
			}
		}

		public void ppu_ROT_REF_LY_Update(int layer)
		{
			if (ppu_BG_On[layer])
			{
				ppu_BG_Ref_LY_Change[layer] = true;
			}
		}

		public void ppu_Update_Special_FX(ushort value)
		{
			ppu_Special_FX = (ushort)(value & 0x3FFF);

			ppu_SFX_mode = ((ppu_Special_FX >> 6) & 3);

			ppu_SFX_OBJ_Target_1 = ((ppu_Special_FX >> 4) & 0x1) != 0;

			ppu_SFX_OBJ_Target_2 = ((ppu_Special_FX >> 12) & 0x1) != 0;
		}

		public void ppu_Update_Alpha(ushort value)
		{
			ppu_Alpha = (ushort)(value & 0x1F1F);

			if ((ppu_Alpha & 0x10) == 0x10)
			{
				ppu_SFX_Alpha_Num_1 = 16;
			}
			else
			{
				ppu_SFX_Alpha_Num_1 = ppu_Alpha & 0xF;
			}

			if ((ppu_Alpha & 0x1000) == 0x1000)
			{
				ppu_SFX_Alpha_Num_2 = 16;
			}
			else
			{
				ppu_SFX_Alpha_Num_2 = (ppu_Alpha >> 8) & 0xF;
			}
		}

		public void ppu_Update_Bright(ushort value)
		{
			ppu_Bright = value;

			if ((ppu_Bright & 0x10) == 0x10)
			{
				ppu_SFX_BRT_Num = 16;
			}
			else
			{
				ppu_SFX_BRT_Num = ppu_Bright & 0xF;
			}
		}

		public void ppu_Update_Mosaic(ushort value)
		{
			ppu_Mosaic = value;

			int j;

			ushort mosaic_x, mosaic_y;

			ppu_BG_Mosaic_X = (ushort)((ppu_Mosaic & 0xF) + 1);
			ppu_BG_Mosaic_Y = (ushort)(((ppu_Mosaic >> 4) & 0xF) + 1);
			ppu_OBJ_Mosaic_X = (ushort)(((ppu_Mosaic >> 8) & 0xF) + 1);
			ppu_OBJ_Mosaic_Y = (ushort)(((ppu_Mosaic >> 12) & 0xF) + 1);

			mosaic_y = 0;
			j = 0;

			for (int i = 0; i < 0x200; i++)
			{
				ppu_MOS_BG_Y[i] = mosaic_y;

				j++;

				if (j == ppu_BG_Mosaic_Y)
				{
					mosaic_y += ppu_BG_Mosaic_Y;
					j = 0;
				}
			}

			mosaic_x = 0;
			j = 0;

			for (int i = 0; i < 0x200; i++)
			{
				ppu_MOS_OBJ_X[i] = mosaic_x;

				j++;

				if (j == ppu_OBJ_Mosaic_X)
				{
					mosaic_x += ppu_OBJ_Mosaic_X;
					j = 0;
				}
			}
		}

		public void ppu_BG_CTRL_Write(int lyr)
		{
			ppu_BG_Priority[lyr] = (byte)(ppu_BG_CTRL[lyr] & 3);
			ppu_BG_Char_Base[lyr] = ((ppu_BG_CTRL[lyr] >> 2) & 3) * 16 * 1024;
			ppu_BG_Mosaic[lyr] = (ppu_BG_CTRL[lyr] & 0x40) == 0x40;
			ppu_BG_Pal_Size[lyr] = (ppu_BG_CTRL[lyr] & 0x80) == 0x80;
			ppu_BG_Screen_Base[lyr] = ((ppu_BG_CTRL[lyr] >> 8) & 0x1F) * 2 * 1024;
			ppu_BG_Overflow[lyr] = (ppu_BG_CTRL[lyr] & 0x2000) == 0x2000;
			ppu_BG_Screen_Size[lyr] = (byte)((ppu_BG_CTRL[lyr] >> 14) & 3);

			if (ppu_BG_Mode == 0)
			{
				switch (ppu_BG_Screen_Size[lyr])
				{
					case 0:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 256;
						BG_Num_Tiles[lyr] = 32;
						break;

					case 1:
						BG_Scale_X[lyr] = 512;
						BG_Scale_Y[lyr] = 256;
						BG_Num_Tiles[lyr] = 32;
						break;

					case 2:
						BG_Scale_X[lyr] = 256;
						BG_Scale_Y[lyr] = 512;
						BG_Num_Tiles[lyr] = 32;
						break;

					case 3:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 512;
						BG_Num_Tiles[lyr] = 32;
						break;
				}
			}
			else if ((lyr < 2) && (ppu_BG_Mode == 1))
			{
				switch (ppu_BG_Screen_Size[lyr])
				{
					case 0:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 256;
						BG_Num_Tiles[lyr] = 32;
						break;

					case 1:
						BG_Scale_X[lyr] = 512;
						BG_Scale_Y[lyr] = 256;
						BG_Num_Tiles[lyr] = 32;
						break;

					case 2:
						BG_Scale_X[lyr] = 256;
						BG_Scale_Y[lyr] = 512;
						BG_Num_Tiles[lyr] = 32;
						break;

					case 3:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 512;
						BG_Num_Tiles[lyr] = 32;
						break;
				}
			}
			else
			{
				switch (ppu_BG_Screen_Size[lyr])
				{
					case 0:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 128;
						BG_Num_Tiles[lyr] = 16;
						break;

					case 1:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 256;
						BG_Num_Tiles[lyr] = 32;
						break;

					case 2:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 512;
						BG_Num_Tiles[lyr] = 64;
						break;

					case 3:
						BG_Scale_X[lyr] = BG_Scale_Y[lyr] = 1024;
						BG_Num_Tiles[lyr] = 128;
						break;
				}
			}	
		}

		public void ppu_Tick()
		{
			// start the next scanline
			if (ppu_Cycle == 1232)
			{
				// scanline callback
				if ((ppu_LY + 1) == _scanlineCallbackLine)
				{
					if (_scanlineCallback != null)
					{
						_scanlineCallback();
					}
				}

				ppu_Cycle = 0;
				ppu_Display_Cycle = 0;
				ppu_LY += 1;

				if (ppu_LY == 228)
				{
					// display starts occuring at scanline 0
					ppu_In_VBlank = false;
					ppu_LY = 0;

					// reset rotation and scaling offsetreference Y value
					ppu_ROT_REF_LY[2] = 0;
					ppu_ROT_REF_LY[3] = 0;
				}

				// exit HBlank
				ppu_STAT &= 0xFD;

				// clear the LYC flag bit
				ppu_STAT &= 0xFB;

				// Check LY = LYC in 1 cycle
				// video capture DMA in 6 cycles
				ppu_LYC_Vid_Check_cd = 6;
				ppu_Delays = true;
				delays_to_process = true;

				if (ppu_LY == 160)
				{
					// vblank flag turns on on scanline 160
					VBlank_Rise = true;
					ppu_In_VBlank = true;
					ppu_STAT |= 1;

					// trigger VBL IRQ
					ppu_VBL_IRQ_cd = 4;
					ppu_Delays = true;
					delays_to_process = true;

					On_VBlank();
				}
				else if (ppu_LY == 227)
				{
					// vblank flag turns off on scanline 227
					ppu_STAT &= 0xFE;
				}

				// reset sprite evaluation  in 40 cycles
				if ((ppu_LY == 227) || (ppu_LY <= 159))
				{
					ppu_Sprite_Delays = true;
					delays_to_process = true;
					ppu_Sprite_cd = 40;
				}

				// update BG toggles
				for (int i = 0; i < 4; i++)
				{
					if (ppu_BG_On_Update_Time[i] > 0)
					{
						ppu_BG_On_Update_Time[i]--;

						if (ppu_BG_On_Update_Time[i] == 0)
						{
							ppu_BG_On[i] = true;
						}
					}
				}

				if (ppu_OBJ_On_Time > 0)
				{
					ppu_OBJ_On_Time--;

					if (ppu_OBJ_On_Time == 0)
					{
						ppu_OBJ_On = true;
					}
				}

				if (ppu_Forced_Blank_Time > 0)
				{
					ppu_Forced_Blank_Time--;

					if (ppu_Forced_Blank_Time == 0)
					{
						ppu_Forced_Blank = false;
					}
				}

				// check Y range for windows
				// checks happen even if window is disabled
				if (ppu_LY == ppu_WIN0_Top) { ppu_WIN0_Active = true; }
				if (ppu_LY == ppu_WIN0_Bot) { ppu_WIN0_Active = false; }

				if (ppu_LY == ppu_WIN1_Top) { ppu_WIN1_Active = true; }
				if (ppu_LY == ppu_WIN1_Bot) { ppu_WIN1_Active = false; }			
			}
			else if (ppu_Cycle == 1007)
			{
				// Enter HBlank
				ppu_STAT |= 2;

				ppu_HBL_IRQ_cd = 4;
				ppu_Delays = true;
				delays_to_process = true;	
			}

			if (!ppu_Forced_Blank)
			{
				if (ppu_In_VBlank)
				{
					if (ppu_LY == 227)					
					{
						ppu_VRAM_High_Access = false;
						ppu_OAM_Access = false;

						if (!ppu_Sprite_Eval_Finished && (ppu_Sprite_Render_Cycle < ppu_Sprite_Eval_Time))
						{
							if (((ppu_Cycle & 1) == 1) && (ppu_Cycle >= 40)) { ppu_Render_Sprites(); }
						}
					}
				}
				else
				{
					ppu_VRAM_High_Access = false;
					ppu_VRAM_Access = false;
					ppu_PALRAM_Access = false;
					ppu_OAM_Access = false;

					if (!ppu_Sprite_Eval_Finished && (ppu_Sprite_Render_Cycle < ppu_Sprite_Eval_Time))
					{
						if ((ppu_Cycle & 1) == 1) { ppu_Render_Sprites(); }
					}

					if (!ppu_Rendering_Complete)
					{						
						ppu_Render();
					}
				}
			}

			ppu_Cycle += 1;
		}

		public void ppu_Render()
		{
			ulong cur_x;
			ulong sol_x, sol_y;

			uint R, G, B;
			uint R2, G2, B2;

			int cur_layer_priority;
			int second_layer_priority;

			int temp_addr;
			int temp_color;

			int Screen_Offset;

			int VRAM_ofst_X, VRAM_ofst_Y;

			int cur_BG_layer;
			int second_BG_layer;

			bool Is_Outside;
			bool OBJ_Has_Pixel;
			bool OBJ_Go;
			bool Color_FX_Go;

			uint spr_pixel = 0;
			int spr_priority = 0;
			bool spr_semi_transparent = false;

			BG_Go[0] = ppu_BG_On[0];
			BG_Go[1] = ppu_BG_On[1];
			BG_Go[2] = ppu_BG_On[2];
			BG_Go[3] = ppu_BG_On[3];

			if (!ppu_PAL_Rendering_Complete)
			{
				// move pixel data up up the pipeline
				if (((ppu_Cycle & 3) == 0) && (ppu_Cycle >= 36))
				{
					switch (ppu_BG_Mode)
					{
						case 0:
							for (int c0 = 0; c0 < 4; c0++)
							{
								if (ppu_BG_On[c0])
								{
									temp_color = 0;

									ppu_Pixel_Color_M[c0] = ppu_Pixel_Color_1[c0];
									ppu_BG_Has_Pixel_M[c0] = ppu_BG_Has_Pixel_1[c0];

									if (ppu_BG_Pal_Size[c0])
									{
										if ((ppu_Scroll_Cycle[c0] & 7) == 0)
										{
											if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
											{
												temp_color = ppu_Pixel_Color[c0] & 0xFF;
											}
											else
											{
												temp_color = (ppu_Pixel_Color[c0] >> 8) & 0xFF;
											}
										}
										else
										{
											if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
											{
												temp_color = (ppu_Pixel_Color[c0] >> 8) & 0xFF;
											}
											else
											{
												temp_color = ppu_Pixel_Color[c0] & 0xFF;
											}
										}

										temp_color <<= 1;

										ppu_Pixel_Color_1[c0] = temp_color;
										ppu_BG_Has_Pixel_1[c0] = temp_color != 0;
									}
									else
									{
										if ((ppu_Scroll_Cycle[c0] & 0xF) == 0)
										{
											if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
											{
												temp_color = ((ppu_Pixel_Color[c0] >> 8) & 0xF);
											}
											else
											{
												temp_color = ((ppu_Pixel_Color[c0] >> 4) & 0xF);
											}
										}
										else if ((ppu_Scroll_Cycle[c0] & 0xF) == 4)
										{
											if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
											{
												temp_color = ((ppu_Pixel_Color[c0] >> 12) & 0xF);
											}
											else
											{
												temp_color = (ppu_Pixel_Color[c0] & 0xF);
											}
										}
										else if ((ppu_Scroll_Cycle[c0] & 0xF) == 8)
										{
											if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
											{
												temp_color = (ppu_Pixel_Color[c0] & 0xF);
											}
											else
											{
												temp_color = ((ppu_Pixel_Color[c0] >> 12) & 0xF);
											}
										}
										else if ((ppu_Scroll_Cycle[c0] & 0xF) == 12)
										{
											if ((ppu_BG_Effect_Byte[c0] & 0x4) == 0x0)
											{
												temp_color = ((ppu_Pixel_Color[c0] >> 4) & 0xF);
											}
											else
											{
												temp_color = ((ppu_Pixel_Color[c0] >> 8) & 0xF);
											}
										}

										temp_color |= (ppu_BG_Effect_Byte[c0] & 0xF0);

										temp_color <<= 1;

										ppu_Pixel_Color_1[c0] = temp_color;
										ppu_BG_Has_Pixel_1[c0] = ((temp_color & 0x1F) != 0);
									}
								}
							}
							break;

						case 1:
							for (int c1 = 0; c1 < 2; c1++)
							{	
								if (ppu_BG_On[c1])
								{
									temp_color = 0;

									ppu_Pixel_Color_M[c1] = ppu_Pixel_Color_1[c1];
									ppu_BG_Has_Pixel_M[c1] = ppu_BG_Has_Pixel_1[c1];

									if (ppu_BG_Pal_Size[c1])
									{
										if ((ppu_Scroll_Cycle[c1] & 7) == 0)
										{									
											if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
											{
												temp_color = ppu_Pixel_Color[c1] & 0xFF;
											}
											else
											{
												temp_color = (ppu_Pixel_Color[c1] >> 8) & 0xFF;
											}
										}
										else
										{
											if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
											{
												temp_color = (ppu_Pixel_Color[c1] >> 8) & 0xFF;
											}
											else
											{		
												temp_color = ppu_Pixel_Color[c1] & 0xFF;
											}
										}

										temp_color <<= 1;

										ppu_Pixel_Color_1[c1] = temp_color;
										ppu_BG_Has_Pixel_1[c1] = temp_color != 0;
									}
									else
									{
										if ((ppu_Scroll_Cycle[c1] & 0xF) == 0)
										{
											if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
											{
												temp_color = ((ppu_Pixel_Color[c1] >> 8) & 0xF);
											}
											else
											{
												temp_color = ((ppu_Pixel_Color[c1] >> 4) & 0xF);
											}
										}
										else if ((ppu_Scroll_Cycle[c1] & 0xF) == 4)
										{
											if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
											{
												temp_color = ((ppu_Pixel_Color[c1] >> 12) & 0xF);
											}
											else
											{
												temp_color = (ppu_Pixel_Color[c1] & 0xF);
											}
										}
										else if ((ppu_Scroll_Cycle[c1] & 0xF) == 8)
										{
											if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
											{
												temp_color = (ppu_Pixel_Color[c1] & 0xF);
											}
											else
											{
												temp_color = ((ppu_Pixel_Color[c1] >> 12) & 0xF);
											}
										}
										else if ((ppu_Scroll_Cycle[c1] & 0xF) == 12)
										{
											if ((ppu_BG_Effect_Byte[c1] & 0x4) == 0x0)
											{
												temp_color = ((ppu_Pixel_Color[c1] >> 4) & 0xF);
											}
											else
											{
												temp_color = ((ppu_Pixel_Color[c1] >> 8) & 0xF);
											}
										}

										temp_color |= (ppu_BG_Effect_Byte[c1] & 0xF0);

										temp_color <<= 1;

										ppu_Pixel_Color_1[c1] = temp_color;
										ppu_BG_Has_Pixel_1[c1] = ((temp_color & 0x1F) != 0);
									}		
								}
							}

							ppu_Pixel_Color_M[2] = ppu_Pixel_Color_1[2];
							ppu_Pixel_Color_1[2] = ppu_Pixel_Color_2[2];
							ppu_Pixel_Color_2[2] = ppu_Pixel_Color[2];

							ppu_BG_Has_Pixel_M[2] = ppu_BG_Has_Pixel_1[2];
							ppu_BG_Has_Pixel_1[2] = ppu_BG_Has_Pixel_2[2];
							ppu_BG_Has_Pixel_2[2] = ppu_BG_Has_Pixel[2];
							break;

						case 2:
							ppu_Pixel_Color_M[2] = ppu_Pixel_Color_1[2];
							ppu_Pixel_Color_1[2] = ppu_Pixel_Color_2[2];
							ppu_Pixel_Color_2[2] = ppu_Pixel_Color[2];

							ppu_Pixel_Color_M[3] = ppu_Pixel_Color_1[3];
							ppu_Pixel_Color_1[3] = ppu_Pixel_Color_2[3];
							ppu_Pixel_Color_2[3] = ppu_Pixel_Color[3];

							ppu_BG_Has_Pixel_M[2] = ppu_BG_Has_Pixel_1[2];
							ppu_BG_Has_Pixel_1[2] = ppu_BG_Has_Pixel_2[2];
							ppu_BG_Has_Pixel_2[2] = ppu_BG_Has_Pixel[2];

							ppu_BG_Has_Pixel_M[3] = ppu_BG_Has_Pixel_1[3];
							ppu_BG_Has_Pixel_1[3] = ppu_BG_Has_Pixel_2[3];
							ppu_BG_Has_Pixel_2[3] = ppu_BG_Has_Pixel[3];
							break;

						case 3:
						case 4:
						case 5:
							ppu_Pixel_Color_M[2] = ppu_Pixel_Color_1[2];
							ppu_Pixel_Color_1[2] = ppu_Pixel_Color_2[2];
							ppu_Pixel_Color_2[2] = ppu_Pixel_Color[2];

							ppu_BG_Has_Pixel_M[2] = ppu_BG_Has_Pixel_1[2];
							ppu_BG_Has_Pixel_1[2] = ppu_BG_Has_Pixel_2[2];
							ppu_BG_Has_Pixel_2[2] = ppu_BG_Has_Pixel[2];
							break;
					}
				}

				// render
				if (ppu_Cycle >= 46)
				{
					// determine what BG pixels will actually be rendered
					if (((ppu_Cycle-2) & 3) == 0)
					{
						cur_layer_priority = 4;
						second_layer_priority = 4;

						cur_BG_layer = 5;  // convenient to set to 5 for special effects
						second_BG_layer = 5;

						Is_Outside = true;
						OBJ_Go = false;
						Color_FX_Go = false;

						ppu_Fetch_BG = true;

						ppu_Brighten_Final_Pixel = false;
						ppu_Blend_Final_Pixel = false;

						if (ppu_Sprite_Is_Mosaic[ppu_Sprite_ofst_draw + ppu_Display_Cycle])
						{
							// if we are algined with the mosaic grid, latch new data
							// otherwise, if the currently latched data is not mosaic data, latch new data at the current pixel
							if ((ppu_Display_Cycle % ppu_OBJ_Mosaic_X) == 0)
							{
								ppu_Sprite_Pixel_Occupied_Latch = ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_draw + ppu_MOS_OBJ_X[ppu_Display_Cycle]];

								OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch & ppu_OBJ_On;

								if (OBJ_Has_Pixel)
								{
									spr_priority = ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_MOS_OBJ_X[ppu_Display_Cycle]];
									spr_semi_transparent = ppu_Sprite_Semi_Transparent[ppu_Sprite_ofst_draw + ppu_MOS_OBJ_X[ppu_Display_Cycle]];
									spr_pixel = ppu_Sprite_Pixels[ppu_Sprite_ofst_draw + ppu_MOS_OBJ_X[ppu_Display_Cycle]];
								}

								ppu_Sprite_Pixel_Latch = spr_pixel;
								ppu_Sprite_Priority_Latch = spr_priority;

								ppu_Sprite_Semi_Transparent_Latch = spr_semi_transparent;
								ppu_Sprite_Mosaic_Latch = true;
							}
							else
							{
								if (ppu_Sprite_Mosaic_Latch)
								{
									OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch & ppu_OBJ_On;
									spr_pixel = ppu_Sprite_Pixel_Latch;
									spr_priority = ppu_Sprite_Priority_Latch;
									spr_semi_transparent = ppu_Sprite_Semi_Transparent_Latch;
								}
								else
								{
									ppu_Sprite_Pixel_Occupied_Latch = ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_draw + ppu_Display_Cycle];

									OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch & ppu_OBJ_On;

									if (OBJ_Has_Pixel)
									{
										spr_priority = ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_Display_Cycle];
										spr_semi_transparent = ppu_Sprite_Semi_Transparent[ppu_Sprite_ofst_draw + ppu_Display_Cycle];
										spr_pixel = ppu_Sprite_Pixels[ppu_Sprite_ofst_draw + ppu_Display_Cycle];
									}


									ppu_Sprite_Pixel_Latch = spr_pixel;
									ppu_Sprite_Priority_Latch = spr_priority;

									ppu_Sprite_Semi_Transparent_Latch = spr_semi_transparent;
									ppu_Sprite_Mosaic_Latch = true;
								}
							}						
						}
						else
						{
							ppu_Sprite_Pixel_Occupied_Latch = ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_draw + ppu_Display_Cycle];

							OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch & ppu_OBJ_On;

							if (OBJ_Has_Pixel)
							{
								spr_priority = ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_Display_Cycle];
								spr_semi_transparent = ppu_Sprite_Semi_Transparent[ppu_Sprite_ofst_draw + ppu_Display_Cycle];
								spr_pixel = ppu_Sprite_Pixels[ppu_Sprite_ofst_draw + ppu_Display_Cycle];
							}


							ppu_Sprite_Pixel_Latch = spr_pixel;
							ppu_Sprite_Priority_Latch = spr_priority;

							ppu_Sprite_Semi_Transparent_Latch = spr_semi_transparent;
							ppu_Sprite_Mosaic_Latch = false;
						}

						BG_Go_Disp[0] = ppu_BG_On[0];
						BG_Go_Disp[1] = ppu_BG_On[1];
						BG_Go_Disp[2] = ppu_BG_On[2];
						BG_Go_Disp[3] = ppu_BG_On[3];

						ppu_BG_Pixel_F = 0;
						ppu_BG_Pixel_S = 0;

						ppu_Fetch_Target_1 = false;
						ppu_Fetch_Target_2 = false;

						// latch final color from BG's according to mosaic
						for (int i = 0; i < 4; i++)
						{
							if (ppu_BG_On[i])
							{
								if (ppu_BG_Mosaic[i])
								{
									if ((ppu_Display_Cycle % ppu_BG_Mosaic_X_Mod) == 0)
									{
										ppu_Pixel_Color_R[i] = ppu_Pixel_Color_M[i];
										ppu_BG_Has_Pixel_R[i] = ppu_BG_Has_Pixel_M[i];
									}
								}
								else
								{
									ppu_Pixel_Color_R[i] = ppu_Pixel_Color_M[i];
									ppu_BG_Has_Pixel_R[i] = ppu_BG_Has_Pixel_M[i];
								}
							}
						}					

						// Check enabled pixels
						if (ppu_Any_Window_On)
						{
							if (ppu_WIN0_On && (((ppu_Display_Cycle - ppu_WIN0_Left) & 0xFF) < ((ppu_WIN0_Right - ppu_WIN0_Left) & 0xFF)) &&
								ppu_WIN0_Active)
							{
								Is_Outside = false;

								for (int w0 = 0; w0 < 4; w0++) { BG_Go_Disp[w0] &= ppu_WIN0_BG_En[w0]; }

								OBJ_Go = ppu_WIN0_OBJ_En & OBJ_Has_Pixel;
								Color_FX_Go = ppu_WIN0_Color_En;
							}
							else if (ppu_WIN1_On && (((ppu_Display_Cycle - ppu_WIN1_Left) & 0xFF) < ((ppu_WIN1_Right - ppu_WIN1_Left) & 0xFF)) &&
									ppu_WIN1_Active)
							{
								Is_Outside = false;

								for (int w1 = 0; w1 < 4; w1++) { BG_Go_Disp[w1] &= ppu_WIN1_BG_En[w1]; }

								OBJ_Go = ppu_WIN1_OBJ_En & OBJ_Has_Pixel;
								Color_FX_Go = ppu_WIN1_Color_En;
							}
							else if (ppu_OBJ_WIN && ppu_Sprite_Object_Window[ppu_Sprite_ofst_draw + ppu_Display_Cycle])
							{
								Is_Outside = false;

								for (int ob = 0; ob < 4; ob++) { BG_Go_Disp[ob] &= ppu_OBJ_BG_En[ob]; }

								OBJ_Go = ppu_OBJ_OBJ_En & OBJ_Has_Pixel;
								Color_FX_Go = ppu_OBJ_Color_En;
							}

							if (Is_Outside)
							{
								for (int outs = 0; outs < 4; outs++) { BG_Go_Disp[outs] &= ppu_OUT_BG_En[outs]; }

								OBJ_Go = ppu_OUT_OBJ_En & OBJ_Has_Pixel;
								Color_FX_Go = ppu_OUT_Color_En;
							}
						}
						else
						{
							OBJ_Go = OBJ_Has_Pixel;
							Color_FX_Go = true;
						}

						switch (ppu_BG_Mode)
						{
							case 0:
								for (int c0 = 0; c0 < 4; c0++)
								{
									if (BG_Go_Disp[c0])
									{
										if (ppu_BG_Has_Pixel_R[c0])
										{
											if (ppu_BG_Priority[c0] < cur_layer_priority)
											{
												ppu_BG_Pixel_S = ppu_BG_Pixel_F;
												second_BG_layer = cur_BG_layer;
												second_layer_priority = cur_layer_priority;

												ppu_BG_Pixel_F = (uint)ppu_Pixel_Color_R[c0];
												cur_BG_layer = c0;
												cur_layer_priority = ppu_BG_Priority[c0];
											}
											else if (ppu_BG_Priority[c0] < second_layer_priority)
											{
												ppu_BG_Pixel_S = (uint)ppu_Pixel_Color_R[c0];
												second_BG_layer = c0;
												second_layer_priority = ppu_BG_Priority[c0];
											}
										}
									}
								}
								break;

							case 1:
								for (int c1 = 0; c1 < 2; c1++)
								{
									if (BG_Go_Disp[c1])
									{
										if (ppu_BG_Has_Pixel_R[c1])
										{
											if (ppu_BG_Priority[c1] < cur_layer_priority)
											{
												ppu_BG_Pixel_S = ppu_BG_Pixel_F;
												second_BG_layer = cur_BG_layer;
												second_layer_priority = cur_layer_priority;

												ppu_BG_Pixel_F = (uint)ppu_Pixel_Color_R[c1];
												cur_BG_layer = c1;
												cur_layer_priority = ppu_BG_Priority[c1];
											}
											else if (ppu_BG_Priority[c1] < second_layer_priority)
											{
												ppu_BG_Pixel_S = (uint)ppu_Pixel_Color_R[c1];
												second_BG_layer = c1;
												second_layer_priority = ppu_BG_Priority[c1];
											}
										}
									}
								}

								if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
								{
									if (ppu_BG_Priority[2] < cur_layer_priority)
									{
										ppu_BG_Pixel_S = ppu_BG_Pixel_F;
										second_BG_layer = cur_BG_layer;
										second_layer_priority = cur_layer_priority;

										ppu_BG_Pixel_F = (uint)ppu_Pixel_Color_R[2];
										cur_BG_layer = 2;
										cur_layer_priority = ppu_BG_Priority[2];
									}
									else if (ppu_BG_Priority[2] < second_layer_priority)
									{
										ppu_BG_Pixel_S = (uint)ppu_Pixel_Color_R[2];
										second_BG_layer = 2;
										second_layer_priority = ppu_BG_Priority[2];
									}
								}
								break;

							case 2:
								for (int c2 = 2; c2 < 4; c2++)
								{
									if (BG_Go_Disp[c2] && ppu_BG_Has_Pixel_R[c2])
									{
										if (ppu_BG_Priority[c2] < cur_layer_priority)
										{
											ppu_BG_Pixel_S = ppu_BG_Pixel_F;
											second_BG_layer = cur_BG_layer;
											second_layer_priority = cur_layer_priority;

											ppu_BG_Pixel_F = (uint)ppu_Pixel_Color_R[c2];
											cur_BG_layer = c2;
											cur_layer_priority = ppu_BG_Priority[c2];
										}
										else if (ppu_BG_Priority[c2] < second_layer_priority)
										{
											ppu_BG_Pixel_S = (uint)ppu_Pixel_Color_R[c2];
											second_BG_layer = c2;
											second_layer_priority = ppu_BG_Priority[c2];
										}
									}
								}
								break;

							case 3:
								// Direct Bitmap only, BG2
								if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
								{
									// no transparency possible
									cur_BG_layer = 2;
									cur_layer_priority = ppu_BG_Priority[2];

									ppu_Fetch_BG = false;

									ppu_BG_Pixel_F = (uint)ppu_Pixel_Color_R[2];
								}
								break;

							case 4:
								// bitmaps, only BG2
								if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
								{
									// no transparency possible
									cur_BG_layer = 2;
									cur_layer_priority = ppu_BG_Priority[2];
									ppu_BG_Pixel_F = (uint)ppu_Pixel_Color_R[2];
								}
								break;

							case 5:
								// bitmaps, only BG2
								if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
								{
									// no transparency possible
									cur_BG_layer = 2;
									cur_layer_priority = ppu_BG_Priority[2];

									ppu_Fetch_BG = false;

									ppu_BG_Pixel_F = (uint)ppu_Pixel_Color_R[2];
								}
								break;

							case 6:
								// invalid
								break;

							case 7:
								// invalid
								break;
						}

						// determine final pixel color, based on sprites and special effects
						if (OBJ_Go)
						{
							//Console.WriteLine(ppu_Sprite_Priority[ppu_Sprite_ofst_draw + ppu_Display_Cycle] + " " + cur_layer_priority + " " + cur_BG_layer + " " + ppu_LY);
							// sprite pixel available, check ordering
							if (spr_priority <= cur_layer_priority)
							{
								// sprite pixel has higher priority than BG pixel
								if (spr_semi_transparent)
								{
									// semi transparent pixels with highest priority always enable alpha blending if possible, even if otherwise disabled.
									// alpha blend if possible
									if ((ppu_Special_FX & (1 << (cur_BG_layer + 8))) != 0)
									{
										// Alpha blending Sprite - BG
										ppu_Final_Pixel = spr_pixel;
										ppu_Blend_Pixel = ppu_BG_Pixel_F;

										ppu_Fetch_Target_1 = true;
										ppu_Fetch_Target_2 = ppu_Fetch_BG;

										ppu_Blend_Final_Pixel = true;
									}
									else
									{
										ppu_Final_Pixel = spr_pixel;

										ppu_Fetch_Target_1 = true;

										if ((ppu_SFX_mode >= 2) && ppu_SFX_OBJ_Target_1 && Color_FX_Go)
										{
											ppu_Brighten_Final_Pixel = true;
										}
									}
								}
								else if (Color_FX_Go)
								{
									// Alpha blending as normal if enabled
									if (ppu_SFX_mode != 1)
									{
										ppu_Final_Pixel = spr_pixel;

										ppu_Fetch_Target_1 = true;

										if ((ppu_SFX_mode != 0) && ppu_SFX_OBJ_Target_1)
										{
											ppu_Brighten_Final_Pixel = true;
										}
									}
									else
									{
										// alpha blend if possible
										if (((ppu_Special_FX & (1 << (cur_BG_layer + 8))) != 0) && ppu_SFX_OBJ_Target_1)
										{
											// Alpha blending Sprite - BG
											ppu_Final_Pixel = spr_pixel;
											ppu_Blend_Pixel = ppu_BG_Pixel_F;

											ppu_Fetch_Target_1 = true;
											ppu_Fetch_Target_2 = ppu_Fetch_BG;

											ppu_Blend_Final_Pixel = true;
										}
										else
										{
											ppu_Final_Pixel = spr_pixel;

											ppu_Fetch_Target_1 = true;
										}
									}
								}
								else
								{
									ppu_Final_Pixel = spr_pixel;

									ppu_Fetch_Target_1 = true;
								}
							}
							else
							{
								if (Color_FX_Go)
								{
									// BG pixel has higher priority than sprite pixel
									if (ppu_SFX_mode != 1)
									{
										ppu_Final_Pixel = ppu_BG_Pixel_F;

										ppu_Fetch_Target_1 = ppu_Fetch_BG;

										if ((ppu_SFX_mode != 0) && ((ppu_Special_FX & (1 << cur_BG_layer)) != 0))
										{
											ppu_Brighten_Final_Pixel = true;
										}
									}
									else
									{
										// alpha blend if possible
										// check if another bg layer has a higher priority pixel than the sprite
										if ((ppu_Special_FX & (1 << cur_BG_layer)) != 0)
										{
											if ((second_layer_priority < spr_priority) && ((ppu_Special_FX & (1 << (second_BG_layer + 8))) != 0))
											{
												// Alpha blending BG - BG
												ppu_Final_Pixel = ppu_BG_Pixel_F;
												ppu_Blend_Pixel = ppu_BG_Pixel_S;

												ppu_Fetch_Target_1 = ppu_Fetch_BG;
												ppu_Fetch_Target_2 = true;

												ppu_Blend_Final_Pixel = true;
											}
											else if ((spr_priority <= second_layer_priority) && ppu_SFX_OBJ_Target_2)
											{
												// Alpha blending BG - Sprite
												ppu_Final_Pixel = ppu_BG_Pixel_F;
												ppu_Blend_Pixel = spr_pixel;

												ppu_Fetch_Target_1 = ppu_Fetch_BG;
												ppu_Fetch_Target_2 = true;

												ppu_Blend_Final_Pixel = true;
											}
											else
											{
												ppu_Final_Pixel = ppu_BG_Pixel_F;

												ppu_Fetch_Target_1 = ppu_Fetch_BG;
											}
										}
										else
										{
											ppu_Final_Pixel = ppu_BG_Pixel_F;

											ppu_Fetch_Target_1 = ppu_Fetch_BG;
										}
									}
								}
								else
								{
									ppu_Final_Pixel = ppu_BG_Pixel_F;

									ppu_Fetch_Target_1 = ppu_Fetch_BG;
								}
							}
						}
						else
						{
							// only BG pixels available, check brightness and 1st target for special effects
							if (Color_FX_Go)
							{
								if (ppu_SFX_mode != 1)
								{
									ppu_Final_Pixel = ppu_BG_Pixel_F;

									ppu_Fetch_Target_1 = ppu_Fetch_BG;

									if ((ppu_SFX_mode >= 2) && ((ppu_Special_FX & (1 << cur_BG_layer)) != 0))
									{
										ppu_Brighten_Final_Pixel = true;
									}
								}
								else
								{
									// alpha blend if possible
									// check if the top two layers are targets for blending
									if (((ppu_Special_FX & (1 << cur_BG_layer)) != 0) && ((ppu_Special_FX & (1 << (second_BG_layer + 8))) != 0))
									{
										// Alpha blending BG - BG
										ppu_Final_Pixel = ppu_BG_Pixel_F;
										ppu_Blend_Pixel = ppu_BG_Pixel_S;

										ppu_Fetch_Target_1 = ppu_Fetch_BG;
										ppu_Fetch_Target_2 = true;

										ppu_Blend_Final_Pixel = true;
									}
									else
									{
										ppu_Final_Pixel = ppu_BG_Pixel_F;

										ppu_Fetch_Target_1 = ppu_Fetch_BG;
									}
								}
							}
							else
							{
								ppu_Final_Pixel = ppu_BG_Pixel_F;

								ppu_Fetch_Target_1 = ppu_Fetch_BG;
							}
						}
					}
					else if (((ppu_Cycle - 2) & 3) == 1)
					{					
						if (ppu_Fetch_Target_1)
						{
							ppu_PALRAM_Access = true;

							ppu_Final_Pixel = (uint)(PALRAM[ppu_Final_Pixel] | (PALRAM[ppu_Final_Pixel + 1] << 8));

							if (ppu_PALRAM_In_Use)
							{
								cpu_Fetch_Wait += 1;
								dma_Access_Wait += 1;
							}
						}
					}
					else if (((ppu_Cycle - 2) & 3) == 3)
					{
						if (ppu_Fetch_Target_2)
						{
							ppu_PALRAM_Access = true;

							ppu_Blend_Pixel = (uint)(PALRAM[ppu_Blend_Pixel] | (PALRAM[ppu_Blend_Pixel + 1] << 8));

							if (ppu_PALRAM_In_Use)
							{
								cpu_Fetch_Wait += 1;
								dma_Access_Wait += 1;
							}
						}

						if (ppu_Brighten_Final_Pixel)
						{
							R = (ppu_Final_Pixel >> 10) & 0x1F;
							G = (ppu_Final_Pixel >> 5) & 0x1F;
							B = ppu_Final_Pixel & 0x1F;

							if (ppu_SFX_mode == 2)
							{
								R = (uint)(R + (((31 - R) * ppu_SFX_BRT_Num) >> 4));
								G = (uint)(G + (((31 - G) * ppu_SFX_BRT_Num) >> 4));
								B = (uint)(B + (((31 - B) * ppu_SFX_BRT_Num) >> 4));
							}
							else
							{
								R = (uint)(R - ((R * ppu_SFX_BRT_Num) >> 4));
								G = (uint)(G - ((G * ppu_SFX_BRT_Num) >> 4));
								B = (uint)(B - ((B * ppu_SFX_BRT_Num) >> 4));
							}

							ppu_Final_Pixel = (uint)(0xFF000000 |
												 (R << 3) |
												 (G << 11) |
												 (B << 19));
						}
						else if (ppu_Blend_Final_Pixel)
						{
							R = (ppu_Final_Pixel >> 10) & 0x1F;
							G = (ppu_Final_Pixel >> 5) & 0x1F;
							B = ppu_Final_Pixel & 0x1F;

							R2 = (ppu_Blend_Pixel >> 10) & 0x1F;
							G2 = (ppu_Blend_Pixel >> 5) & 0x1F;
							B2 = ppu_Blend_Pixel & 0x1F;

							R = (uint)(((R * ppu_SFX_Alpha_Num_1) >> 4) + ((R2 * ppu_SFX_Alpha_Num_2) >> 4));
							G = (uint)(((G * ppu_SFX_Alpha_Num_1) >> 4) + ((G2 * ppu_SFX_Alpha_Num_2) >> 4));
							B = (uint)(((B * ppu_SFX_Alpha_Num_1) >> 4) + ((B2 * ppu_SFX_Alpha_Num_2) >> 4));

							if (R > 31) { R = 31; }
							if (G > 31) { G = 31; }
							if (B > 31) { B = 31; }

							ppu_Final_Pixel = (uint)(0xFF000000 |
												 (R << 3) |
												 (G << 11) |
												 (B << 19));
						}
						else
						{
							ppu_Final_Pixel = (uint)(0xFF000000 |
													((ppu_Final_Pixel & 0x1F) << 19) |
													((ppu_Final_Pixel & 0x3E0) << 6) |
													((ppu_Final_Pixel & 0x7C00) >> 7));
						}

						// push pixel to display
						vid_buffer[ppu_Display_Cycle + ppu_LY * 240] = unchecked((int)ppu_Final_Pixel);

						if (ppu_Do_Green_Swap)
						{
							if ((ppu_Display_Cycle & 1) == 1)
							{
								int temp_pixel = vid_buffer[ppu_Display_Cycle - 1 + ppu_LY * 240];

								int temp_pixel_2 = temp_pixel;

								temp_pixel &= unchecked((int)0xFFFF00FF);

								temp_pixel |= (int)(ppu_Final_Pixel & 0x0000FF00);

								ppu_Final_Pixel &= 0xFFFF00FF;
								ppu_Final_Pixel |= (uint)(temp_pixel_2 & 0x0000FF00);

								vid_buffer[ppu_Display_Cycle + ppu_LY * 240] = unchecked((int)ppu_Final_Pixel);
								vid_buffer[ppu_Display_Cycle - 1 + ppu_LY * 240] = unchecked((int)temp_pixel);
							}
						}

						ppu_Display_Cycle += 1;

						if (ppu_Display_Cycle == 240)
						{
							ppu_PAL_Rendering_Complete = true;

							ppu_Rendering_Complete = true;

							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
							ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
						}
					}
				}			
			}

			int a0, a1;

			switch (ppu_BG_Mode)
			{
				case 0:
					a0 = ppu_Cycle & 3;

					if (!ppu_BG_Rendering_Complete[a0])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[a0]))
						{
							if ((ppu_Scroll_Cycle[a0] & 31) == 0)
							{
								// calculate scrolling
								if (ppu_BG_Mosaic[a0])
								{
									ppu_X_RS = (int)(((ppu_Fetch_Count[a0] << 3) + ppu_BG_X_Latch[a0]) & 0x1FF);
									ppu_Y_RS = (int)((ppu_MOS_BG_Y[ppu_LY] + ppu_BG_Y_Latch[a0]) & 0x1FF);
								}
								else
								{
									ppu_X_RS = (int)(((ppu_Fetch_Count[a0] << 3) + ppu_BG_X_Latch[a0]) & 0x1FF);
									ppu_Y_RS = (int)((ppu_LY + ppu_BG_Y_Latch[a0]) & 0x1FF);
								}

								// always wrap around, this means pixels will always be in a valid range
								ppu_X_RS &= (BG_Scale_X[a0] - 1);
								ppu_Y_RS &= (BG_Scale_Y[a0] - 1);

								ppu_Y_Flip_Ofst[a0] = ppu_Y_RS & 7;

								// this access will always be in bounds
								ppu_Set_VRAM_Access_True();

								VRAM_ofst_X = ppu_X_RS >> 3;
								VRAM_ofst_Y = ppu_Y_RS >> 3;

								Screen_Offset = 0;

								if (VRAM_ofst_X > 31)
								{
									if (VRAM_ofst_Y > 31)
									{
										Screen_Offset = 6 * 1024;
									}
									else
									{
										Screen_Offset = 2 * 1024;
									}
								}
								else if (VRAM_ofst_Y > 31)
								{
									if (ppu_BG_Screen_Size[a0] == 2)
									{
										Screen_Offset = 2 * 1024;
									}
									else
									{
										Screen_Offset = 4 * 1024;
									}
								}

								VRAM_ofst_X &= 31;
								VRAM_ofst_Y &= 31;

								ppu_Tile_Addr[a0] = ppu_BG_Screen_Base[a0] + Screen_Offset + VRAM_ofst_Y * BG_Num_Tiles[a0] * 2 + VRAM_ofst_X * 2;
								
								ppu_VRAM_Open_Bus = (ushort)(VRAM[ppu_Tile_Addr[a0]] | (VRAM[ppu_Tile_Addr[a0] + 1] << 8));

								ppu_BG_Effect_Byte_New[a0] = (byte)(ppu_VRAM_Open_Bus >> 8);

								ppu_Tile_Addr[a0] = (ushort)(ppu_VRAM_Open_Bus & 0x3FF);
							}
							else if (((ppu_Scroll_Cycle[a0] & 31) == 4) || ((ppu_Scroll_Cycle[a0] & 31) == 20))
							{
								// this update happens here so that rendering isn't effected further up
								ppu_BG_Effect_Byte[a0] = ppu_BG_Effect_Byte_New[a0];

								if (ppu_BG_Pal_Size[a0])
								{
									temp_addr = ppu_Tile_Addr[a0];

									temp_addr = temp_addr * 64 + ppu_BG_Char_Base[a0];

									if ((ppu_BG_Effect_Byte[a0] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[a0] & 31) == 4)
										{
											temp_addr += 0;
										}
										else
										{
											temp_addr += 4;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[a0] & 31) == 4)
										{
											temp_addr += 6;
										}
										else
										{
											temp_addr += 2;
										}
									}

									if ((ppu_BG_Effect_Byte[a0] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[a0] * 8;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[a0]) * 8;
									}

									if (temp_addr < 0x10000)
									{
										ppu_Set_VRAM_Access_True();

										ppu_VRAM_Open_Bus = (ushort)(VRAM[temp_addr] | (VRAM[temp_addr + 1] << 8));
									}

									ppu_Pixel_Color[a0] = ppu_VRAM_Open_Bus;
								}
								else
								{
									temp_addr = ppu_Tile_Addr[a0];

									temp_addr = temp_addr * 32 + ppu_BG_Char_Base[a0];

									if ((ppu_BG_Effect_Byte[a0] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[a0] * 4;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[a0]) * 4;
									}

									if ((ppu_BG_Effect_Byte[a0] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[a0] & 31) == 4)
										{
											temp_addr += 0;
										}
										else
										{
											temp_addr += 2;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[a0] & 31) == 4)
										{
											temp_addr += 2;
										}
										else
										{
											temp_addr += 0;
										}
									}

									if (temp_addr < 0x10000)
									{
										ppu_Set_VRAM_Access_True();

										ppu_VRAM_Open_Bus = (ushort)(VRAM[temp_addr] | (VRAM[temp_addr + 1] << 8));
									}

									ppu_Pixel_Color[a0] = ppu_VRAM_Open_Bus;
								}
							}
							else if (((ppu_Scroll_Cycle[a0] & 31) == 12) || ((ppu_Scroll_Cycle[a0] & 31) == 28))
							{
								// this access will only occur in 256color mode
								if (ppu_BG_Pal_Size[a0])
								{
									temp_addr = ppu_Tile_Addr[a0];

									temp_addr = temp_addr * 64 + ppu_BG_Char_Base[a0];

									if ((ppu_BG_Effect_Byte[a0] & 0x4) == 0x0)
									{
										if ((ppu_Scroll_Cycle[a0] & 31) == 12)
										{
											temp_addr += 2;
										}
										else
										{
											temp_addr += 6;
										}
									}
									else
									{
										if ((ppu_Scroll_Cycle[a0] & 31) == 12)
										{
											temp_addr += 4;
										}
										else
										{
											temp_addr += 0;
										}
									}

									if ((ppu_BG_Effect_Byte[a0] & 0x8) == 0x0)
									{
										temp_addr += ppu_Y_Flip_Ofst[a0] * 8;
									}
									else
									{
										temp_addr += (7 - ppu_Y_Flip_Ofst[a0]) * 8;
									}

									if (temp_addr < 0x10000)
									{
										ppu_Set_VRAM_Access_True();

										ppu_VRAM_Open_Bus = (ushort)(VRAM[temp_addr] | (VRAM[temp_addr + 1] << 8));
									}

									ppu_Pixel_Color[a0] = ppu_VRAM_Open_Bus;
								}

								if ((ppu_Scroll_Cycle[a0] & 31) == 28)
								{
									ppu_Fetch_Count[a0] += 1;

									if (ppu_Fetch_Count[a0] == 31)
									{
										ppu_BG_Rendering_Complete[a0] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}

							ppu_Scroll_Cycle[a0] += 4;
						}
					}
					break;

				case 1:
					a1 = ppu_Cycle & 3;

					if (a1 < 2)
					{
						if (!ppu_BG_Rendering_Complete[a1])
						{
							if (ppu_Cycle >= ppu_BG_Start_Time[a1])
							{
								if ((ppu_Scroll_Cycle[a1] & 31) == 0)
								{
									// calculate scrolling
									if (ppu_BG_Mosaic[a1])
									{
										ppu_X_RS = (int)(((ppu_Fetch_Count[a1] << 3) + ppu_BG_X_Latch[a1]) & 0x1FF);
										ppu_Y_RS = (int)((ppu_MOS_BG_Y[ppu_LY] + ppu_BG_Y_Latch[a1]) & 0x1FF);
									}
									else
									{
										ppu_X_RS = (int)(((ppu_Fetch_Count[a1] << 3) + ppu_BG_X_Latch[a1]) & 0x1FF);
										ppu_Y_RS = (int)((ppu_LY + ppu_BG_Y_Latch[a1]) & 0x1FF);
									}

									// always wrap around, so pixel is always in range
									ppu_X_RS &= (BG_Scale_X[a1] - 1);
									ppu_Y_RS &= (BG_Scale_Y[a1] - 1);

									ppu_Y_Flip_Ofst[a1] = ppu_Y_RS & 7;

									// this access will always be in bounds
									ppu_Set_VRAM_Access_True();

									VRAM_ofst_X = ppu_X_RS >> 3;
									VRAM_ofst_Y = ppu_Y_RS >> 3;

									Screen_Offset = 0;

									if (VRAM_ofst_X > 31)
									{
										if (VRAM_ofst_Y > 31)
										{
											Screen_Offset = 6 * 1024;
										}
										else
										{
											Screen_Offset = 2 * 1024;
										}
									}
									else if (VRAM_ofst_Y > 31)
									{
										if (ppu_BG_Screen_Size[a1] == 2)
										{
											Screen_Offset = 2 * 1024;
										}
										else
										{
											Screen_Offset = 4 * 1024;
										}
									}

									VRAM_ofst_X &= 31;
									VRAM_ofst_Y &= 31;

									ppu_Tile_Addr[a1] = ppu_BG_Screen_Base[a1] + Screen_Offset + VRAM_ofst_Y * BG_Num_Tiles[a1] * 2 + VRAM_ofst_X * 2;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[ppu_Tile_Addr[a1]] | (VRAM[ppu_Tile_Addr[a1] + 1] << 8));

									ppu_BG_Effect_Byte_New[a1] = (byte)(ppu_VRAM_Open_Bus >> 8);

									ppu_Tile_Addr[a1] = (ushort)(ppu_VRAM_Open_Bus & 0x3FF);
								}
								else if (((ppu_Scroll_Cycle[a1] & 31) == 4) || ((ppu_Scroll_Cycle[a1] & 31) == 20))
								{
									// this update happens here so that rendering isn't effected further up
									ppu_BG_Effect_Byte[a1] = ppu_BG_Effect_Byte_New[a1];

									if (ppu_BG_Pal_Size[a1])
									{
										temp_addr = ppu_Tile_Addr[a1];

										temp_addr = temp_addr * 64 + ppu_BG_Char_Base[a1];

										if ((ppu_BG_Effect_Byte[a1] & 0x4) == 0x0)
										{
											if ((ppu_Scroll_Cycle[a1] & 31) == 4)
											{
												temp_addr += 0;
											}
											else
											{
												temp_addr += 4;
											}
										}
										else
										{
											if ((ppu_Scroll_Cycle[a1] & 31) == 4)
											{
												temp_addr += 6;
											}
											else
											{
												temp_addr += 2;
											}
										}

										if ((ppu_BG_Effect_Byte[a1] & 0x8) == 0x0)
										{
											temp_addr += ppu_Y_Flip_Ofst[a1] * 8;
										}
										else
										{
											temp_addr += (7 - ppu_Y_Flip_Ofst[a1]) * 8;
										}

										if (temp_addr < 0x10000)
										{
											ppu_Set_VRAM_Access_True();

											ppu_VRAM_Open_Bus = (ushort)(VRAM[temp_addr] | (VRAM[temp_addr + 1] << 8));
										}

										ppu_Pixel_Color[a1] = ppu_VRAM_Open_Bus;
									}
									else
									{
										temp_addr = ppu_Tile_Addr[a1];

										temp_addr = temp_addr * 32 + ppu_BG_Char_Base[a1];

										if ((ppu_BG_Effect_Byte[a1] & 0x8) == 0x0)
										{
											temp_addr += ppu_Y_Flip_Ofst[a1] * 4;
										}
										else
										{
											temp_addr += (7 - ppu_Y_Flip_Ofst[a1]) * 4;
										}

										if ((ppu_BG_Effect_Byte[a1] & 0x4) == 0x0)
										{
											if ((ppu_Scroll_Cycle[a1] & 31) == 4)
											{
												temp_addr += 0;
											}
											else
											{
												temp_addr += 2;
											}
										}
										else
										{
											if ((ppu_Scroll_Cycle[a1] & 31) == 4)
											{
												temp_addr += 2;
											}
											else
											{
												temp_addr += 0;
											}
										}

										if (temp_addr < 0x10000)
										{
											ppu_Set_VRAM_Access_True();

											ppu_VRAM_Open_Bus = (ushort)(VRAM[temp_addr] | (VRAM[temp_addr + 1] << 8));
										}

										ppu_Pixel_Color[a1] = ppu_VRAM_Open_Bus;
									}
								}
								else if (((ppu_Scroll_Cycle[a1] & 31) == 12) || ((ppu_Scroll_Cycle[a1] & 31) ==28))
								{
									// this access will only occur in 256color mode
									if (ppu_BG_Pal_Size[a1])
									{
										temp_addr = ppu_Tile_Addr[a1];

										temp_addr = temp_addr * 64 + ppu_BG_Char_Base[a1];

										if ((ppu_BG_Effect_Byte[a1] & 0x4) == 0x0)
										{
											if ((ppu_Scroll_Cycle[a1] & 31) == 12)
											{
												temp_addr += 2;
											}
											else
											{
												temp_addr += 6;
											}
										}
										else
										{
											if ((ppu_Scroll_Cycle[a1] & 31) == 12)
											{
												temp_addr += 4;
											}
											else
											{
												temp_addr += 0;
											}
										}

										if ((ppu_BG_Effect_Byte[a1] & 0x8) == 0x0)
										{
											temp_addr += ppu_Y_Flip_Ofst[a1] * 8;
										}
										else
										{
											temp_addr += (7 - ppu_Y_Flip_Ofst[a1]) * 8;
										}

										if (temp_addr < 0x10000)
										{
											ppu_Set_VRAM_Access_True();

											ppu_VRAM_Open_Bus = (ushort)(VRAM[temp_addr] | (VRAM[temp_addr + 1] << 8));
										}

										ppu_Pixel_Color[a1] = ppu_VRAM_Open_Bus;
									}

									if ((ppu_Scroll_Cycle[a1] & 31) == 28)
									{
										ppu_Fetch_Count[a1] += 1;

										if (ppu_Fetch_Count[a1] == 31)
										{
											ppu_BG_Rendering_Complete[a1] = true;

											ppu_Rendering_Complete = true;

											ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
											ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
											ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
											ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
											ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
										}
									}
								}

								ppu_Scroll_Cycle[a1] += 4;			
							}
						}
					}

					if (!ppu_BG_Rendering_Complete[2])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
						{
							if ((ppu_Cycle & 3) == 2)
							{
								// calculate rotation and scaling
								cur_x = (ulong)ppu_Fetch_Count[2];

								sol_x = ppu_F_Rot_A_2 * cur_x;
								sol_y = ppu_F_Rot_C_2 * cur_x;

								sol_x += ppu_Current_Ref_X_2;
								sol_y += ppu_Current_Ref_Y_2;

								sol_x >>= 8;
								sol_y >>= 8;

								ppu_X_RS = (ushort)sol_x;
								ppu_Y_RS = (ushort)sol_y;

								// adjust if wraparound is enabled
								if (ppu_BG_Overflow[2])
								{
									ppu_X_RS &= (BG_Scale_X[2] - 1);
									ppu_Y_RS &= (BG_Scale_Y[2] - 1);
								}

								VRAM_ofst_X = ppu_X_RS >> 3;
								VRAM_ofst_Y = ppu_Y_RS >> 3;

								int m1_2_ofst = ppu_BG_Screen_Base[2] + VRAM_ofst_Y * BG_Num_Tiles[2] + VRAM_ofst_X;

								if (m1_2_ofst < 0x10000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_Tile_Addr[2] = VRAM[m1_2_ofst] << 6;

									m1_2_ofst &= 0xFFFE;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[m1_2_ofst] | (VRAM[m1_2_ofst + 1] << 8));
								}
								else
								{
									if ((m1_2_ofst & 1) == 1)
									{
										ppu_Tile_Addr[2] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 2);
									}
									else
									{
										ppu_Tile_Addr[2] = ((ppu_VRAM_Open_Bus & 0xFF) << 6);
									}
								}

								// determine if pixel is in valid range, and pick out color if so
								if ((ppu_X_RS < BG_Scale_X[2]) && (ppu_Y_RS < BG_Scale_Y[2]) && (ppu_Fetch_Count[2] < 240))
								{
									ppu_BG_Has_Pixel[2] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;

									if (ppu_Fetch_Count[2] == 243)
									{
										ppu_BG_Rendering_Complete[2] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}
							else if ((ppu_Cycle & 3) == 3)
							{
								ppu_Tile_Addr[2] += ppu_BG_Char_Base[2] + (ppu_X_RS & 7) + (ppu_Y_RS & 7) * 8;

								if (ppu_Tile_Addr[2] < 0x10000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_Pixel_Color[2] = VRAM[ppu_Tile_Addr[2]] << 1;

									ppu_Tile_Addr[2] &= 0xFFFE;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[ppu_Tile_Addr[2]] | (VRAM[ppu_Tile_Addr[2] + 1] << 8));
								}
								else
								{
									if ((ppu_Tile_Addr[2] & 1) == 1)
									{
										ppu_Pixel_Color[2] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 7);
									}
									else
									{
										ppu_Pixel_Color[2] = ((ppu_VRAM_Open_Bus & 0xFF) << 1);
									}
								}

								if (ppu_Pixel_Color[2] != 0)
								{
									ppu_BG_Has_Pixel[2] &= true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;
								}

								if (ppu_Fetch_Count[2] >= 240)
								{
									ppu_BG_Has_Pixel[2] = false;
								}

								ppu_Fetch_Count[2] += 1;
							}
						}
					}
					break;

				case 2:
					if (!ppu_BG_Rendering_Complete[2])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
						{
							if ((ppu_Cycle & 3) == 2)
							{
								// calculate rotation and scaling
								cur_x = (ulong)ppu_Fetch_Count[2];

								sol_x = ppu_F_Rot_A_2 * cur_x;
								sol_y = ppu_F_Rot_C_2 * cur_x;

								sol_x += ppu_Current_Ref_X_2;
								sol_y += ppu_Current_Ref_Y_2;

								sol_x >>= 8;
								sol_y >>= 8;

								ppu_X_RS = (ushort)sol_x;
								ppu_Y_RS = (ushort)sol_y;

								// adjust if wraparound is enabled
								if (ppu_BG_Overflow[2])
								{
									ppu_X_RS &= (BG_Scale_X[2] - 1);
									ppu_Y_RS &= (BG_Scale_Y[2] - 1);
								}

								VRAM_ofst_X = ppu_X_RS >> 3;
								VRAM_ofst_Y = ppu_Y_RS >> 3;

								int m2_2_ofst = ppu_BG_Screen_Base[2] + VRAM_ofst_Y * BG_Num_Tiles[2] + VRAM_ofst_X;

								if (m2_2_ofst < 0x10000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_Tile_Addr[2] = VRAM[m2_2_ofst] << 6;

									m2_2_ofst &= 0xFFFE;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[m2_2_ofst] | (VRAM[m2_2_ofst + 1] << 8));
								}
								else
								{
									if ((m2_2_ofst & 1) == 1)
									{
										ppu_Tile_Addr[2] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 2);
									}
									else
									{
										ppu_Tile_Addr[2] = ((ppu_VRAM_Open_Bus & 0xFF) << 6);
									}
								}

								// determine if pixel is in valid range, and pick out color if so
								if ((ppu_X_RS < BG_Scale_X[2]) && (ppu_Y_RS < BG_Scale_Y[2]) && (ppu_Fetch_Count[2] < 240))
								{
									ppu_BG_Has_Pixel[2] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;

									if (ppu_Fetch_Count[2] == 243)
									{
										ppu_BG_Rendering_Complete[2] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}
							else if ((ppu_Cycle & 3) == 3)
							{
								ppu_Tile_Addr[2] += ppu_BG_Char_Base[2] + (ppu_X_RS & 7) + (ppu_Y_RS & 7) * 8;

								if (ppu_Tile_Addr[2] < 0x10000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_Pixel_Color[2] = VRAM[ppu_Tile_Addr[2]] << 1;

									ppu_Tile_Addr[2] &= 0xFFFE;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[ppu_Tile_Addr[2]] | (VRAM[ppu_Tile_Addr[2] + 1] << 8));
								}
								else
								{
									if ((ppu_Tile_Addr[2] & 1) == 1)
									{
										ppu_Pixel_Color[2] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 7);
									}
									else
									{
										ppu_Pixel_Color[2] = ((ppu_VRAM_Open_Bus & 0xFF) << 1);
									}
								}

								if (ppu_Pixel_Color[2] != 0)
								{
									ppu_BG_Has_Pixel[2] &= true;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;
								}

								if (ppu_Fetch_Count[2] >= 240)
								{
									ppu_BG_Has_Pixel[2] = false;
								}

								ppu_Fetch_Count[2] += 1;
							}
						}
					}

					if (!ppu_BG_Rendering_Complete[3])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[3]))
						{
							if ((ppu_Cycle & 3) == 0)
							{
								cur_x = (ulong)ppu_Fetch_Count[3];

								sol_x = ppu_F_Rot_A_3 * cur_x;
								sol_y = ppu_F_Rot_C_3 * cur_x;

								sol_x += ppu_Current_Ref_X_3;
								sol_y += ppu_Current_Ref_Y_3;

								sol_x >>= 8;
								sol_y >>= 8;

								ppu_X_RS = (ushort)sol_x;
								ppu_Y_RS = (ushort)sol_y;

								// adjust if wraparound is enabled
								if (ppu_BG_Overflow[3])
								{
									ppu_X_RS &= (BG_Scale_X[3] - 1);
									ppu_Y_RS &= (BG_Scale_Y[3] - 1);
								}

								VRAM_ofst_X = ppu_X_RS >> 3;
								VRAM_ofst_Y = ppu_Y_RS >> 3;

								int m2_3_ofst = ppu_BG_Screen_Base[3] + VRAM_ofst_Y * BG_Num_Tiles[3] + VRAM_ofst_X;

								if (m2_3_ofst < 0x10000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_Tile_Addr[3] = VRAM[m2_3_ofst] << 6;

									m2_3_ofst &= 0xFFFE;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[m2_3_ofst] | (VRAM[m2_3_ofst + 1] << 8));
								}
								else
								{
									if ((m2_3_ofst & 1) == 1)
									{
										ppu_Tile_Addr[3] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 2);
									}
									else
									{
										ppu_Tile_Addr[3] = ((ppu_VRAM_Open_Bus & 0xFF) << 6);
									}
								}

								// determine if pixel is in valid range, and pick out color if so
								if ((ppu_X_RS < BG_Scale_X[3]) && (ppu_Y_RS < BG_Scale_Y[3]) && (ppu_Fetch_Count[3] < 240))
								{
									ppu_BG_Has_Pixel[3] = true;
								}
								else
								{
									ppu_BG_Has_Pixel[3] = false;
								}
							}
							else if ((ppu_Cycle & 3) == 1)
							{
								ppu_Tile_Addr[3] += ppu_BG_Char_Base[3] + (ppu_X_RS & 7) + (ppu_Y_RS & 7) * 8;

								if (ppu_Tile_Addr[3] < 0x10000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_Pixel_Color[3] = VRAM[ppu_Tile_Addr[3]] << 1;

									ppu_Tile_Addr[3] &= 0xFFFE;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[ppu_Tile_Addr[3]] | (VRAM[ppu_Tile_Addr[3] + 1] << 8));
								}
								else
								{
									if ((ppu_Tile_Addr[3] & 1) == 1)
									{
										ppu_Pixel_Color[3] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 7);
									}
									else
									{
										ppu_Pixel_Color[3] = ((ppu_VRAM_Open_Bus & 0xFF) << 1);
									}
								}

								if (ppu_Pixel_Color[3] != 0)
								{
									ppu_BG_Has_Pixel[3] &= true;
								}
								else
								{
									ppu_BG_Has_Pixel[3] = false;
								}

								if (ppu_Fetch_Count[3] >= 240)
								{
									ppu_BG_Has_Pixel[3] = false;
								}

								ppu_Fetch_Count[3] += 1;

								if (ppu_Fetch_Count[3] == 244)
								{
									ppu_BG_Rendering_Complete[3] = true;

									ppu_Rendering_Complete = true;

									ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
									ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
								}
							}
						}
					}
					break;

				case 3:
					// Direct Bitmap only, BG2
					if (!ppu_BG_Rendering_Complete[2])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
						{
							if ((ppu_Cycle & 3) == 3)
							{
								// calculate rotation and scaling
								cur_x = (ulong)ppu_Fetch_Count[2];

								sol_x = ppu_F_Rot_A_2 * cur_x;
								sol_y = ppu_F_Rot_C_2 * cur_x;

								sol_x += ppu_Current_Ref_X_2;
								sol_y += ppu_Current_Ref_Y_2;

								sol_x >>= 8;
								sol_y >>= 8;

								ppu_X_RS = (ushort)sol_x;
								ppu_Y_RS = (ushort)sol_y;

								// pixel color comes direct from VRAM
								int m3_ofst = (ppu_X_RS + ppu_Y_RS * 240) * 2;

								if (m3_ofst < 0x14000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_VRAM_Open_Bus = (ushort)(VRAM[m3_ofst] | (VRAM[m3_ofst + 1] << 8));
								}

								ppu_Pixel_Color[2] = ppu_VRAM_Open_Bus;

								if ((ppu_X_RS < 240) && (ppu_Y_RS < 160) && (ppu_Fetch_Count[2] < 240))
								{
									ppu_BG_Has_Pixel[2] = true;

									ppu_Fetch_Count[2] += 1;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;

									ppu_Fetch_Count[2] += 1;

									if (ppu_Fetch_Count[2] == 243)
									{
										ppu_BG_Rendering_Complete[2] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}
						}
					}
					break;

				case 4:
					// bitmaps, only BG2
					if (!ppu_BG_Rendering_Complete[2])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
						{
							if ((ppu_Cycle & 3) == 3)
							{
								// calculate rotation and scaling
								cur_x = (ulong)ppu_Fetch_Count[2];

								sol_x = ppu_F_Rot_A_2 * cur_x;
								sol_y = ppu_F_Rot_C_2 * cur_x;

								sol_x += ppu_Current_Ref_X_2;
								sol_y += ppu_Current_Ref_Y_2;

								sol_x >>= 8;
								sol_y >>= 8;

								ppu_X_RS = (ushort)sol_x;
								ppu_Y_RS = (ushort)sol_y;

								// pixel color comes direct from VRAM
								int m4_ofst = ppu_Display_Frame * 0xA000 + ppu_Y_RS * 240 + ppu_X_RS;

								if (m4_ofst < 0x14000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_Pixel_Color[2] = VRAM[m4_ofst];

									ppu_Pixel_Color[2] <<= 1;

									m4_ofst &= 0x13FFE;

									ppu_VRAM_Open_Bus = (ushort)(VRAM[m4_ofst] | (VRAM[m4_ofst + 1] << 8));
								}
								else
								{
									if ((m4_ofst & 1) == 1)
									{
										ppu_Pixel_Color[2] = (ppu_VRAM_Open_Bus & 0xFF00) >> 7;
									}
									else
									{
										ppu_Pixel_Color[2] = (ppu_VRAM_Open_Bus & 0xFF) << 1;
									}			
								}

								if ((ppu_X_RS < 240) && (ppu_Y_RS < 160) && (ppu_Fetch_Count[2] < 240))
								{
									if (ppu_Pixel_Color[2] != 0)
									{
										ppu_BG_Has_Pixel[2] = true;
									}
									else
									{
										ppu_BG_Has_Pixel[2] = false;
									}

									ppu_Fetch_Count[2] += 1;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;

									ppu_Fetch_Count[2] += 1;
									if (ppu_Fetch_Count[2] == 243)
									{
										ppu_BG_Rendering_Complete[2] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}		
						}
					}
					break;

				case 5:
					// bitmaps, only BG2
					if (!ppu_BG_Rendering_Complete[2])
					{
						if ((ppu_Cycle >= ppu_BG_Start_Time[2]))
						{
							if ((ppu_Cycle & 3) == 3)
							{
								// calculate rotation and scaling
								cur_x = (ulong)ppu_Fetch_Count[2];

								sol_x = ppu_F_Rot_A_2 * cur_x;
								sol_y = ppu_F_Rot_C_2 * cur_x;

								sol_x += ppu_Current_Ref_X_2;
								sol_y += ppu_Current_Ref_Y_2;

								sol_x >>= 8;
								sol_y >>= 8;

								ppu_X_RS = (ushort)sol_x;
								ppu_Y_RS = (ushort)sol_y;

								// pixel color comes direct from VRAM
								int m5_ofst = ppu_Display_Frame * 0xA000 + ppu_X_RS * 2 + ppu_Y_RS * 160 * 2;

								if (m5_ofst < 0x14000)
								{
									ppu_Set_VRAM_Access_True();

									ppu_VRAM_Open_Bus = (ushort)(VRAM[m5_ofst] | (VRAM[m5_ofst + 1] << 8));
								}

								ppu_Pixel_Color[2] = ppu_VRAM_Open_Bus;

								// display split into 2 frames, outside of 160 x 128, display backdrop
								if ((ppu_X_RS < 160) && (ppu_Y_RS < 128) && (ppu_Fetch_Count[2] < 240))
								{				
									ppu_BG_Has_Pixel[2] = true;

									ppu_Fetch_Count[2] += 1;
								}
								else
								{
									ppu_BG_Has_Pixel[2] = false;

									ppu_Fetch_Count[2] += 1;
									if (ppu_Fetch_Count[2] == 243)
									{
										ppu_BG_Rendering_Complete[2] = true;

										ppu_Rendering_Complete = true;

										ppu_Rendering_Complete &= ppu_PAL_Rendering_Complete;
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[0];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[1];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[2];
										ppu_Rendering_Complete &= ppu_BG_Rendering_Complete[3];
									}
								}
							}						
						}
					}
					break;

				case 6:
					// invalid
					break;

				case 7:
					// invalid
					break;
			}
		}

		public void ppu_Set_VRAM_Access_True()
		{
			ppu_VRAM_Access = true;

			if (ppu_VRAM_In_Use)
			{
				cpu_Fetch_Wait += 1;
				dma_Access_Wait += 1;
			}
		}

		public void ppu_Render_Sprites()
		{
			uint pix_color;
			uint pal_scale;

			int spr_tile;
			int spr_tile_row;
			int tile_x_offset;
			int tile_y_offset;

			int actual_x_index, actual_y_index;
			int rel_x_offset;

			bool double_size;

			// local version of other variables used for evaluation
			uint cur_spr_y = 0;

			int spr_size_x_ofst = 0;
			int spr_size_y_ofst = 0;
			int spr_x_pos = 0;
			int spr_y_pos = 0;
			int spr_x_size = 0;
			int spr_y_size = 0;

			ushort spr_attr_0 = 0;
			ushort spr_attr_1 = 0;

			byte ly_check = 0;

			bool rot_scale = false;

			if (ppu_Fetch_Sprite_VRAM)
			{
				// no VRAM access after eval cycle 960 when H blank is free, even though one more OAM access may still occur
				if ((ppu_Sprite_Render_Cycle < 960) || !ppu_HBL_Free)
				{
					ppu_VRAM_High_Access = true;

					if (ppu_VRAM_High_In_Use)
					{
						cpu_Fetch_Wait += 1;
						dma_Access_Wait += 1;
					}

					for (int i = 0; i < 1 + (ppu_Rot_Scale ? 0 : 1); i++)
					{
						ppu_Cur_Sprite_X = (uint)((ppu_Sprite_X_Pos + ppu_Fetch_Sprite_VRAM_Cnt) & 0x1FF);

						rel_x_offset = ppu_Fetch_Sprite_VRAM_Cnt;

						// if sprite is in range horizontally
						if (ppu_Cur_Sprite_X < 240)
						{
							// if the sprite's position is not occupied by a higher priority sprite, or it is a sprite window sprite, process it
							if (!ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] || (ppu_Sprite_Mode == 2) ||
								(((ppu_Sprite_Attr_2 >> 10) & 3) < ppu_Sprite_Priority[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X]))
							{
								spr_tile = ppu_Sprite_Attr_2 & ppu_Sprite_VRAM_Mod;

								// look up the actual pixel to be used in the sprite rotation tables
								actual_x_index = ppu_ROT_OBJ_X[rel_x_offset];
								actual_y_index = ppu_ROT_OBJ_Y[rel_x_offset];

								if ((actual_x_index < ppu_Sprite_X_Size) && (actual_y_index < ppu_Sprite_Y_Size))
								{
									// pick out the tile to use
									if ((ppu_Sprite_Attr_0 & 0x2000) == 0)
									{
										if (ppu_OBJ_Dim)
										{
											spr_tile += (actual_x_index >> 3) + (ppu_Sprite_X_Scale) * (int)(actual_y_index >> 3);
										}
										else
										{
											// large x values wrap around
											spr_tile += (0x20) * (int)(actual_y_index >> 3);

											spr_tile_row = (int)(spr_tile & 0xFFFFFFE0);

											spr_tile += (actual_x_index >> 3);

											spr_tile &= 0x1F;
											spr_tile |= spr_tile_row;
										}

										spr_tile <<= 5;

										// pick out the correct pixel from the tile
										tile_x_offset = actual_x_index & 7;
										tile_y_offset = (int)(actual_y_index & 7);

										spr_tile += (tile_x_offset >> 1) + tile_y_offset * 4;

										spr_tile &= 0x7FFF;

										pix_color = (uint)VRAM[0x10000 + spr_tile];

										if ((tile_x_offset & 1) == 0)
										{
											pix_color &= 0xF;
										}
										else
										{
											pix_color = (pix_color >> 4) & 0xF;
										}

										pix_color *= 2;
										pix_color += (uint)(32 * (ppu_Sprite_Attr_2 >> 12));

										pal_scale = 0x1E;
									}
									else
									{
										spr_tile <<= 5;

										if (ppu_OBJ_Dim)
										{
											spr_tile += ((actual_x_index >> 3) + (ppu_Sprite_X_Scale) * (int)(actual_y_index >> 3)) << 6;
										}
										else
										{
											// large x values wrap around
											spr_tile += ((0x20) * (int)(actual_y_index >> 3) << 5);

											spr_tile_row = (int)(spr_tile & 0xFFFFFC00);

											spr_tile += ((actual_x_index >> 3) << 6);

											spr_tile &= 0x3FF;
											spr_tile |= spr_tile_row;
										}

										// pick out the correct pixel from the tile
										tile_x_offset = (actual_x_index & 7);
										tile_y_offset = (int)(actual_y_index & 7);

										spr_tile += tile_x_offset + tile_y_offset * 8;

										spr_tile &= 0x7FFF;

										pix_color = (uint)(VRAM[0x10000 + spr_tile] << 1);

										pal_scale = 0x1FE;
									}

									// only allow upper half of vram sprite tiles to be used in modes 3-5
									if ((ppu_BG_Mode >= 3) && (spr_tile < 0x4000))
									{
										pix_color = 0;
									}

									// mosaic state is updated even if pixel is transparent
									ppu_Sprite_Is_Mosaic[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = ppu_Sprite_Mosaic;

									if ((pix_color & pal_scale) != 0)
									{
										if (ppu_Sprite_Mode < 2)
										{
											ppu_Sprite_Pixels[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = pix_color + 0x200;

											ppu_Sprite_Priority[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = (ppu_Sprite_Attr_2 >> 10) & 3;

											ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = true;

											ppu_Sprite_Semi_Transparent[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = ppu_Sprite_Mode == 1;
										}
										else if (ppu_Sprite_Mode == 2)
										{
											ppu_Sprite_Object_Window[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = true;
										}
									}
									else
									{
										// glitchy update to priority, even though this does not happen on non-transparent pixels
										if (ppu_Sprite_Mode == 2)
										{
											if (((ppu_Sprite_Attr_2 >> 10) & 3) < ppu_Sprite_Priority[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X])
											{
												ppu_Sprite_Priority[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = (ppu_Sprite_Attr_2 >> 10) & 3;
											}
										}
									}
								}
							}
						}

						ppu_Fetch_Sprite_VRAM_Cnt += 1;
					}
				}
				else
				{
					// still keep track of access count to know when to trigger next OAM fetch
					ppu_Fetch_Sprite_VRAM_Cnt += 1;

					if (!ppu_Rot_Scale) { ppu_Fetch_Sprite_VRAM_Cnt += 1; }
				}

				if (ppu_Fetch_Sprite_VRAM_Cnt == (ppu_Sprite_X_Size + ppu_Sprite_Size_X_Ofst))
				{
					if (ppu_Process_Sprite == 127)
					{
						ppu_Sprite_Eval_Finished = true;
					}
					else if (ppu_Sprite_Next_Fetch == 0)
					{
						ppu_Fetch_OAM_0 = true;
					}
					else
					{
						ppu_Fetch_OAM_2 = true;
					}

					ppu_Fetch_Sprite_VRAM = false;

					ppu_Sprite_Next_Fetch = 3;
				}
			}

			if (ppu_Fetch_OAM_0 && !ppu_Sprite_Eval_Finished)
			{
				ppu_OAM_Access = true;
				ppu_New_Sprite = false;

				spr_attr_0 = (ushort)(OAM[ppu_Current_Sprite * 8] | (OAM[ppu_Current_Sprite * 8 + 1] << 8));
				spr_attr_1 = (ushort)(OAM[ppu_Current_Sprite * 8 + 2] | (OAM[ppu_Current_Sprite * 8 + 3] << 8));

				spr_x_pos = spr_attr_1 & 0x1FF;
				spr_y_pos = spr_attr_0 & 0xFF;

				spr_x_size = ppu_OBJ_Sizes_X[((spr_attr_1 >> 14) & 3) * 4 + ((spr_attr_0 >> 14) & 3)];
				spr_y_size = ppu_OBJ_Sizes_Y[((spr_attr_1 >> 14) & 3) * 4 + ((spr_attr_0 >> 14) & 3)];

				// check if the sprite is in range (in the Y direction)

				// check scaling, only in rotation and scaling mode
				double_size = (spr_attr_0 & 0x200) == 0x200;
				rot_scale = (spr_attr_0 & 0x100) == 0x100;

				spr_size_y_ofst = 0;
				spr_size_x_ofst = 0;

				if (double_size && rot_scale)
				{
					spr_size_y_ofst = spr_y_size;
					spr_size_x_ofst = spr_x_size;
				}

				// NOTE: double_size means disable in the context of not being in rot_scale mode
				if (double_size && !rot_scale)
				{
					// sprite not displayed
					ppu_New_Sprite = true;
				}
				else
				{
					ly_check = ppu_Sprite_LY_Check;

					// account for screen wrapping
					// if the object would appear at the top of the screen, that is the only part that is drawn
					if (spr_y_pos + spr_y_size + spr_size_y_ofst > 0xFF)
					{
						if ((ly_check >= ((spr_y_pos + spr_y_size + spr_size_y_ofst) & 0xFF)))
						{
							// sprite vertically out of range
							ppu_New_Sprite = true;
						}
					}
					else
					{
						if ((ly_check < spr_y_pos) || (ly_check >= ((spr_y_pos + spr_y_size + spr_size_y_ofst) & 0xFF)))
						{
							// sprite vertically out of range
							ppu_New_Sprite = true;
						}
					}

					cur_spr_y = (uint)((ly_check - spr_y_pos) & 0xFF);

					// account for Mosaic
					if (((spr_attr_0 & 0x1000) == 0x1000) && !ppu_New_Sprite)
					{
						ly_check = (byte)ppu_Sprite_Mosaic_Y_Compare;

						// account for screen wrapping
						// if the object would appear at the top of the screen, that is the only part that is drawn
						if (spr_y_pos + spr_y_size + spr_size_y_ofst > 0xFF)
						{
							if ((ly_check >= ((spr_y_pos + spr_y_size + spr_size_y_ofst) & 0xFF)))
							{
								// sprite vertically out of range
								ppu_New_Sprite = true;
							}
						}
						else
						{
							if ((ly_check < spr_y_pos) || (ly_check >= ((spr_y_pos + spr_y_size + spr_size_y_ofst) & 0xFF)))
							{
								// sprite vertically out of range
								ppu_New_Sprite = true;
							}
						}

						if (ppu_New_Sprite)
						{
							ppu_New_Sprite = false;
							cur_spr_y = 0;
						}
						else
						{
							cur_spr_y = (uint)((ly_check - spr_y_pos) & 0xFF);
						}
					}
				}

				// check x range
				if (spr_x_pos >= 240)
				{
					if ((spr_x_pos + spr_x_size + spr_size_x_ofst) <= 512)
					{
						ppu_New_Sprite = true;
					}
				}

				if (ppu_New_Sprite)
				{
					ppu_Current_Sprite += 1;

					if (ppu_Current_Sprite == 128)
					{
						ppu_Fetch_OAM_0 = false;

						if (!ppu_Fetch_Sprite_VRAM && !ppu_Fetch_OAM_A_D)
						{
							ppu_Sprite_Eval_Finished = true;
						}					
					}
					else
					{
						// repeat if not processing another sprite
						if (ppu_Fetch_Sprite_VRAM)
						{
							ppu_Fetch_OAM_0 = false;
							ppu_Sprite_Next_Fetch = 0;
						}
						else if (ppu_Fetch_OAM_A_D)
						{
							ppu_Fetch_OAM_0 = false;
							ppu_Sprite_Next_Fetch = 0;
						}
					}				
				}
				else
				{
					// found a sprite, process it if not accessing VRAM
					ppu_Fetch_OAM_0 = false;

					ppu_Process_Sprite_Temp = ppu_Current_Sprite;

					// send local variables to temp variables, to be loaded with the second access
					ppu_Cur_Sprite_Y_Temp = cur_spr_y;

					ppu_Sprite_Size_X_Ofst_Temp = spr_size_x_ofst;
					ppu_Sprite_Size_Y_Ofst_Temp = spr_size_y_ofst;
					ppu_Sprite_X_Pos_Temp = spr_x_pos;
					ppu_Sprite_Y_Pos_Temp = spr_y_pos;
					ppu_Sprite_X_Size_Temp = spr_x_size;
					ppu_Sprite_Y_Size_Temp = spr_y_size;

					ppu_Sprite_Attr_0_Temp = spr_attr_0;
					ppu_Sprite_Attr_1_Temp = spr_attr_1;

					ppu_Rot_Scale_Temp = rot_scale;

					ppu_Current_Sprite += 1;

					if (ppu_Fetch_Sprite_VRAM)
					{
						ppu_Sprite_Next_Fetch = 2;
					}
					else if (ppu_Fetch_OAM_A_D)
					{
						ppu_Sprite_Next_Fetch = 2;
					}
					else
					{
						ppu_Fetch_OAM_2 = true;
					}
				}
			}
			else if (ppu_Fetch_OAM_2 && !ppu_Sprite_Eval_Finished)
			{			
				ppu_OAM_Access = true;
				ppu_Fetch_OAM_2 = false;

				ppu_Process_Sprite = ppu_Process_Sprite_Temp;

				ppu_Sprite_Attr_2 = (ushort)(OAM[ppu_Process_Sprite * 8 + 4] | (OAM[ppu_Process_Sprite * 8 + 5] << 8));

				// send temp variables to rendering variables
				ppu_Cur_Sprite_Y = ppu_Cur_Sprite_Y_Temp;

				ppu_Sprite_Size_X_Ofst = ppu_Sprite_Size_X_Ofst_Temp;
				ppu_Sprite_Size_Y_Ofst = ppu_Sprite_Size_Y_Ofst_Temp;
				ppu_Sprite_X_Pos = ppu_Sprite_X_Pos_Temp;
				ppu_Sprite_Y_Pos = ppu_Sprite_Y_Pos_Temp;
				ppu_Sprite_X_Size = ppu_Sprite_X_Size_Temp;
				ppu_Sprite_Y_Size = ppu_Sprite_Y_Size_Temp;

				ppu_Sprite_Attr_0 = ppu_Sprite_Attr_0_Temp;
				ppu_Sprite_Attr_1 = ppu_Sprite_Attr_1_Temp;

				ppu_Rot_Scale = ppu_Rot_Scale_Temp;

				if (ppu_Rot_Scale)
				{
					ppu_Fetch_OAM_A_D = true;
					ppu_Fetch_OAM_A_D_Cnt = 0;
				}
				else
				{
					ppu_Fetch_Sprite_VRAM = true;
					
					if (ppu_Current_Sprite < 128) { ppu_Fetch_OAM_0 = true; }

					ppu_Sprite_VRAM_Cnt_Reset();

					// scan through the properties of this sprite on this scanline
					ppu_Do_Sprite_Calculation();
				}
			}

			if (ppu_Fetch_OAM_A_D)
			{
				// TODO: access A-D here, note that we skip the first one since it immediately runs from the above line
				if (ppu_Fetch_OAM_A_D_Cnt == 0)
				{
					ppu_Param_Pick = (ppu_Sprite_Attr_1 >> 9) & 0x1F;
				}
				else if (ppu_Fetch_OAM_A_D_Cnt == 1)
				{
					ppu_Sprite_A_Latch = (ushort)(OAM[0x06 + 0x20 * ppu_Param_Pick] + (OAM[0x06 + 0x20 * ppu_Param_Pick + 1] << 8));

					ppu_OAM_Access = true;
				}
				else if (ppu_Fetch_OAM_A_D_Cnt == 2)
				{
					ppu_Sprite_B_Latch = (ushort)(OAM[0x0E + 0x20 * ppu_Param_Pick] + (OAM[0x0E + 0x20 * ppu_Param_Pick + 1] << 8));
					
					ppu_OAM_Access = true;
				}
				else if (ppu_Fetch_OAM_A_D_Cnt == 3)
				{
					ppu_Sprite_C_Latch = (ushort)(OAM[0x16 + 0x20 * ppu_Param_Pick] + (OAM[0x16 + 0x20 * ppu_Param_Pick + 1] << 8));

					ppu_OAM_Access = true;
				}
				else if (ppu_Fetch_OAM_A_D_Cnt == 4)
				{
					ppu_Sprite_D_Latch = (ushort)(OAM[0x1E + 0x20 * ppu_Param_Pick] + (OAM[0x1E + 0x20 * ppu_Param_Pick + 1] << 8));

					ppu_OAM_Access = true;

					// next cycle will start evaluation of next sprite
					if (ppu_Current_Sprite < 128) { ppu_Fetch_OAM_0 = true; }

					// scan through the properties of this sprite on this scanline
					ppu_Do_Sprite_Calculation_Rot();
				}

				ppu_Fetch_OAM_A_D_Cnt += 1;
				
				// 5 here, extra cycle for processing
				if (ppu_Fetch_OAM_A_D_Cnt == 6)
				{
					ppu_Fetch_OAM_A_D = false;

					ppu_Fetch_Sprite_VRAM = true;

					ppu_Sprite_VRAM_Cnt_Reset();
				}
			}

			ppu_Sprite_Render_Cycle += 2;
		}

		public void ppu_Sprite_VRAM_Cnt_Reset()
		{
			ppu_Fetch_Sprite_VRAM_Cnt = 0;

			ppu_Sprite_X_Scale = ppu_Sprite_X_Size >> 3;

			ppu_Sprite_Mode = (ppu_Sprite_Attr_0 >> 10) & 3;

			// GBA tek says lower bit of tile number should be ignored in some cases, but it appears this is not the case?
			// more testing needed
			if ((ppu_Sprite_Attr_0 & 0x2000) == 0)
			{
				ppu_Sprite_VRAM_Mod = 0x3FF;
			}
			else
			{
				ppu_Sprite_VRAM_Mod = 0x3FF;
			}

			ppu_Sprite_Mosaic = (ppu_Sprite_Attr_0 & 0x1000) == 0x1000;

			// clip left side of sprite if needed
			// note if the sprite is completely clipped it is skipped altogether
			// so this case only comes up if there is definitely something to clip
			if (ppu_Sprite_X_Pos >= 240)
			{
				ppu_Fetch_Sprite_VRAM_Cnt = 512 - ppu_Sprite_X_Pos;

				if (!ppu_Rot_Scale) { ppu_Fetch_Sprite_VRAM_Cnt &= 0xFFFE; }
			}
		}

		public void ppu_Do_Sprite_Calculation_Rot()
		{
			int i_A, i_B, i_C, i_D;

			uint A, B, C, D;

			double f_A, f_B, f_C, f_D;

			double cur_x, cur_y;
			double sol_x, sol_y;

			A = ppu_Sprite_A_Latch;
			B = ppu_Sprite_B_Latch;
			C = ppu_Sprite_C_Latch;
			D = ppu_Sprite_D_Latch;

			i_A = (int)((A >> 8) & 0x7F);
			i_B = (int)((B >> 8) & 0x7F);
			i_C = (int)((C >> 8) & 0x7F);
			i_D = (int)((D >> 8) & 0x7F);

			if ((A & 0x8000) == 0x8000) { i_A |= unchecked((int)0xFFFFFF80); }
			if ((B & 0x8000) == 0x8000) { i_B |= unchecked((int)0xFFFFFF80); }
			if ((C & 0x8000) == 0x8000) { i_C |= unchecked((int)0xFFFFFF80); }
			if ((D & 0x8000) == 0x8000) { i_D |= unchecked((int)0xFFFFFF80); }

			// convert to floats
			f_A = i_A;
			f_B = i_B;
			f_C = i_C;
			f_D = i_D;

			f_A += ppu_Fract_Parts[A & 0xFF];
			f_B += ppu_Fract_Parts[B & 0xFF];
			f_C += ppu_Fract_Parts[C & 0xFF];
			f_D += ppu_Fract_Parts[D & 0xFF];

			if (((ppu_Sprite_Attr_0 >> 9) & 0x1) == 1)
			{
				for (int j = 0; j < 2 * ppu_Sprite_X_Size; j++)
				{
					cur_x = j - ppu_Sprite_X_Size;

					cur_y = ppu_Cur_Sprite_Y - ppu_Sprite_Y_Size;

					sol_x = f_A * cur_x + f_B * cur_y;
					sol_y = f_C * cur_x + f_D * cur_y;

					sol_x += ppu_Sprite_X_Size >> 1;
					sol_y += ppu_Sprite_Y_Size >> 1;

					sol_x = Math.Floor(sol_x);
					sol_y = Math.Floor(sol_y);

					ppu_ROT_OBJ_X[j] = (ushort)(sol_x);
					ppu_ROT_OBJ_Y[j] = (ushort)(sol_y);
				}
			}
			else
			{
				for (int j = 0; j < ppu_Sprite_X_Size; j++)
				{
					cur_x = j - (ppu_Sprite_X_Size >> 1);

					cur_y = ppu_Cur_Sprite_Y - (ppu_Sprite_Y_Size >> 1);

					sol_x = f_A * cur_x + f_B * cur_y;
					sol_y = f_C * cur_x + f_D * cur_y;

					sol_x += ppu_Sprite_X_Size >> 1;
					sol_y += ppu_Sprite_Y_Size >> 1;

					sol_x = Math.Floor(sol_x);
					sol_y = Math.Floor(sol_y);

					ppu_ROT_OBJ_X[j] = (ushort)(sol_x);
					ppu_ROT_OBJ_Y[j] = (ushort)(sol_y);
				}
			}
		}

		public void ppu_Do_Sprite_Calculation()
		{
			bool h_flip, v_flip;

			double sol_x, sol_y;

			h_flip = ((ppu_Sprite_Attr_1 & 0x1000) == 0x1000);
			v_flip = ((ppu_Sprite_Attr_1 & 0x2000) == 0x2000);

			for (int j = 0; j < ppu_Sprite_X_Size; j++)
			{
				// horizontal flip
				if (h_flip)
				{
					sol_x = ppu_Sprite_X_Size - 1 - j;
				}
				else
				{
					sol_x = j;
				}

				// vertical flip
				if (v_flip)
				{
					sol_y = ppu_Sprite_Y_Size - 1 - ppu_Cur_Sprite_Y;
				}
				else
				{
					sol_y = ppu_Cur_Sprite_Y;
				}

				ppu_ROT_OBJ_X[j] = (ushort)sol_x;
				ppu_ROT_OBJ_Y[j] = (ushort)sol_y;
			}
		}

		public void ppu_Convert_Rotation_to_ulong_AC(int layer)
		{
			if (layer == 2)
			{
				ppu_F_Rot_A_2 = ppu_BG_Rot_A[2];
				ppu_F_Rot_C_2 = ppu_BG_Rot_C[2];

				if ((ppu_F_Rot_A_2 & 0x8000) != 0) { ppu_F_Rot_A_2 |= 0xFFFFFFFFFFFF0000; }
				if ((ppu_F_Rot_C_2 & 0x8000) != 0) { ppu_F_Rot_C_2 |= 0xFFFFFFFFFFFF0000; }
			}
			else
			{
				ppu_F_Rot_A_3 = ppu_BG_Rot_A[3];
				ppu_F_Rot_C_3 = ppu_BG_Rot_C[3];

				if ((ppu_F_Rot_A_3 & 0x8000) != 0) { ppu_F_Rot_A_3 |= 0xFFFFFFFFFFFF0000; }
				if ((ppu_F_Rot_C_3 & 0x8000) != 0) { ppu_F_Rot_C_3 |= 0xFFFFFFFFFFFF0000; }
			}
		}

		public void ppu_Reset()
		{			
			ppu_X_RS = ppu_Y_RS = 0;
			
			ppu_BG_Ref_X[2] = ppu_BG_Ref_X[3] = 0;
			ppu_BG_Ref_Y[2] = ppu_BG_Ref_Y[3] = 0;

			ppu_ROT_REF_LY[2] = ppu_ROT_REF_LY[3] = 0;

			ppu_CTRL = ppu_Green_Swap = 0;

			ppu_VBL_IRQ_cd = ppu_HBL_IRQ_cd = ppu_LYC_IRQ_cd = ppu_Sprite_cd = 0;

			ppu_LYC_Vid_Check_cd = 0;

			for (int i = 0; i < 4; i++)
			{
				ppu_BG_CTRL[i] = 0;
				ppu_BG_X[0] = 0;
				ppu_BG_Y[0] = 0;

				ppu_BG_X_Latch[0] = 0;
				ppu_BG_Y_Latch[0] = 0;

				ppu_BG_On_Update_Time[i] = 0;

				ppu_BG_Ref_X_Change[i] = false;
				ppu_BG_Ref_LY_Change[i] = false;
			}

			ppu_Forced_Blank_Time = ppu_OBJ_On_Time = 0;

			ppu_BG_Rot_A[2] = ppu_BG_Rot_B[2] = ppu_BG_Rot_C[2] = ppu_BG_Rot_D[2] = 0;

			ppu_BG_Rot_A[3] = ppu_BG_Rot_B[3] = ppu_BG_Rot_C[3] = ppu_BG_Rot_D[3] = 0;

			ppu_WIN_Hor_0 = ppu_WIN_Hor_1 = ppu_WIN_Vert_0 = ppu_WIN_Vert_1 = 0;

			ppu_WIN_In = ppu_WIN_Out = ppu_Mosaic = ppu_Special_FX = ppu_Alpha = ppu_Bright = 0;

			ppu_STAT = 0;

			ppu_LYC = 0xFF;

			// based on music4.gba, initial state would either be Ly = 225 or 161.
			// based on console verification testing, it seems 225 is correct.
			ppu_LY = 225;

			// 1 gives the correct value in music4.gba
			ppu_Cycle = 1;

			ppu_Display_Cycle = 0;

			ppu_In_VBlank = true;
			ppu_Delays = false;
			ppu_Sprite_Delays = false;
			ppu_Do_Green_Swap = false;

			ppu_WIN0_Active  = ppu_WIN1_Active = false;

			// reset sprite evaluation variables
			ppu_Current_Sprite = 0;
			ppu_New_Sprite = true;
			ppu_Sprite_Eval_Finished = false;

			for (int i = 0; i < 240 * 2; i++) 
			{ 
				ppu_Sprite_Pixels[i] = 0;
				ppu_Sprite_Priority[i] = 3;
				ppu_Sprite_Pixel_Occupied[i] = false;
				ppu_Sprite_Semi_Transparent[i] = false;
				ppu_Sprite_Object_Window[i] = false;
				ppu_Sprite_Is_Mosaic[i] = false;
			}

			ppu_Cur_Sprite_X = ppu_Cur_Sprite_Y = ppu_Cur_Sprite_Y_Temp = 0;

			ppu_Current_Sprite = ppu_Process_Sprite = ppu_Process_Sprite_Temp = 0;

			ppu_Sprite_ofst_eval = 0;
			ppu_Sprite_ofst_draw = 240;

			ppu_Sprite_X_Pos = ppu_Sprite_Y_Pos = 0;
			ppu_Sprite_X_Pos_Temp = ppu_Sprite_Y_Pos_Temp = 0;
			ppu_Sprite_X_Size = ppu_Sprite_Y_Size = 0;
			ppu_Sprite_X_Size_Temp = ppu_Sprite_Y_Size_Temp = 0;
			ppu_Sprite_Render_Cycle = 0;
			ppu_Fetch_OAM_A_D_Cnt = ppu_Fetch_Sprite_VRAM_Cnt = ppu_Sprite_VRAM_Mod = 0;
			ppu_Sprite_X_Scale = 0;
			ppu_Sprite_Size_X_Ofst = ppu_Sprite_Size_Y_Ofst = 0;
			ppu_Sprite_Size_X_Ofst_Temp = ppu_Sprite_Size_Y_Ofst_Temp = 0;
			ppu_Sprite_Mode = ppu_Sprite_Next_Fetch = 0;
			ppu_Param_Pick = 0;
			ppu_Sprite_Mosaic_Y_Counter = ppu_Sprite_Mosaic_Y_Compare = 0;

			ppu_Sprite_Attr_0 = ppu_Sprite_Attr_1 = ppu_Sprite_Attr_2 = 0;
			ppu_Sprite_Attr_0_Temp = ppu_Sprite_Attr_1_Temp = 0;

			ppu_Sprite_LY_Check = 0;

			ppu_Rot_Scale = ppu_Rot_Scale_Temp = false;
			ppu_Fetch_OAM_0 = ppu_Fetch_OAM_2 = ppu_Fetch_OAM_A_D = false;
			ppu_Fetch_Sprite_VRAM = ppu_New_Sprite = ppu_Sprite_Eval_Finished = false;
			ppu_Sprite_Mosaic = false;

			ppu_VRAM_High_Access = false;
			ppu_VRAM_Access = false;
			ppu_PALRAM_Access = false;
			ppu_OAM_Access = false;

			ppu_VRAM_In_Use = ppu_VRAM_High_In_Use = ppu_PALRAM_In_Use = false;

			ppu_Sprite_A_Latch = ppu_Sprite_B_Latch = ppu_Sprite_C_Latch= ppu_Sprite_D_Latch = 0;


			ppu_Sprite_Pixel_Latch = 0;
			ppu_Sprite_Priority_Latch = 0;

			ppu_Sprite_Semi_Transparent_Latch = false;
			ppu_Sprite_Mosaic_Latch = false;
			ppu_Sprite_Pixel_Occupied_Latch = false;

			// BG rendering
			for (int i = 0; i < 4; i++)
			{
				ppu_Fetch_Count[i] = 0;
				ppu_Scroll_Cycle[i] = 0;
				ppu_Pixel_Color[i] = 0;
				ppu_Pixel_Color_2[i] = 0;
				ppu_Pixel_Color_1[i] = 0;
				ppu_Pixel_Color_M[i] = 0;
				ppu_Pixel_Color_R[i] = 0;
				ppu_BG_Start_Time[i] = 0;
				ppu_BG_Effect_Byte[i] = 0;
				ppu_BG_Effect_Byte_New[i] = 0;
				ppu_Tile_Addr[i] = 0;
				ppu_Y_Flip_Ofst[i] = 0;

				ppu_BG_Rendering_Complete[i] = false;
				ppu_BG_Has_Pixel[i] = false;
				ppu_BG_Has_Pixel_2[i] = false;
				ppu_BG_Has_Pixel_1[i] = false;
				ppu_BG_Has_Pixel_M[i] = false;
				ppu_BG_Has_Pixel_R[i] = false;
			}

			ppu_Base_LY_2 = ppu_Base_LY_3 = 0;

			ppu_BG_Pixel_F = 0;
			ppu_BG_Pixel_S = 0;
			ppu_Final_Pixel = 0;
			ppu_Blend_Pixel = 0;

			ppu_BG_Mosaic_X_Mod = 1;

			ppu_Brighten_Final_Pixel = false;
			ppu_Blend_Final_Pixel = false;

			ppu_Fetch_BG = true;

			ppu_Fetch_Target_1 = false;
			ppu_Fetch_Target_2 = false;

			ppu_Rendering_Complete = false;
			ppu_PAL_Rendering_Complete = false;

			ppu_VRAM_Open_Bus =  0;

			// PPU power up
			ppu_CTRL_Write(0);

			// update derived values
			ppu_Calc_Win0();
			ppu_Calc_Win1();
			ppu_Update_Win_In(0);
			ppu_Update_Win_Out(0);
			ppu_BG_CTRL_Write(0);
			ppu_BG_CTRL_Write(1);
			ppu_BG_CTRL_Write(2);
			ppu_BG_CTRL_Write(3);
			ppu_Update_Special_FX(0);
			ppu_Update_Alpha(0);
			ppu_Update_Bright(0);
			ppu_Update_Mosaic(0);

			ppu_Convert_Rotation_to_ulong_AC(2);
			ppu_Convert_Rotation_to_ulong_AC(3);

			double fract = 0.5;
			double f_v = 0;

			// compile fractional parts matrix
			for (int i = 0; i < 256; i++)
			{
				f_v = 0;
				fract = 0.5;

				for (int j = 7; j >= 0; j--)
				{
					if ((i & (1 << j)) == (1 << j)) { f_v += fract; }

					fract *= 0.5;
				}

				ppu_Fract_Parts[i] = f_v;
			}
		}

		public void ppu_SyncState(Serializer ser)
		{
			ser.Sync(nameof(ppu_BG_CTRL), ref ppu_BG_CTRL, false);
			ser.Sync(nameof(ppu_BG_X), ref ppu_BG_X, false);
			ser.Sync(nameof(ppu_BG_Y), ref ppu_BG_Y, false);
			ser.Sync(nameof(ppu_BG_X_Latch), ref ppu_BG_X_Latch, false);
			ser.Sync(nameof(ppu_BG_Y_Latch), ref ppu_BG_Y_Latch, false);

			ser.Sync(nameof(ppu_BG_Ref_X), ref ppu_BG_Ref_X, false);
			ser.Sync(nameof(ppu_BG_Ref_Y), ref ppu_BG_Ref_Y, false);

			ser.Sync(nameof(ppu_ROT_REF_LY), ref ppu_ROT_REF_LY, false);

			ser.Sync(nameof(ppu_BG_On_Update_Time), ref ppu_BG_On_Update_Time, false);

			ser.Sync(nameof(ppu_BG_Rot_A), ref ppu_BG_Rot_A, false);
			ser.Sync(nameof(ppu_BG_Rot_B), ref ppu_BG_Rot_B, false);
			ser.Sync(nameof(ppu_BG_Rot_C), ref ppu_BG_Rot_C, false);
			ser.Sync(nameof(ppu_BG_Rot_D), ref ppu_BG_Rot_D, false);

			ser.Sync(nameof(ppu_BG_On), ref ppu_BG_On, false);
			ser.Sync(nameof(ppu_BG_On_New), ref ppu_BG_On_New, false);
			ser.Sync(nameof(ppu_BG_Ref_X_Change), ref ppu_BG_Ref_X_Change, false);
			ser.Sync(nameof(ppu_BG_Ref_LY_Change), ref ppu_BG_Ref_LY_Change, false);

			ser.Sync(nameof(ppu_BG_Mode), ref ppu_BG_Mode);
			ser.Sync(nameof(ppu_Display_Frame), ref ppu_Display_Frame);

			ser.Sync(nameof(ppu_X_RS), ref ppu_X_RS);
			ser.Sync(nameof(ppu_Y_RS), ref ppu_Y_RS);

			ser.Sync(nameof(ppu_Forced_Blank_Time), ref ppu_Forced_Blank_Time);
			ser.Sync(nameof(ppu_OBJ_On_Time), ref ppu_OBJ_On_Time);

			ser.Sync(nameof(ppu_VBL_IRQ_cd), ref ppu_VBL_IRQ_cd);
			ser.Sync(nameof(ppu_HBL_IRQ_cd), ref ppu_HBL_IRQ_cd);
			ser.Sync(nameof(ppu_LYC_IRQ_cd), ref ppu_LYC_IRQ_cd);
			ser.Sync(nameof(ppu_Sprite_cd), ref ppu_Sprite_cd);

			ser.Sync(nameof(ppu_LYC_Vid_Check_cd), ref ppu_LYC_Vid_Check_cd);

			ser.Sync(nameof(ppu_CTRL), ref ppu_CTRL);
			ser.Sync(nameof(ppu_Green_Swap), ref ppu_Green_Swap);
			ser.Sync(nameof(ppu_Cycle), ref ppu_Cycle);
			ser.Sync(nameof(ppu_Display_Cycle), ref ppu_Display_Cycle);
			ser.Sync(nameof(ppu_Sprite_Eval_Time), ref ppu_Sprite_Eval_Time);

			ser.Sync(nameof(ppu_WIN_Hor_0), ref ppu_WIN_Hor_0);
			ser.Sync(nameof(ppu_WIN_Hor_1), ref ppu_WIN_Hor_1);
			ser.Sync(nameof(ppu_WIN_Vert_0), ref ppu_WIN_Vert_0);
			ser.Sync(nameof(ppu_WIN_Vert_1), ref ppu_WIN_Vert_1);

			ser.Sync(nameof(ppu_WIN_In), ref ppu_WIN_In);
			ser.Sync(nameof(ppu_WIN_Out), ref ppu_WIN_Out);
			ser.Sync(nameof(ppu_Mosaic), ref ppu_Mosaic);
			ser.Sync(nameof(ppu_Special_FX), ref ppu_Special_FX);
			ser.Sync(nameof(ppu_Alpha), ref ppu_Alpha);
			ser.Sync(nameof(ppu_Bright), ref ppu_Bright);

			ser.Sync(nameof(ppu_STAT), ref ppu_STAT);
			ser.Sync(nameof(ppu_LYC), ref ppu_LYC);
			ser.Sync(nameof(ppu_LY), ref ppu_LY);

			ser.Sync(nameof(ppu_HBL_Free), ref ppu_HBL_Free);
			ser.Sync(nameof(ppu_OBJ_Dim), ref ppu_OBJ_Dim);
			ser.Sync(nameof(ppu_Forced_Blank), ref ppu_Forced_Blank);
			ser.Sync(nameof(ppu_Any_Window_On), ref ppu_Any_Window_On);
			ser.Sync(nameof(ppu_OBJ_On), ref ppu_OBJ_On);
			ser.Sync(nameof(ppu_WIN0_On), ref ppu_WIN0_On);
			ser.Sync(nameof(ppu_WIN1_On), ref ppu_WIN1_On);
			ser.Sync(nameof(ppu_OBJ_WIN), ref ppu_OBJ_WIN);
			ser.Sync(nameof(ppu_WIN0_Active), ref ppu_WIN0_Active);
			ser.Sync(nameof(ppu_WIN1_Active), ref ppu_WIN1_Active);

			ser.Sync(nameof(ppu_In_VBlank), ref ppu_In_VBlank);
			ser.Sync(nameof(ppu_Delays), ref ppu_Delays);
			ser.Sync(nameof(ppu_Sprite_Delays), ref ppu_Sprite_Delays);
			ser.Sync(nameof(ppu_Do_Green_Swap), ref ppu_Do_Green_Swap);

			ser.Sync(nameof(ppu_VRAM_In_Use), ref ppu_VRAM_In_Use);
			ser.Sync(nameof(ppu_VRAM_High_In_Use), ref ppu_VRAM_High_In_Use);
			ser.Sync(nameof(ppu_PALRAM_In_Use), ref ppu_PALRAM_In_Use);

			ser.Sync(nameof(ppu_VRAM_High_Access), ref ppu_VRAM_High_Access);
			ser.Sync(nameof(ppu_VRAM_Access), ref ppu_VRAM_Access);
			ser.Sync(nameof(ppu_PALRAM_Access), ref ppu_PALRAM_Access);
			ser.Sync(nameof(ppu_OAM_Access), ref ppu_OAM_Access);

			// Sprite Evaluation
			ser.Sync(nameof(ppu_Sprite_Pixels), ref ppu_Sprite_Pixels, false);
			ser.Sync(nameof(ppu_Sprite_Priority), ref ppu_Sprite_Priority, false);
			ser.Sync(nameof(ppu_Sprite_Pixel_Occupied), ref ppu_Sprite_Pixel_Occupied, false);
			ser.Sync(nameof(ppu_Sprite_Semi_Transparent), ref ppu_Sprite_Semi_Transparent, false);
			ser.Sync(nameof(ppu_Sprite_Object_Window), ref ppu_Sprite_Object_Window, false);
			ser.Sync(nameof(ppu_Sprite_Is_Mosaic), ref ppu_Sprite_Is_Mosaic, false);

			ser.Sync(nameof(ppu_Cur_Sprite_X), ref ppu_Cur_Sprite_X);
			ser.Sync(nameof(ppu_Cur_Sprite_Y), ref ppu_Cur_Sprite_Y);
			ser.Sync(nameof(ppu_Cur_Sprite_Y_Temp), ref ppu_Cur_Sprite_Y_Temp);		

			ser.Sync(nameof(ppu_Current_Sprite), ref ppu_Current_Sprite);
			ser.Sync(nameof(ppu_Process_Sprite), ref ppu_Process_Sprite);
			ser.Sync(nameof(ppu_Process_Sprite_Temp), ref ppu_Process_Sprite_Temp);
			ser.Sync(nameof(ppu_Sprite_ofst_eval), ref ppu_Sprite_ofst_eval);
			ser.Sync(nameof(ppu_Sprite_ofst_draw), ref ppu_Sprite_ofst_draw);
			ser.Sync(nameof(ppu_Sprite_X_Pos), ref ppu_Sprite_X_Pos);
			ser.Sync(nameof(ppu_Sprite_Y_Pos), ref ppu_Sprite_Y_Pos);
			ser.Sync(nameof(ppu_Sprite_X_Pos_Temp), ref ppu_Sprite_X_Pos_Temp);
			ser.Sync(nameof(ppu_Sprite_Y_Pos_Temp), ref ppu_Sprite_Y_Pos_Temp);
			ser.Sync(nameof(ppu_Sprite_X_Size), ref ppu_Sprite_X_Size);
			ser.Sync(nameof(ppu_Sprite_Y_Size), ref ppu_Sprite_Y_Size);
			ser.Sync(nameof(ppu_Sprite_X_Size_Temp), ref ppu_Sprite_X_Size_Temp);
			ser.Sync(nameof(ppu_Sprite_Y_Size_Temp), ref ppu_Sprite_Y_Size_Temp);
			ser.Sync(nameof(ppu_Sprite_Render_Cycle), ref ppu_Sprite_Render_Cycle);
			ser.Sync(nameof(ppu_Fetch_OAM_A_D_Cnt), ref ppu_Fetch_OAM_A_D_Cnt);
			ser.Sync(nameof(ppu_Fetch_Sprite_VRAM_Cnt), ref ppu_Fetch_Sprite_VRAM_Cnt);
			ser.Sync(nameof(ppu_Sprite_VRAM_Mod), ref ppu_Sprite_VRAM_Mod);
			ser.Sync(nameof(ppu_Sprite_X_Scale), ref ppu_Sprite_X_Scale);
			ser.Sync(nameof(ppu_Sprite_Size_X_Ofst), ref ppu_Sprite_Size_X_Ofst);
			ser.Sync(nameof(ppu_Sprite_Size_Y_Ofst), ref ppu_Sprite_Size_Y_Ofst);
			ser.Sync(nameof(ppu_Sprite_Size_X_Ofst_Temp), ref ppu_Sprite_Size_X_Ofst_Temp);
			ser.Sync(nameof(ppu_Sprite_Size_Y_Ofst_Temp), ref ppu_Sprite_Size_Y_Ofst_Temp);
			ser.Sync(nameof(ppu_Sprite_Mode), ref ppu_Sprite_Mode);
			ser.Sync(nameof(ppu_Sprite_Next_Fetch), ref ppu_Sprite_Next_Fetch);
			ser.Sync(nameof(ppu_Param_Pick), ref ppu_Param_Pick);
			ser.Sync(nameof(ppu_Sprite_Mosaic_Y_Counter), ref ppu_Sprite_Mosaic_Y_Counter);
			ser.Sync(nameof(ppu_Sprite_Mosaic_Y_Compare), ref ppu_Sprite_Mosaic_Y_Compare);
			ser.Sync(nameof(ppu_Sprite_Attr_0), ref ppu_Sprite_Attr_0);
			ser.Sync(nameof(ppu_Sprite_Attr_1), ref ppu_Sprite_Attr_1);
			ser.Sync(nameof(ppu_Sprite_Attr_2), ref ppu_Sprite_Attr_2);
			ser.Sync(nameof(ppu_Sprite_Attr_0_Temp), ref ppu_Sprite_Attr_0_Temp);
			ser.Sync(nameof(ppu_Sprite_Attr_1_Temp), ref ppu_Sprite_Attr_1_Temp);

			ser.Sync(nameof(ppu_Sprite_LY_Check), ref ppu_Sprite_LY_Check);

			ser.Sync(nameof(ppu_Sprite_A_Latch), ref ppu_Sprite_A_Latch);
			ser.Sync(nameof(ppu_Sprite_B_Latch), ref ppu_Sprite_B_Latch);
			ser.Sync(nameof(ppu_Sprite_C_Latch), ref ppu_Sprite_C_Latch);
			ser.Sync(nameof(ppu_Sprite_D_Latch), ref ppu_Sprite_D_Latch);

			ser.Sync(nameof(ppu_Rot_Scale), ref ppu_Rot_Scale);
			ser.Sync(nameof(ppu_Rot_Scale_Temp), ref ppu_Rot_Scale_Temp);
			ser.Sync(nameof(ppu_Fetch_OAM_0), ref ppu_Fetch_OAM_0);
			ser.Sync(nameof(ppu_Fetch_OAM_2), ref ppu_Fetch_OAM_2);
			ser.Sync(nameof(ppu_Fetch_OAM_A_D), ref ppu_Fetch_OAM_A_D);
			ser.Sync(nameof(ppu_Fetch_Sprite_VRAM), ref ppu_Fetch_Sprite_VRAM);
			ser.Sync(nameof(ppu_New_Sprite), ref ppu_New_Sprite);
			ser.Sync(nameof(ppu_Sprite_Eval_Finished), ref ppu_Sprite_Eval_Finished);
			ser.Sync(nameof(ppu_Sprite_Mosaic), ref ppu_Sprite_Mosaic);

			// sprite latches
			ser.Sync(nameof(ppu_Sprite_Pixel_Latch), ref ppu_Sprite_Pixel_Latch);

			ser.Sync(nameof(ppu_Sprite_Priority_Latch), ref ppu_Sprite_Priority_Latch);

			ser.Sync(nameof(ppu_Sprite_Semi_Transparent_Latch), ref ppu_Sprite_Semi_Transparent_Latch);
			ser.Sync(nameof(ppu_Sprite_Mosaic_Latch), ref ppu_Sprite_Mosaic_Latch);
			ser.Sync(nameof(ppu_Sprite_Pixel_Occupied_Latch), ref ppu_Sprite_Pixel_Occupied_Latch);

			// BG rendering
			ser.Sync(nameof(ppu_Fetch_Count), ref ppu_Fetch_Count, false);
			ser.Sync(nameof(ppu_Scroll_Cycle), ref ppu_Scroll_Cycle, false);

			ser.Sync(nameof(ppu_Pixel_Color), ref ppu_Pixel_Color, false);
			ser.Sync(nameof(ppu_Pixel_Color_2), ref ppu_Pixel_Color_2, false);
			ser.Sync(nameof(ppu_Pixel_Color_1), ref ppu_Pixel_Color_1, false);
			ser.Sync(nameof(ppu_Pixel_Color_M), ref ppu_Pixel_Color_M, false);
			ser.Sync(nameof(ppu_Pixel_Color_R), ref ppu_Pixel_Color_R, false);
			ser.Sync(nameof(ppu_Tile_Addr), ref ppu_Tile_Addr, false);
			ser.Sync(nameof(ppu_Y_Flip_Ofst), ref ppu_Y_Flip_Ofst, false);

			ser.Sync(nameof(ppu_BG_Start_Time), ref ppu_BG_Start_Time, false);

			ser.Sync(nameof(ppu_BG_Effect_Byte), ref ppu_BG_Effect_Byte, false);
			ser.Sync(nameof(ppu_BG_Effect_Byte_New), ref ppu_BG_Effect_Byte_New, false);

			ser.Sync(nameof(ppu_BG_Rendering_Complete), ref ppu_BG_Rendering_Complete, false);

			ser.Sync(nameof(ppu_BG_Has_Pixel), ref ppu_BG_Has_Pixel, false);
			ser.Sync(nameof(ppu_BG_Has_Pixel_2), ref ppu_BG_Has_Pixel_2, false);
			ser.Sync(nameof(ppu_BG_Has_Pixel_1), ref ppu_BG_Has_Pixel_1, false);
			ser.Sync(nameof(ppu_BG_Has_Pixel_M), ref ppu_BG_Has_Pixel_M, false);
			ser.Sync(nameof(ppu_BG_Has_Pixel_R), ref ppu_BG_Has_Pixel_R, false);

			ser.Sync(nameof(ppu_BG_Pixel_F), ref ppu_BG_Pixel_F);
			ser.Sync(nameof(ppu_BG_Pixel_S), ref ppu_BG_Pixel_S);
			ser.Sync(nameof(ppu_Final_Pixel), ref ppu_Final_Pixel);
			ser.Sync(nameof(ppu_Blend_Pixel), ref ppu_Blend_Pixel);

			ser.Sync(nameof(ppu_BG_Mosaic_X_Mod), ref ppu_BG_Mosaic_X_Mod);

			ser.Sync(nameof(ppu_Brighten_Final_Pixel), ref ppu_Brighten_Final_Pixel);
			ser.Sync(nameof(ppu_Blend_Final_Pixel), ref ppu_Blend_Final_Pixel);

			ser.Sync(nameof(ppu_Fetch_BG), ref ppu_Fetch_BG);

			ser.Sync(nameof(ppu_Fetch_Target_1), ref ppu_Fetch_Target_1);
			ser.Sync(nameof(ppu_Fetch_Target_2), ref ppu_Fetch_Target_2);

			ser.Sync(nameof(ppu_Rendering_Complete), ref ppu_Rendering_Complete);
			ser.Sync(nameof(ppu_PAL_Rendering_Complete), ref ppu_PAL_Rendering_Complete);

			ser.Sync(nameof(ppu_VRAM_Open_Bus), ref ppu_VRAM_Open_Bus);

			ser.Sync(nameof(ppu_Base_LY_2), ref ppu_Base_LY_2);
			ser.Sync(nameof(ppu_Base_LY_3), ref ppu_Base_LY_3);

			ser.Sync(nameof(ppu_Current_Ref_X_2), ref ppu_Current_Ref_X_2);
			ser.Sync(nameof(ppu_Current_Ref_Y_2), ref ppu_Current_Ref_Y_2);
			ser.Sync(nameof(ppu_Current_Ref_X_3), ref ppu_Current_Ref_X_3);
			ser.Sync(nameof(ppu_Current_Ref_Y_3), ref ppu_Current_Ref_Y_3);


			// update derived values
			ppu_Calc_Win0();
			ppu_Calc_Win1();
			ppu_Update_Win_In(ppu_WIN_In);
			ppu_Update_Win_Out(ppu_WIN_Out);
			ppu_BG_CTRL_Write(0);
			ppu_BG_CTRL_Write(1);
			ppu_BG_CTRL_Write(2);
			ppu_BG_CTRL_Write(3);
			ppu_Update_Special_FX(ppu_Special_FX);
			ppu_Update_Alpha(ppu_Alpha);
			ppu_Update_Bright(ppu_Bright);
			ppu_Update_Mosaic(ppu_Mosaic);

			ppu_Convert_Rotation_to_ulong_AC(2);
			ppu_Convert_Rotation_to_ulong_AC(3);
		}
	}

#pragma warning restore CS0675 // Bitwise-or operator used on a sign-extended operand
}
