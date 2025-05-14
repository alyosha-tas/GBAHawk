#pragma once

#include <cstdint>
#include <iomanip>
#include <string>

#include "GBA_System.h"

// Note:

/*	Enabling sprites through ppu ctrl starts evaluation immediately. However it takes an additional scanline for them to start displaying.
*   It seems sprite evaluation always runs either way, but when sprites are not enabled they are declared invalid after the first oam read
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
*/

namespace GBAHawk
{
	void GBA_System::ppu_Reset()
	{
		ppu_X_RS = ppu_Y_RS = 0;

		ppu_BG_Ref_X[2] = ppu_BG_Ref_X[3] = 0;
		ppu_BG_Ref_Y[2] = ppu_BG_Ref_Y[3] = 0;

		ppu_CTRL = ppu_Green_Swap = 0;

		ppu_VBL_IRQ_cd = ppu_HBL_IRQ_cd = ppu_LYC_IRQ_cd = ppu_Sprite_cd = ppu_Sprite_Disp_cd = 0;

		ppu_LYC_Vid_Check_cd = 0;

		for (int i = 0; i < 4; i++)
		{
			ppu_BG_CTRL[i] = 0;
			ppu_BG_X[i] = 0;
			ppu_BG_Y[i] = 0;

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
		ppu_Sprite_Delay_SL = false;
		ppu_Sprite_Delay_Disp = false;

		// reset sprite evaluation variables
		ppu_Current_Sprite = 0;
		ppu_New_Sprite = true;
		ppu_Sprite_Eval_Finished = false;
		ppu_Do_Green_Swap = false;

		ppu_WIN0_Active = ppu_WIN1_Active = false;

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

		ppu_Sprite_LY_Check = 0;

		ppu_Sprite_Attr_0 = ppu_Sprite_Attr_1 = ppu_Sprite_Attr_2 = 0;
		ppu_Sprite_Attr_0_Temp = ppu_Sprite_Attr_1_Temp = 0;

		ppu_Rot_Scale = ppu_Rot_Scale_Temp = false;
		ppu_Fetch_OAM_0 = ppu_Fetch_OAM_2 = ppu_Fetch_OAM_A_D = false;
		ppu_Fetch_Sprite_VRAM = ppu_New_Sprite = false;
		ppu_Sprite_Mosaic = false;

		ppu_VRAM_High_Access = false;
		ppu_VRAM_Access = false;
		ppu_PALRAM_Access = false;
		ppu_OAM_Access = false;

		ppu_VRAM_In_Use = ppu_VRAM_High_In_Use = ppu_PALRAM_In_Use = false;

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

		ppu_VRAM_Open_Bus = 0;

		// PPU power up
		ppu_CTRL_Write(0);

		ppu_OBJ_On_Disp = false;

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
	}

	void GBA_System::ppu_Render()
	{
		uint64_t cur_x;
		uint64_t sol_x, sol_y;

		uint32_t R, G, B;
		uint32_t R2, G2, B2;

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

		uint32_t spr_pixel = 0;
		uint32_t spr_priority = 0;
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

								ppu_Pixel_Color_1[c0] = temp_color;
								ppu_BG_Has_Pixel_1[c0] = ((temp_color & 0xF) != 0);
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

								ppu_Pixel_Color_1[c1] = temp_color;
								ppu_BG_Has_Pixel_1[c1] = ((temp_color & 0xF) != 0);
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
				if (((ppu_Cycle - 2) & 3) == 0)
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

							OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch && ppu_OBJ_On_Disp;

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
								OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch && ppu_OBJ_On_Disp;
								spr_pixel = ppu_Sprite_Pixel_Latch;
								spr_priority = ppu_Sprite_Priority_Latch;
								spr_semi_transparent = ppu_Sprite_Semi_Transparent_Latch;
							}
							else
							{
								ppu_Sprite_Pixel_Occupied_Latch = ppu_Sprite_Pixel_Occupied[ppu_Sprite_ofst_draw + ppu_Display_Cycle];

								OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch && ppu_OBJ_On_Disp;

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

						OBJ_Has_Pixel = ppu_Sprite_Pixel_Occupied_Latch && ppu_OBJ_On_Disp;

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

							OBJ_Go = ppu_WIN0_OBJ_En && OBJ_Has_Pixel;
							Color_FX_Go = ppu_WIN0_Color_En;
						}
						else if (ppu_WIN1_On && (((ppu_Display_Cycle - ppu_WIN1_Left) & 0xFF) < ((ppu_WIN1_Right - ppu_WIN1_Left) & 0xFF)) &&
							ppu_WIN1_Active)
						{
							Is_Outside = false;

							for (int w1 = 0; w1 < 4; w1++) { BG_Go_Disp[w1] &= ppu_WIN1_BG_En[w1]; }

							OBJ_Go = ppu_WIN1_OBJ_En && OBJ_Has_Pixel;
							Color_FX_Go = ppu_WIN1_Color_En;
						}
						else if (ppu_OBJ_WIN && ppu_Sprite_Object_Window[ppu_Sprite_ofst_draw + ppu_Display_Cycle])
						{
							Is_Outside = false;

							for (int ob = 0; ob < 4; ob++) { BG_Go_Disp[ob] &= ppu_OBJ_BG_En[ob]; }

							OBJ_Go = ppu_OBJ_OBJ_En && OBJ_Has_Pixel;
							Color_FX_Go = ppu_OBJ_Color_En;
						}

						if (Is_Outside)
						{
							for (int outs = 0; outs < 4; outs++) { BG_Go_Disp[outs] &= ppu_OUT_BG_En[outs]; }

							OBJ_Go = ppu_OUT_OBJ_En && OBJ_Has_Pixel;
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

										ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[c0];
										cur_BG_layer = c0;
										cur_layer_priority = ppu_BG_Priority[c0];
									}
									else if (ppu_BG_Priority[c0] < second_layer_priority)
									{
										ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[c0];
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

										ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[c1];
										cur_BG_layer = c1;
										cur_layer_priority = ppu_BG_Priority[c1];
									}
									else if (ppu_BG_Priority[c1] < second_layer_priority)
									{
										ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[c1];
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

								ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
								cur_BG_layer = 2;
								cur_layer_priority = ppu_BG_Priority[2];
							}
							else if (ppu_BG_Priority[2] < second_layer_priority)
							{
								ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[2];
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

									ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[c2];
									cur_BG_layer = c2;
									cur_layer_priority = ppu_BG_Priority[c2];
								}
								else if (ppu_BG_Priority[c2] < second_layer_priority)
								{
									ppu_BG_Pixel_S = (uint32_t)ppu_Pixel_Color_R[c2];
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

							ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
						}
						break;

					case 4:
						// bitmaps, only BG2
						if (BG_Go_Disp[2] && ppu_BG_Has_Pixel_R[2])
						{
							// no transparency possible
							cur_BG_layer = 2;
							cur_layer_priority = ppu_BG_Priority[2];
							ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
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

							ppu_BG_Pixel_F = (uint32_t)ppu_Pixel_Color_R[2];
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

						ppu_Final_Pixel = (uint32_t)(PALRAM_16[ppu_Final_Pixel]);

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

						ppu_Blend_Pixel = (uint32_t)(PALRAM_16[ppu_Blend_Pixel]);

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
							R = (uint32_t)(R + (((31 - R) * ppu_SFX_BRT_Num) >> 4));
							G = (uint32_t)(G + (((31 - G) * ppu_SFX_BRT_Num) >> 4));
							B = (uint32_t)(B + (((31 - B) * ppu_SFX_BRT_Num) >> 4));
						}
						else
						{
							R = (uint32_t)(R - ((R * ppu_SFX_BRT_Num) >> 4));
							G = (uint32_t)(G - ((G * ppu_SFX_BRT_Num) >> 4));
							B = (uint32_t)(B - ((B * ppu_SFX_BRT_Num) >> 4));
						}

						ppu_Final_Pixel = (uint32_t)(0xFF000000 |
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

						R = (uint32_t)(((R * ppu_SFX_Alpha_Num_1) >> 4) + ((R2 * ppu_SFX_Alpha_Num_2) >> 4));
						G = (uint32_t)(((G * ppu_SFX_Alpha_Num_1) >> 4) + ((G2 * ppu_SFX_Alpha_Num_2) >> 4));
						B = (uint32_t)(((B * ppu_SFX_Alpha_Num_1) >> 4) + ((B2 * ppu_SFX_Alpha_Num_2) >> 4));

						if (R > 31) { R = 31; }
						if (G > 31) { G = 31; }
						if (B > 31) { B = 31; }

						ppu_Final_Pixel = (uint32_t)(0xFF000000 |
							(R << 3) |
							(G << 11) |
							(B << 19));
					}
					else
					{
						ppu_Final_Pixel = (uint32_t)(0xFF000000 |
							((ppu_Final_Pixel & 0x1F) << 19) |
							((ppu_Final_Pixel & 0x3E0) << 6) |
							((ppu_Final_Pixel & 0x7C00) >> 7));
					}

					// push pixel to display
					video_buffer[ppu_Display_Cycle + ppu_LY * 240] = ppu_Final_Pixel;

					if (ppu_Do_Green_Swap)
					{
						if ((ppu_Display_Cycle & 1) == 1)
						{
							uint32_t temp_pixel = video_buffer[ppu_Display_Cycle - 1 + ppu_LY * 240];

							uint32_t temp_pixel_2 = temp_pixel;

							temp_pixel &= 0xFFFF00FF;

							temp_pixel |= ppu_Final_Pixel & 0x0000FF00;

							ppu_Final_Pixel &= 0xFFFF00FF;
							ppu_Final_Pixel |= temp_pixel_2 & 0x0000FF00;

							video_buffer[ppu_Display_Cycle + ppu_LY * 240] = ppu_Final_Pixel;
							video_buffer[ppu_Display_Cycle - 1 + ppu_LY * 240] = temp_pixel;
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
							ppu_Y_RS = (int)((ppu_MOS_BG_Y[ppu_LY] + ppu_BG_Y[a0]) & 0x1FF);
						}
						else
						{
							ppu_X_RS = (int)(((ppu_Fetch_Count[a0] << 3) + ppu_BG_X_Latch[a0]) & 0x1FF);
							ppu_Y_RS = (int)((ppu_LY + ppu_BG_Y[a0]) & 0x1FF);
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

						ppu_VRAM_Open_Bus = VRAM_16[ppu_Tile_Addr[a0] >> 1];

						ppu_BG_Effect_Byte_New[a0] = (uint8_t)(ppu_VRAM_Open_Bus >> 8);

						ppu_Tile_Addr[a0] = (uint16_t)(ppu_VRAM_Open_Bus & 0x3FF);
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

								ppu_VRAM_Open_Bus = VRAM_16[temp_addr >> 1];
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

								ppu_VRAM_Open_Bus = VRAM_16[temp_addr >> 1];
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

								ppu_VRAM_Open_Bus = VRAM_16[temp_addr >> 1];
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
								ppu_Y_RS = (int)((ppu_MOS_BG_Y[ppu_LY] + ppu_BG_Y[a1]) & 0x1FF);
							}
							else
							{
								ppu_X_RS = (int)(((ppu_Fetch_Count[a1] << 3) + ppu_BG_X_Latch[a1]) & 0x1FF);
								ppu_Y_RS = (int)((ppu_LY + ppu_BG_Y[a1]) & 0x1FF);
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

							ppu_VRAM_Open_Bus = VRAM_16[ppu_Tile_Addr[a1] >> 1];

							ppu_BG_Effect_Byte_New[a1] = (uint8_t)(ppu_VRAM_Open_Bus >> 8);

							ppu_Tile_Addr[a1] = (uint16_t)(ppu_VRAM_Open_Bus & 0x3FF);
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

									ppu_VRAM_Open_Bus = VRAM_16[temp_addr >> 1];
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

									ppu_VRAM_Open_Bus = VRAM_16[temp_addr >> 1];
								}

								ppu_Pixel_Color[a1] = ppu_VRAM_Open_Bus;
							}
						}
						else if (((ppu_Scroll_Cycle[a1] & 31) == 12) || ((ppu_Scroll_Cycle[a1] & 31) == 28))
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

									ppu_VRAM_Open_Bus = VRAM_16[temp_addr >> 1];
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
						cur_x = (uint64_t)ppu_Fetch_Count[2];

						sol_x = ppu_F_Rot_A_2 * cur_x;
						sol_y = ppu_F_Rot_C_2 * cur_x;

						sol_x += ppu_Current_Ref_X_2;
						sol_y += ppu_Current_Ref_Y_2;

						sol_x >>= 8;
						sol_y >>= 8;

						ppu_X_RS = (uint16_t)sol_x;
						ppu_Y_RS = (uint16_t)sol_y;

						// adjust if wraparound is enabled
						if (ppu_BG_Overflow[2])
						{
							ppu_X_RS &= (BG_Scale_X[2] - 1);
							ppu_Y_RS &= (BG_Scale_Y[2] - 1);
						}

						VRAM_ofst_X = ppu_X_RS >> 3;
						VRAM_ofst_Y = ppu_Y_RS >> 3;

						uint32_t m1_2_ofst = ppu_BG_Screen_Base[2] + VRAM_ofst_Y * BG_Num_Tiles[2] + VRAM_ofst_X;

						if (m1_2_ofst < 0x10000)
						{
							ppu_Set_VRAM_Access_True();

							ppu_Tile_Addr[2] = ((uint32_t)VRAM[m1_2_ofst] << 6);

							m1_2_ofst &= 0xFFFE;

							ppu_VRAM_Open_Bus = VRAM_16[m1_2_ofst >> 1];
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

							ppu_Pixel_Color[2] = VRAM[ppu_Tile_Addr[2]];

							ppu_Tile_Addr[2] &= 0xFFFE;

							ppu_VRAM_Open_Bus = VRAM_16[ppu_Tile_Addr[2] >> 1];
						}
						else
						{
							if ((ppu_Tile_Addr[2] & 1) == 1)
							{
								ppu_Pixel_Color[2] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 8);
							}
							else
							{
								ppu_Pixel_Color[2] = ppu_VRAM_Open_Bus & 0xFF;
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
						cur_x = (uint64_t)ppu_Fetch_Count[2];

						sol_x = ppu_F_Rot_A_2 * cur_x;
						sol_y = ppu_F_Rot_C_2 * cur_x;

						sol_x += ppu_Current_Ref_X_2;
						sol_y += ppu_Current_Ref_Y_2;

						sol_x >>= 8;
						sol_y >>= 8;

						ppu_X_RS = (uint16_t)sol_x;
						ppu_Y_RS = (uint16_t)sol_y;

						// adjust if wraparound is enabled
						if (ppu_BG_Overflow[2])
						{
							ppu_X_RS &= (BG_Scale_X[2] - 1);
							ppu_Y_RS &= (BG_Scale_Y[2] - 1);
						}

						VRAM_ofst_X = ppu_X_RS >> 3;
						VRAM_ofst_Y = ppu_Y_RS >> 3;

						uint32_t m2_2_ofst = ppu_BG_Screen_Base[2] + VRAM_ofst_Y * BG_Num_Tiles[2] + VRAM_ofst_X;

						if (m2_2_ofst < 0x10000)
						{
							ppu_Set_VRAM_Access_True();

							ppu_Tile_Addr[2] = ((uint32_t)VRAM[m2_2_ofst] << 6);

							m2_2_ofst &= 0xFFFE;

							ppu_VRAM_Open_Bus = VRAM_16[m2_2_ofst >> 1];
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

							ppu_Pixel_Color[2] = VRAM[ppu_Tile_Addr[2]];

							ppu_Tile_Addr[2] &= 0xFFFE;

							ppu_VRAM_Open_Bus = VRAM_16[ppu_Tile_Addr[2] >> 1];
						}
						else
						{
							if ((ppu_Tile_Addr[2] & 1) == 1)
							{
								ppu_Pixel_Color[2] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 8);
							}
							else
							{
								ppu_Pixel_Color[2] = ppu_VRAM_Open_Bus & 0xFF;
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
						// calculate rotation and scaling
						cur_x = (uint64_t)ppu_Fetch_Count[3];

						sol_x = ppu_F_Rot_A_3 * cur_x;
						sol_y = ppu_F_Rot_C_3 * cur_x;

						sol_x += ppu_Current_Ref_X_3;
						sol_y += ppu_Current_Ref_Y_3;

						sol_x >>= 8;
						sol_y >>= 8;

						ppu_X_RS = (uint16_t)sol_x;
						ppu_Y_RS = (uint16_t)sol_y;

						// adjust if wraparound is enabled
						if (ppu_BG_Overflow[3])
						{
							ppu_X_RS &= (BG_Scale_X[3] - 1);
							ppu_Y_RS &= (BG_Scale_Y[3] - 1);
						}

						VRAM_ofst_X = ppu_X_RS >> 3;
						VRAM_ofst_Y = ppu_Y_RS >> 3;

						uint32_t m2_3_ofst = ppu_BG_Screen_Base[3] + VRAM_ofst_Y * BG_Num_Tiles[3] + VRAM_ofst_X;

						if (m2_3_ofst < 0x10000)
						{
							ppu_Set_VRAM_Access_True();

							ppu_Tile_Addr[3] = ((uint32_t)VRAM[m2_3_ofst] << 6);

							m2_3_ofst &= 0xFFFE;

							ppu_VRAM_Open_Bus = VRAM_16[m2_3_ofst >> 1];
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

							ppu_Pixel_Color[3] = VRAM[ppu_Tile_Addr[3]];

							ppu_Tile_Addr[3] &= 0xFFFE;

							ppu_VRAM_Open_Bus = VRAM_16[ppu_Tile_Addr[3] >> 1];
						}
						else
						{
							if ((ppu_Tile_Addr[3] & 1) == 1)
							{
								ppu_Pixel_Color[3] = ((ppu_VRAM_Open_Bus & 0xFF00) >> 8);
							}
							else
							{
								ppu_Pixel_Color[3] = ppu_VRAM_Open_Bus & 0xFF;
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
						cur_x = (uint64_t)ppu_Fetch_Count[2];

						sol_x = ppu_F_Rot_A_2 * cur_x;
						sol_y = ppu_F_Rot_C_2 * cur_x;

						sol_x += ppu_Current_Ref_X_2;
						sol_y += ppu_Current_Ref_Y_2;

						sol_x >>= 8;
						sol_y >>= 8;

						ppu_X_RS = (uint16_t)sol_x;
						ppu_Y_RS = (uint16_t)sol_y;

						// pixel color comes direct from VRAM
						uint32_t m3_ofst = (ppu_X_RS + ppu_Y_RS * 240) * 2;

						if (m3_ofst < 0x14000)
						{
							ppu_Set_VRAM_Access_True();

							ppu_VRAM_Open_Bus = VRAM_16[m3_ofst >> 1];
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
						cur_x = (uint64_t)ppu_Fetch_Count[2];

						sol_x = ppu_F_Rot_A_2 * cur_x;
						sol_y = ppu_F_Rot_C_2 * cur_x;

						sol_x += ppu_Current_Ref_X_2;
						sol_y += ppu_Current_Ref_Y_2;

						sol_x >>= 8;
						sol_y >>= 8;

						ppu_X_RS = (uint16_t)sol_x;
						ppu_Y_RS = (uint16_t)sol_y;

						// pixel color comes direct from VRAM
						uint32_t m4_ofst = ppu_Display_Frame * 0xA000 + ppu_Y_RS * 240 + ppu_X_RS;

						if (m4_ofst < 0x14000)
						{
							ppu_Set_VRAM_Access_True();

							ppu_Pixel_Color[2] = VRAM[m4_ofst];

							m4_ofst &= 0x13FFE;

							ppu_VRAM_Open_Bus = VRAM_16[m4_ofst >> 1];
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
						cur_x = (uint64_t)ppu_Fetch_Count[2];

						sol_x = ppu_F_Rot_A_2 * cur_x;
						sol_y = ppu_F_Rot_C_2 * cur_x;

						sol_x += ppu_Current_Ref_X_2;
						sol_y += ppu_Current_Ref_Y_2;

						sol_x >>= 8;
						sol_y >>= 8;

						ppu_X_RS = (uint16_t)sol_x;
						ppu_Y_RS = (uint16_t)sol_y;

						// pixel color comes direct from VRAM
						uint32_t m5_ofst = ppu_Display_Frame * 0xA000 + ppu_X_RS * 2 + ppu_Y_RS * 160 * 2;

						if (m5_ofst < 0x14000)
						{
							ppu_Set_VRAM_Access_True();

							ppu_VRAM_Open_Bus = VRAM_16[m5_ofst >> 1];
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

	void GBA_System::ppu_Render_Sprites()
	{
		uint32_t pix_color;
		uint32_t pal_scale;

		uint32_t spr_tile;
		uint32_t spr_tile_row;
		uint32_t tile_x_offset;
		uint32_t tile_y_offset;

		uint32_t actual_x_index, actual_y_index;
		uint32_t rel_x_offset, rel_y_offset;

		bool double_size;

		// local version of other variables used for evaluation
		uint32_t cur_spr_y = 0;

		uint32_t spr_size_x_ofst = 0;
		uint32_t spr_size_y_ofst = 0;
		uint32_t spr_x_pos = 0;
		uint32_t spr_y_pos = 0;
		uint32_t spr_x_size = 0;
		uint32_t spr_y_size = 0;

		uint16_t spr_attr_0 = 0;
		uint16_t spr_attr_1 = 0;

		uint8_t ly_check = 0;

		bool rot_scale = false;

		if (ppu_Fetch_Sprite_VRAM)
		{
			// no VRAM access after eval cycle 960 when H blank is free, even though one more OAM access amy still occur
			if (ppu_Sprite_Render_Cycle <= ppu_Sprite_Eval_Time_VRAM)
			{
				ppu_VRAM_High_Access = true;

				if (ppu_VRAM_High_In_Use)
				{
					cpu_Fetch_Wait += 1;
					dma_Access_Wait += 1;
				}

				for (int i = 0; i < 1 + (ppu_Rot_Scale ? 0 : 1); i++)
				{
					ppu_Cur_Sprite_X = (uint32_t)((ppu_Sprite_X_Pos + ppu_Fetch_Sprite_VRAM_Cnt) & 0x1FF);

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

									pix_color = (uint32_t)VRAM[0x10000 + spr_tile];

									if ((tile_x_offset & 1) == 0)
									{
										pix_color &= 0xF;
									}
									else
									{
										pix_color = (pix_color >> 4) & 0xF;
									}

									pix_color += (uint32_t)(16 * (ppu_Sprite_Attr_2 >> 12));

									pal_scale = 0xF;
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

									pix_color = (uint32_t)VRAM[0x10000 + spr_tile];

									pal_scale = 0xFF;
								}

								// sprite info not added on last possible cycle
								// presumably this happens on the odd cycle that vram is not being read on
								// and that is the cycle that processing is shut down
								if (ppu_Sprite_Render_Cycle != ppu_Sprite_Eval_Time_VRAM)
								{
									// only allow upper half of vram sprite tiles to be used in modes 3-5
									if ((ppu_BG_Mode >= 3) && (spr_tile < 0x4000))
									{
										pix_color = 0;
									}

									// mosaic state is updated even if pixel is transparent
									ppu_Sprite_Is_Mosaic[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = ppu_Sprite_Mosaic;

									if ((pix_color & pal_scale) != 0)
									{
										if (ppu_Sprite_Mode != 2)
										{
											ppu_Sprite_Pixels[ppu_Sprite_ofst_eval + ppu_Cur_Sprite_X] = pix_color + 0x100;

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
			if (ppu_OBJ_On)
			{
				ppu_OAM_Access = true;
				spr_attr_0 = OAM_16[ppu_Current_Sprite * 4];
				spr_attr_1 = OAM_16[ppu_Current_Sprite * 4 + 1];
			}
			else
			{
				ppu_OAM_Access = false;
				spr_attr_0 = 0xFFFF;
				spr_attr_1 = 0xFFFF;
			}
			
			ppu_New_Sprite = false;	

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

				cur_spr_y = (uint32_t)((ly_check - spr_y_pos) & 0xFF);

				// account for Mosaic
				if (((spr_attr_0 & 0x1000) == 0x1000) && !ppu_New_Sprite)
				{
					ly_check = (uint8_t)ppu_Sprite_Mosaic_Y_Compare;

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
						cur_spr_y = (uint32_t)((ly_check - spr_y_pos) & 0xFF);
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

			if (!ppu_OBJ_On)
			{
				ppu_New_Sprite = true;
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

			ppu_Sprite_Attr_2 = OAM_16[ppu_Process_Sprite * 4 + 2];

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
				ppu_Sprite_A_Latch = OAM_16[0x03 + 0x10 * ppu_Param_Pick];

				ppu_OAM_Access = true;
			}
			else if (ppu_Fetch_OAM_A_D_Cnt == 2)
			{
				ppu_Sprite_B_Latch = OAM_16[0x07 + 0x10 * ppu_Param_Pick];

				ppu_OAM_Access = true;
			}
			else if (ppu_Fetch_OAM_A_D_Cnt == 3)
			{
				ppu_Sprite_C_Latch = OAM_16[0x0B + 0x10 * ppu_Param_Pick];

				ppu_OAM_Access = true;
			}
			else if (ppu_Fetch_OAM_A_D_Cnt == 4)
			{
				ppu_Sprite_D_Latch = OAM_16[0xF + 0x10 * ppu_Param_Pick];

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
}