#ifndef SNES_System_H
#define SNES_System_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>
#include <cstring>
#include <inttypes.h>
#include <cmath>

#include "../Common/Savestate.h"
#include "../Common/Common_Ops.h"

#ifndef _WIN32
#define sprintf_s snprintf
#endif

using namespace std;

// Notes:

/*
*	
* 
*/

//Message_String = "Uop " + to_string((int)uop) + " cyc: " + to_string(TotalExecutedCycles);

//MessageCallback(Message_String.length());

namespace SNESHawk
{
	class Mappers;
	
	class SNES_System
	{
	public:
		
		Mappers* mapper_pntr = nullptr;

		bool* CPU_NMI = nullptr;
		bool* CPU_IRQ = nullptr;
		
		// Various sync settings and mapper specific behavior and revision dependent behavior
		bool Is_Lo_ROM = false;
		uint32_t APU_Frequency = 0;
		uint32_t PPU_H_Pos_Reset = 0;
		uint32_t PPU_V_Pos_Reset = 0;
		uint32_t DRAM_Refresh_Pos = 0;

		uint8_t* Cart_RAM = nullptr;
		uint8_t* ROM = nullptr;
		uint8_t* CHR_ROM = nullptr;
		uint32_t Cart_RAM_Length = 0;
		string Message_String = "";

		uint32_t ROM_Length;
		uint32_t ROM_Speed;

		uint32_t Mapper_Number = 0;

		void (*MessageCallback)(int);
		void (*InputPollCallback)();

		uint8_t(*ReadController)(bool);
		void(*StrobeController)(uint8_t, uint8_t);

		uint8_t ReadMemory(uint32_t);

		uint8_t PeekMemory(uint32_t);

		void WriteMemory(uint32_t, uint8_t);

	# pragma region General System

		uint32_t RAM_Start_Up[256] =  { 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255,
										0, 255, 0, 255, 255, 0, 255, 0, 0, 255, 0, 255, 255, 0, 255, 0 };
		
		uint32_t video_buffer[256 * 240] = { };

		//user configuration 
		uint32_t Compiled_Palette[512] = { };

		// 

		void Frame_Advance();

		bool SubFrame_Advance(uint32_t reset_cycle);

		// this function will run one step of the ppu 
		// it will return whether the controller is read or not.
		inline void Single_Step();

		void HardReset();

		uint8_t ReadMemoryDMA(uint32_t addr);

		uint16_t Peek_Memory_16(uint32_t addr);

		uint16_t Peek_Memory_8_Branch(uint32_t addr);

		uint8_t read_joyport(uint32_t addr);

		void write_joyport(uint8_t value);

		uint8_t peek_joyport(uint32_t addr);

		uint8_t PeekReg(uint32_t addr);

		uint8_t ReadReg(uint32_t addr);

		void WriteReg(uint32_t addr, uint8_t value);

		bool Is_Lag;
		bool _irq_apu;
		bool frame_is_done;
		bool Settings_DispBackground = true;
		bool Settings_DispSprites = true;
		bool DMC_DMA_Exec;
		bool Controller_Strobed = false;

		// these variables are for subframe input control
		bool controller_was_latched;
		bool current_strobe;
		bool new_strobe;
		bool alt_lag;
		bool OAM_DMA_Exec = false;
		bool dmc_realign;

		uint8_t Data_Bus; //old data bus values from previous reads
		uint8_t oam_dma_byte;
		uint8_t latched4016;
		uint8_t Controller_Strobed_Value;
		uint8_t Previous_Controller_Latch_1;
		uint8_t Previous_Controller_Latch_2;

		uint16_t oam_dma_addr;

		uint32_t sprdma_countdown;
		uint32_t cpu_deadcounter;
		uint32_t oam_dma_index;

		int32_t old_s = 0;

		// General Variables
		uint64_t FrameCycle;
		uint64_t Clock_Update_Cycle;
		uint64_t Cycle_Count;
		uint64_t Last_Controller_Poll_1;
		uint64_t Last_Controller_Poll_2;

		//hardware/state
		uint8_t RAM[0x20000] = { };
		uint8_t VRAM[0x10000] = { };
		uint8_t Header[0x40] = { };

		SNES_System()
		{
			HardReset();
		}

		void On_VBlank()
		{
			// things to do on vblank
		}

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(Is_Lag, saver);
			saver = bool_saver(_irq_apu, saver);
			saver = bool_saver(frame_is_done, saver);
			saver = bool_saver(Settings_DispBackground, saver);
			saver = bool_saver(Settings_DispSprites, saver);
			saver = bool_saver(DMC_DMA_Exec, saver);
			saver = bool_saver(Controller_Strobed, saver);

			saver = bool_saver(controller_was_latched, saver);
			saver = bool_saver(current_strobe, saver);
			saver = bool_saver(new_strobe, saver);
			saver = bool_saver(alt_lag, saver);
			saver = bool_saver(OAM_DMA_Exec, saver);
			saver = bool_saver(dmc_realign, saver);

			saver = byte_saver(Data_Bus, saver);
			saver = byte_saver(oam_dma_byte, saver);
			saver = byte_saver(latched4016, saver);
			saver = byte_saver(Controller_Strobed_Value, saver);
			saver = byte_saver(Previous_Controller_Latch_1, saver);
			saver = byte_saver(Previous_Controller_Latch_2, saver);

			saver = short_saver(oam_dma_addr, saver);

			saver = int_saver(sprdma_countdown, saver);
			saver = int_saver(cpu_deadcounter, saver);
			saver = int_saver(oam_dma_index, saver);

			saver = int_saver(old_s, saver);

			saver = long_saver(FrameCycle, saver);
			saver = long_saver(Clock_Update_Cycle, saver);
			saver = long_saver(Cycle_Count, saver);
			saver = long_saver(Last_Controller_Poll_1, saver);
			saver = long_saver(Last_Controller_Poll_2, saver);
			
			saver = byte_array_saver(RAM, saver, 0x20000);
			saver = byte_array_saver(VRAM, saver, 0x10000);

			if (Cart_RAM_Length != 0)
			{
				saver = byte_array_saver(Cart_RAM, saver, Cart_RAM_Length);
			}

			saver = ppu_SaveState(saver);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&Is_Lag, loader);
			loader = bool_loader(&_irq_apu, loader);
			loader = bool_loader(&frame_is_done, loader);
			loader = bool_loader(&Settings_DispBackground, loader);
			loader = bool_loader(&Settings_DispSprites, loader);
			loader = bool_loader(&DMC_DMA_Exec, loader);
			loader = bool_loader(&Controller_Strobed, loader);

			loader = bool_loader(&controller_was_latched, loader);
			loader = bool_loader(&current_strobe, loader);
			loader = bool_loader(&new_strobe, loader);
			loader = bool_loader(&alt_lag, loader);
			loader = bool_loader(&OAM_DMA_Exec, loader);
			loader = bool_loader(&dmc_realign, loader);

			loader = byte_loader(&Data_Bus, loader);
			loader = byte_loader(&oam_dma_byte, loader);
			loader = byte_loader(&latched4016, loader);
			loader = byte_loader(&Controller_Strobed_Value, loader);
			loader = byte_loader(&Previous_Controller_Latch_1, loader);
			loader = byte_loader(&Previous_Controller_Latch_2, loader);

			loader = short_loader(&oam_dma_addr, loader);

			loader = int_loader(&sprdma_countdown, loader);
			loader = int_loader(&cpu_deadcounter, loader);
			loader = int_loader(&oam_dma_index, loader);

			loader = sint_loader(&old_s, loader);

			loader = long_loader(&FrameCycle, loader);
			loader = long_loader(&Clock_Update_Cycle, loader);
			loader = long_loader(&Cycle_Count, loader);
			loader = long_loader(&Last_Controller_Poll_1, loader);
			loader = long_loader(&Last_Controller_Poll_2, loader);

			loader = byte_array_loader(RAM, loader, 0x20000);
			loader = byte_array_loader(VRAM, loader, 0x10000);

			if (Cart_RAM_Length != 0)
			{
				loader = byte_array_loader(Cart_RAM, loader, Cart_RAM_Length);
			}

			loader = ppu_LoadState(loader);

			return loader;
		}

	#pragma endregion

	#pragma region PPU
		
		//blargg: Reading from $2007 when the VRAM address is $3fxx will fill the internal read buffer with the contents at VRAM address $3fxx, in addition to reading the palette RAM. 

		//static const uint8_t powerUpPalette[] =
		//{
		//    0x3F,0x01,0x00,0x01, 0x00,0x02,0x02,0x0D, 0x08,0x10,0x08,0x24, 0x00,0x00,0x04,0x2C,
		//    0x09,0x01,0x34,0x03, 0x00,0x04,0x00,0x14, 0x08,0x3A,0x00,0x02, 0x00,0x20,0x2C,0x08
		//};

		uint32_t  POWER_ON_PALETTE[32] = { 0x09,0x01,0x00,0x01,0x00,0x02,0x02,0x0D,0x08,0x10,0x08,0x24,0x00,0x00,0x04,0x2C,
										   0x09,0x01,0x34,0x03,0x00,0x04,0x00,0x14,0x08,0x3A,0x00,0x02,0x00,0x20,0x2C,0x08 };

		const static uint32_t PPU_PHASE_VBL = 0;
		const static uint32_t PPU_PHASE_BG = 1;
		const static uint32_t PPU_PHASE_OBJ = 2;

		// other values of action are reserved for possibly needed expansions, but this passes
		// ppu_open_bus for now.
		const static uint32_t DecayType_None = 0; // if there is no action, decrement the timer
		const static uint32_t DecayType_All = 1; // reset the timer for all bits (reg 2004 / 2007 (non-palette)
		const static uint32_t DecayType_High = 2; // reset the timer for high 3 bits (reg 2002)
		const static uint32_t DecayType_Low = 3; // reset the timer for all low 6 bits (reg 2007 (palette))

		inline bool PPUON() { return show_bg_new || show_obj_new; }
		inline bool ppu_Is_Rendering() { return (status_sl == 261) || (status_sl < 240); }

		bool ppu_HasClockPPU;
		bool ppu_Chop_Dot;
		bool ppu_IdleSynch;
		bool ppu_Dot_Was_Skipped;
		bool ppu_Toggle_w;

		bool ppu_Color_Disable; //Color disable (0: normal color; 1: AND all palette entries with 110000, effectively producing a monochrome display)
		bool ppu_Show_BG_Leftmost; //Show leftmost 8 pixels of background
		bool ppu_Show_OBJ_Leftmost; //Show sprites in leftmost 8 pixels
		bool ppu_Show_BG; //Show background
		bool ppu_Show_OBJ; //Show sprites
		bool ppu_Intense_Green; //Intensify greens (and darken other colors)
		bool ppu_Intense_Blue; //Intensify blues (and darken other colors)
		bool ppu_Intense_Red; //Intensify reds (and darken other colors)

		bool ppu_Vram_Incr32; //(0: increment by 1, going across; 1: increment by 32, going down)
		bool ppu_OBJ_Pattern_High; //Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000)
		bool ppu_BG_Pattern_High; //Background pattern table address (0: $0000; 1: $1000)
		bool ppu_OBJ_Size_16; //Sprite size (0: 8x8 sprites; 1: 8x16 sprites)
		bool ppu_Layer; //PPU layer select (should always be 0 in the SNES; some Nintendo arcade boards presumably had two PPUs)
		bool ppu_Vblank_NMI_Gen; //Vertical blank NMI generation (0: off; 1: on)

		bool ppu_Reg2002_objoverflow;  //Sprite overflow. The PPU can handle only eight sprites on one scanline and sets this bit if it starts drawing sprites.
		bool ppu_Reg2002_objhit; //Sprite 0 overlap.  Set when a nonzero pixel of sprite 0 is drawn overlapping a nonzero background pixel.  Used for raster timing.
		bool ppu_Reg2002_vblank_active;  //Vertical blank start (0: has not started; 1: has started)
		bool ppu_Reg2002_vblank_active_pending; //set if Reg2002_vblank_active is pending
		bool ppu_Reg2002_vblank_clear_pending; //ppu's clear of vblank flag is pending

		bool sprite_eval_write;
		bool sprite_zero_in_range;
		bool sprite_zero_go;
		bool ppu_was_on;
		bool race_2006;
		bool show_bg_new; //Show background
		bool show_obj_new; //Show sprites
		bool do_vbl;
		bool do_active_sl;
		bool do_pre_vbl;
		bool nmi_destiny;
		bool evenOddDestiny;
		bool last_pipeline;
		bool ppu_Commit_Read;
		bool ppu_Can_Corrupt;
		bool ppu_Sprite_Draw_Glitch;

		uint8_t ppu_OAM_Corrupt_Addr;
		uint8_t VRAMBuffer;
		uint8_t read_value;
		uint8_t reg_2003;
		uint8_t glitch_2007_iter;
		uint8_t glitch_2007_addr;
		uint8_t ppu_temp_oam_y;
		uint8_t ppu_temp_oam_ind = 0;
		uint8_t ppu_BG_NT_Addr;

		// this uint8_t is used to simulate open bus reads and writes
		// it should be modified by every read and write to a ppu register
		uint8_t ppu_Open_Bus;

		uint16_t ppu_Reg_v;
		uint16_t ppu_Reg_t;
		uint16_t ppu_Scroll_x;
		uint16_t ppu_Raster_Pos;
		uint16_t ppu_VRAM_Address;
		uint16_t ppu_BG_Pattern_0;
		uint16_t ppu_BG_Pattern_1;
		uint16_t ppu_Next_BG_Pt_0;
		uint16_t ppu_Next_BG_Pt_1;

		uint32_t cpu_step, cpu_stepcounter;
		uint32_t ppuphase;
		uint32_t ppudead;
		uint32_t NMI_PendingInstructions;
		uint32_t intensity_lsl_6;
		uint32_t status_sl;
		uint32_t status_cycle;
		uint32_t spr_true_count;
		uint32_t soam_index;
		uint32_t oam_index;
		uint32_t yp;
		uint32_t yp_sp;
		uint32_t target;
		uint32_t spriteHeight;
		uint32_t install_2006;
		uint32_t install_2001;
		uint32_t start_up_offset;
		uint32_t NMI_offset;
		uint32_t yp_shift;
		uint32_t sprite_eval_cycle;
		uint32_t xt;
		uint32_t xp;
		uint32_t xstart;
		uint32_t s;
		uint32_t ppu_aux_index;
		uint32_t line;
		uint32_t patternNumber;
		uint32_t patternAddress;
		uint32_t ppu_Sprite_Draw_Cycle;

		uint32_t pixelcolor_latch_1;
		uint32_t pixelcolor_latch_2;

		// sequential reads and RMW instructions cause glitches (but not STA instructions)
		uint64_t double_2007_read;

		uint64_t Total_PPU_Clock_Cycles;

		uint8_t glitchy_reads_2003[8] = { };
		uint8_t OAM[544] = { };
		uint8_t PALRAM[0x200] = { };

		uint8_t ppu_BG_Attr[3] = { };

		uint8_t soam[256] = { };

		struct Sprite_Shifter_Struct
		{
			bool X_Start;
			uint16_t X;
			uint8_t Attr;
			uint8_t Pattern_0;
			uint8_t Pattern_1;
		} ppu_Sprite_Shifters[8];

		uint16_t xbuf[256 * 240] = { };

		uint32_t ppu_open_bus_decay_timer[8] = { };

		void (*NTViewCallback)(void);
		void (*PPUViewCallback)(void);

		int NTView_Scanline = 0;
		int PPUView_Scanline = 0;

		uint8_t ppubus_read(uint32_t addr);

		void ppubus_clock(uint32_t addr);

		uint8_t ppubus_peek(uint32_t addr);

		void ppu_Run();

		void ppu_SoftReset()
		{
			//this hasn't been brought up to date since SNEShawk was first made.
			//in particular http://wiki.SNESdev.com/w/index.php/PPU_power_up_state should be studied, but theres no use til theres test cases
			ppu_Reset();
		}

		void ppu_Reset()
		{
			ppu_Chop_Dot = true;
			ppu_Dot_Was_Skipped = false;
			ppudead = 1;
			ppu_IdleSynch = true;
			ppu_Open_Bus = 0;

			ppu_Reg_v = 0;
			ppu_Reg_t = 0;
			ppu_Scroll_x = 0;
			ppu_Raster_Pos = 0;
			ppu_VRAM_Address = 0;
			ppu_BG_Pattern_0 = 0;
			ppu_BG_Pattern_1 = 0;
			ppu_Next_BG_Pt_0 = 0;
			ppu_Next_BG_Pt_1 = 0;
			ppu_BG_Attr[0] = 0;
			ppu_BG_Attr[1] = 0;
			ppu_BG_Attr[2] = 0;

			ppu_temp_oam_y = 0;
			ppu_temp_oam_ind = 0;
			ppu_BG_NT_Addr = 0;

			double_2007_read = 0;
			start_up_offset = 5;

			ppu_Color_Disable = false;
			ppu_Show_BG_Leftmost = false;
			ppu_Show_OBJ_Leftmost = false;
			ppu_Show_BG = false;
			ppu_Show_OBJ = false;
			ppu_Intense_Green = false;
			ppu_Intense_Blue = false;
			ppu_Intense_Red = false;

			ppu_Vram_Incr32 = false;
			ppu_OBJ_Pattern_High = false;
			ppu_BG_Pattern_High = false;
			ppu_OBJ_Size_16 = false;
			ppu_Layer = false;
			ppu_Vblank_NMI_Gen = false;
			ppu_Commit_Read = false;
			ppu_Can_Corrupt = false;
			ppu_Sprite_Draw_Glitch = false;

			ppu_OAM_Corrupt_Addr = 0;
			intensity_lsl_6 = 0;

			ppu_Reg2002_objoverflow = false;
			ppu_Reg2002_objhit = false;
			ppu_Reg2002_vblank_active = false;
			reg_2003 = 0;
			glitch_2007_iter = 0;
			glitch_2007_addr = 0;
			ppu_Toggle_w = false;
			VRAMBuffer = 0;

			status_cycle = 0;
			status_sl = 0;

			cpu_step = 0;
			cpu_stepcounter = 0;

			NMI_PendingInstructions = 0;

			spr_true_count = 0;
			sprite_eval_write = false;
			read_value = 0;
			sprite_zero_go = false;
			sprite_zero_in_range = false;
			soam_index = 0;
			oam_index = 0;
			yp = 0;
			yp_sp = 0;
			target = 0;
			ppu_was_on = false;
			spriteHeight = 0;
			install_2006 = 0;
			race_2006 = false;
			install_2001 = 0;
			show_bg_new = false;
			show_obj_new = false;
			evenOddDestiny = false;

			ppuphase = 0;

			ppu_Reg2002_vblank_active_pending = 0;
			ppu_Reg2002_vblank_clear_pending = 0;

			Total_PPU_Clock_Cycles = 0;
			do_vbl = 0;
			do_active_sl = 0;
			do_pre_vbl = 0;

			nmi_destiny = 0;
			NMI_offset = 0;
			yp_shift = 0;
			sprite_eval_cycle = 0;
			xt = 0;
			xp = 0;
			xstart = 0;

			s = 0;
			ppu_aux_index = 0;
			line = 0;
			patternNumber = 0;
			patternAddress = 0;
			ppu_Sprite_Draw_Cycle = 0;

			for (int i = 0; i < 8; i++)
			{
				ppu_open_bus_decay_timer[i] = 0;
			}

			for (int i = 0; i < 8; i++)
			{
				glitchy_reads_2003[i] = 0;
			}

			for (int i = 0; i < 256; i++)
			{
				OAM[i] = 0;
				soam[i] = 0xFF;
			}

			for (int i = 0; i < 0x200; i++)
			{
				PALRAM[i] = 0;
			}

			for (int i = 0; i < 8; i++)
			{
				ppu_Sprite_Shifters[i].X_Start = false;
				ppu_Sprite_Shifters[i].X = 0;
				ppu_Sprite_Shifters[i].Attr = 0;
				ppu_Sprite_Shifters[i].Pattern_0 = 0;
				ppu_Sprite_Shifters[i].Pattern_1 = 0;
			}
		}

		void ppu_Install_Latches()
		{
			ppu_Reg_v = ppu_Reg_t;
		}

		void ppu_Install_h_Latches()
		{
			ppu_Reg_v &= 0xFBE0;
			ppu_Reg_v |= (ppu_Reg_t & 0x41F);
		}

		void ppu_Install_v_Latches()
		{
			ppu_Reg_v &= 0x41F;
			ppu_Reg_v |= (ppu_Reg_t & 0xFBE0);
		}

		void ppu_Increment_hsc()
		{
			if ((ppu_Reg_v & 0x001F) == 31) // if coarse X == 31
			{
				ppu_Reg_v &= ~0x001F;         // coarse X = 0
				ppu_Reg_v ^= 0x0400;           // switch horizontal nametable
			}
			else
			{
				ppu_Reg_v += 1;                // increment coarse X
			}

			ppu_Reg_v &= 0x7FFF;
		}

		void ppu_Increment_vs()
		{
			if ((ppu_Reg_v & 0x7000) != 0x7000)			// if fine Y < 7
			{
				ppu_Reg_v += 0x1000;					// increment fine Y
			}
			else
			{
				ppu_Reg_v &= ~0x7000;					// fine Y = 0
				int y = (ppu_Reg_v & 0x03E0) >> 5;      // let y = coarse Y
				if (y == 29)
				{
					y = 0;								// coarse Y = 0
					ppu_Reg_v ^= 0x0800;				// switch vertical nametable
				}
				else if (y == 31)
				{
					y = 0;								// coarse Y = 0, nametable not switched
				}
				else
				{
					y += 1;								// increment coarse Y
				}

				ppu_Reg_v = (ppu_Reg_v & ~0x03E0) | (y << 5); // put coarse Y back into v
			}

			ppu_Reg_v &= 0x7FFF;
		}

		int ppu_Get_NT_Read()
		{
			return 0x2000 | (ppu_Reg_v & 0x0FFF);
		}

		int ppu_Get_AT_Read()
		{
			return 0x23C0 | (ppu_Reg_v & 0x0C00) | ((ppu_Reg_v >> 4) & 0x38) | ((ppu_Reg_v >> 2) & 0x07);
		}

		void ppu_Increment_2007(bool rendering, bool by32)
		{
			// Glitch: both horizontal and vertical increment triggered
			if (rendering)
			{
				ppu_Increment_hsc();
				ppu_Increment_vs();
				return;
			}

			if (by32)
			{
				ppu_Reg_v += 32;
			}
			else
			{
				ppu_Reg_v += 1;
			}

			ppu_Reg_v &= 0x7FFF;
		}

		uint8_t ppu_ReadReg(int addr)                                                   // Register Reads
		{
			uint8_t ret_spec;
			switch (addr)
			{
				case 0: return read_2000();
				case 1: return read_2001();
				case 2: return read_2002();
				case 3: return read_2003();
				case 4: return read_2004();
				case 5: return read_2005();
				case 6: return read_2006();
				case 7:
					{
						ret_spec = ppu_Open_Bus;

						//Message_String = "rd " + to_string(ppu_VRAM_Address) + " cyc: " + to_string(TotalExecutedCycles);

						//MessageCallback(Message_String.length());

						glitch_2007_iter = 0;

						return ret_spec;
					}
				default: throw new exception();
			}
		}

		uint8_t read_2000() { return ppu_Open_Bus; }

		uint8_t read_2001() { return ppu_Open_Bus; }

		uint8_t read_2002()
		{
			uint8_t ret = peek_2002();
			/*
			if (do_the_reread_2002 > 0)
			{
				if (Reg2002_vblank_active || Reg2002_vblank_active_pending)
					Console.WriteLine("reread 2002");
			}
			*/

			// reading from $2002 resets the destination for $2005 and $2006 writes
			ppu_Toggle_w = false;
			ppu_Reg2002_vblank_active = false;
			ppu_Reg2002_vblank_active_pending = false;

			// update the open bus here
			ppu_Open_Bus = ret;
			PpuOpenBusDecay(DecayType_High);
			return ret;
		}

		uint8_t read_2003() { return ppu_Open_Bus; }

		uint8_t read_2004()
		{
			uint8_t ret;

			// behaviour depends on whether things are being rendered or not
			if (PPUON())
			{
				if (ppu_Is_Rendering())
				{
					if (status_cycle == 0)
					{
						ret = soam[0];
					}
					else if (status_cycle <= 64)
					{
						ret = 0xFF; // during this time all reads return FF
					}
					else if (status_cycle <= 256)
					{
						ret = read_value;
					}
					else if (status_cycle <= 320)
					{
						ret = read_value;
					}
					else
					{
						ret = soam[0];
					}
				}
				else
				{
					ret = OAM[reg_2003];
				}
			}
			else
			{
				ret = OAM[reg_2003];
			}

			ppu_Open_Bus = ret;
			PpuOpenBusDecay(DecayType_All);
			return ret;
		}

		uint8_t read_2005() { return ppu_Open_Bus; }

		uint8_t read_2006() { return ppu_Open_Bus; }

		uint8_t read_2007();

		void ppu_WriteReg(int addr, uint8_t value)                                       // Register Writes
		{
			ppu_Open_Bus = value;

			switch (addr & 0x07)
			{
				case 0: write_2000(value); break;
				case 1: write_2001(value); break;
				case 2: write_2002(value); break;
				case 3: write_2003(addr, value); break;
				case 4: write_2004(value); break;
				case 5: write_2005(value); break;
				case 6: write_2006(value); break;
				case 7:
					write_2007(value);

					break;
				default: throw new exception();
			}
		}

		void write_2000(uint8_t value)
		{
			if (!ppu_Vblank_NMI_Gen && ((value & 0x80) != 0) && (ppu_Reg2002_vblank_active) && !ppu_Reg2002_vblank_clear_pending)
			{
				//if we just unleashed the vblank interrupt then activate it now
				//if (ppudead != 1)
				NMI_PendingInstructions = 2;
			}

			ppu_Vram_Incr32 = ((value >> 2) & 1) == 1;
			ppu_OBJ_Pattern_High = ((value >> 3) & 1) == 1;
			ppu_BG_Pattern_High = ((value >> 4) & 1) == 1;
			ppu_OBJ_Size_16 = ((value >> 5) & 1) == 1;
			ppu_Layer = ((value >> 6) & 1) == 1;
			ppu_Vblank_NMI_Gen = ((value >> 7) & 1) == 1;

			ppu_Reg_t &= 0xF3FF;
			ppu_Reg_t |= ((uint16_t)value & 3) << 10;
		}

		void write_2001(uint8_t value)
		{
			ppu_Color_Disable = (value & 1) == 1;
			ppu_Show_BG_Leftmost = ((value >> 1) & 1) == 1;
			ppu_Show_OBJ_Leftmost = ((value >> 2) & 1) == 1;
			ppu_Show_BG = ((value >> 3) & 1) == 1;
			ppu_Show_OBJ = ((value >> 4) & 1) == 1;
			ppu_Intense_Blue = ((value >> 6) & 1) == 1;
			ppu_Intense_Red = ((value >> 7) & 1) == 1;
			ppu_Intense_Green = ((value >> 5) & 1) == 1;
			intensity_lsl_6 = ((value >> 5) & 7) << 6;

			install_2001 = 1;
		}

		void write_2002(uint8_t value) {}

		void write_2003(int addr, uint8_t value)
		{
			// in NTSC this does several glitchy things to corrupt OAM
			// commented out for now until better understood
			uint8_t temp = (uint8_t)(reg_2003 & 0xF8);
			uint8_t temp_2 = (uint8_t)(addr >> 16 & 0xF8);
			/*
			for (int i=0;i<8;i++)
			{
				glitchy_reads_2003[i] = OAM[temp + i];
				//OAM[temp_2 + i] = glitchy_reads_2003[i];
			}
			*/
			reg_2003 = value;
		}

		void write_2004(uint8_t value)
		{
			if ((reg_2003 & 3) == 2)
			{
				//some of the OAM bits are unwired so we mask them out here
				//otherwise we just write this value and move on to the next oam byte
				value &= 0xE3;
			}
			if (ppu_Is_Rendering())
			{
				// don't write to OAM if the screen is on and we are in the active display area
				// this impacts sprite evaluation
				if (show_bg_new || show_obj_new)
				{
					// glitchy increment of OAM index
					oam_index += 4;
					reg_2003 += 4;

					oam_index &= 0xFFC;
					reg_2003 &= 0xFC;
				}
				else
				{
					OAM[reg_2003] = value;
					reg_2003++;
				}
			}
			else
			{
				OAM[reg_2003] = value;
				reg_2003++;
			}
		}

		void write_2005(uint8_t value)
		{
			if (!ppu_Toggle_w)
			{
				ppu_Reg_t &= 0xFFE0;
				ppu_Reg_t |= ((uint16_t)(value >> 3) & 0x1F);
				ppu_Scroll_x = value & 7;
			}
			else
			{
				ppu_Reg_t &= 0x0C1F;
				ppu_Reg_t |= (((uint16_t)(value >> 3) & 0x1F) << 5);
				ppu_Reg_t |= (((uint16_t)value & 7) << 12);
			}

			ppu_Toggle_w ^= true;
		}

		void write_2006(uint8_t value)
		{

			if (!ppu_Toggle_w)
			{
				// Note: upper bit of address cleared (7 vits cleared even though we only write 6)
				ppu_Reg_t &= 0xFF;
				ppu_Reg_t |= (((uint16_t)value & 0x3F) << 8);
			}
			else
			{
				ppu_Reg_t &= 0xFF00;
				ppu_Reg_t |= value;

				// testing indicates that this operation is delayed by 3 pixels
				//install_latches();				
				install_2006 = 3;
			}

			ppu_Toggle_w ^= true;
		}

		void write_2007(uint8_t value);

		void write_2007_Glitch(uint8_t value);

		uint8_t ppu_PeekReg(int addr)
		{
			switch (addr)
			{
				case 0: return peek_2000(); case 1: return peek_2001(); case 2: return peek_2002(); case 3: return peek_2003();
				case 4: return peek_2004(); case 5: return peek_2005(); case 6: return peek_2006(); case 7: return peek_2007();
				default: throw new exception();
			}
		}

		uint8_t peek_2000() { return ppu_Open_Bus; }

		uint8_t peek_2001() { return ppu_Open_Bus; }

		uint8_t peek_2002()
		{
			uint8_t ret = (uint8_t)(ppu_Open_Bus & 0x1F);

			if (ppu_Reg2002_objoverflow)
			{
				ret |= 0x20;
			}

			if (ppu_Reg2002_objhit)
			{
				ret |= 0x40;
			}

			if (ppu_Reg2002_vblank_active)
			{
				ret |= 0x80;
			}

			return ret;
		}

		uint8_t peek_2003() { return ppu_Open_Bus; }

		uint8_t peek_2004() { return OAM[reg_2003]; }

		uint8_t peek_2005() { return ppu_Open_Bus; }

		uint8_t peek_2006() { return ppu_Open_Bus; }

		uint8_t peek_2007()
		{
			int addr = ppu_Reg_v;

			//ordinarily we return the buffered values
			uint8_t ret = VRAMBuffer;

			//in any case, we read from the ppu bus
			// can't do this in peek; updates the value that will be used later
			// VRAMBuffer = ppubus_peek(addr);

			//but reads from the palette are implemented in the PPU and return immediately
			if ((addr & 0x3F00) == 0x3F00)
			{
				//TODO apply greyscale shit?
				ret = PALRAM[addr & 0x1F];
			}

			return ret;
		}

		void PpuOpenBusDecay(uint32_t action)
		{
			switch (action)
			{
				case DecayType_None:
					for (int i = 0; i < 8; i++)
					{
						if (ppu_open_bus_decay_timer[i] == 0)
						{
							ppu_Open_Bus = (uint8_t)(ppu_Open_Bus & (0xff - (1 << i)));
							ppu_open_bus_decay_timer[i] = 1786840; // about 1 second worth of cycles
						}
						else
						{
							ppu_open_bus_decay_timer[i]--;
						}
					}
					break;
				case DecayType_All:
					for (int i = 0; i < 8; i++)
					{
						ppu_open_bus_decay_timer[i] = 1786840;
					}
					break;
				case DecayType_High:
					ppu_open_bus_decay_timer[7] = 1786840;
					ppu_open_bus_decay_timer[6] = 1786840;
					ppu_open_bus_decay_timer[5] = 1786840;
					break;
				case DecayType_Low:
					for (int i = 0; i < 6; i++)
					{
						ppu_open_bus_decay_timer[i] = 1786840;
					}
					break;
			}
		}

		void pipeline(int pixelcolor, int row_check)
		{
			if (row_check > 1)
			{
				if (ppu_Color_Disable)
					pixelcolor_latch_2 &= 0x30;

				xbuf[target-2] = (uint16_t)(pixelcolor_latch_2 | intensity_lsl_6);
			}

			pixelcolor_latch_2 = pixelcolor_latch_1;
			pixelcolor_latch_1 = pixelcolor;
		}

		//address line 3 relates to the pattern table fetch occuring (the PPU always makes them in pairs).
		int get_ptread(int par)
		{
			int hi = ppu_BG_Pattern_High ? 1 : 0;
			return (hi << 0xC) | (par << 0x4) | ((ppu_Reg_v >> 12) & 7);
		}

		void Read_bgdata(int cycle, int i)
		{
			uint8_t at = 0;
			
			switch (cycle)
			{
				case 0:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (ppu_Get_NT_Read() & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 1:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = ppu_Get_NT_Read();
						ppu_BG_NT_Addr = ppubus_read(ppu_VRAM_Address);
					}
					break;
				case 2:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (ppu_Get_AT_Read() & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 3:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = ppu_Get_AT_Read();

						at = ppubus_read(ppu_VRAM_Address);

						// modify at to get appropriate palette shift
						if ((ppu_Reg_v & 0x40) != 0) at >>= 4;
						if ((ppu_Reg_v & 2) != 0) at >>= 2;
						at &= 0x03;
						at <<= 2;
						ppu_BG_Attr[2] = at;
					}
					break;
				case 4:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (get_ptread(ppu_BG_NT_Addr) & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 5:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = get_ptread(ppu_BG_NT_Addr);
						ppu_Next_BG_Pt_0 = ppubus_read(ppu_VRAM_Address);
					}
					break;
				case 6:
					ppu_Commit_Read = PPUON();
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address &= 0xFF;
						ppu_VRAM_Address |= (get_ptread(ppu_BG_NT_Addr) & 0xFF00);
						ppubus_clock(ppu_VRAM_Address);
					}
					break;
				case 7:
					if (ppu_Commit_Read)
					{
						ppu_VRAM_Address = get_ptread(ppu_BG_NT_Addr);
						ppu_VRAM_Address |= 8;
						ppu_Next_BG_Pt_1 = ppubus_read(ppu_VRAM_Address);
					}
					break;
			}
		}

		void TickPPU(bool);

		//not quite emulating all the SNES power up behavior
		//since it is known that the SNES ignores writes to some
		//register before around a full frame, but no games
		//should write to those regs during that time, it needs
		//to wait for vblank
		void NewDeadPPU(uint32_t cycles_to_run)
		{
			for (uint32_t i = 0; i < cycles_to_run; i++)
			{
				ppu_Run();

				status_cycle += 1;

				if (status_cycle == 241 * 341 - start_up_offset)
				{
					ppudead--;

					status_cycle = 0;

					status_sl = 241;

					do_vbl = true;

					frame_is_done = true;
				}

				FrameCycle += 1;
			}
		}

		uint8_t BitReverse(uint8_t b)
		{
			uint8_t ret = (uint8_t)(b >> 7);

			uint8_t bit_0 = (uint8_t)(b & 1);
			uint8_t bit_1 = (uint8_t)(b & 2);
			uint8_t bit_2 = (uint8_t)(b & 4);
			uint8_t bit_3 = (uint8_t)(b & 8);
			uint8_t bit_4 = (uint8_t)(b & 0x10);
			uint8_t bit_5 = (uint8_t)(b & 0x20);
			uint8_t bit_6 = (uint8_t)(b & 0x40);

			ret |= (uint8_t)(bit_0 << 7);
			ret |= (uint8_t)(bit_1 << 5);
			ret |= (uint8_t)(bit_2 << 3);
			ret |= (uint8_t)(bit_3 << 1);
			ret |= (uint8_t)(bit_4 >> 1);
			ret |= (uint8_t)(bit_5 >> 3);
			ret |= (uint8_t)(bit_6 >> 5);

			return ret;
		}

		uint8_t* ppu_SaveState(uint8_t* saver)
		{
			saver = bool_saver(ppu_HasClockPPU, saver);
			saver = bool_saver(ppu_Chop_Dot, saver);
			saver = bool_saver(ppu_IdleSynch, saver);
			saver = bool_saver(ppu_Dot_Was_Skipped, saver);
			saver = bool_saver(ppu_Toggle_w, saver);

			saver = bool_saver(ppu_Color_Disable, saver);
			saver = bool_saver(ppu_Show_BG_Leftmost, saver);
			saver = bool_saver(ppu_Show_OBJ_Leftmost, saver);
			saver = bool_saver(ppu_Show_BG, saver);
			saver = bool_saver(ppu_Show_OBJ, saver);
			saver = bool_saver(ppu_Intense_Green, saver);
			saver = bool_saver(ppu_Intense_Blue, saver);
			saver = bool_saver(ppu_Intense_Red, saver);

			saver = bool_saver(ppu_Vram_Incr32, saver);
			saver = bool_saver(ppu_OBJ_Pattern_High, saver);
			saver = bool_saver(ppu_BG_Pattern_High, saver);
			saver = bool_saver(ppu_OBJ_Size_16, saver);
			saver = bool_saver(ppu_Layer, saver);
			saver = bool_saver(ppu_Vblank_NMI_Gen, saver);

			saver = bool_saver(ppu_Reg2002_objoverflow, saver);
			saver = bool_saver(ppu_Reg2002_objhit, saver);
			saver = bool_saver(ppu_Reg2002_vblank_active, saver);
			saver = bool_saver(ppu_Reg2002_vblank_active_pending, saver);
			saver = bool_saver(ppu_Reg2002_vblank_clear_pending, saver);

			saver = bool_saver(sprite_eval_write, saver);
			saver = bool_saver(sprite_zero_in_range, saver);
			saver = bool_saver(sprite_zero_go, saver);
			saver = bool_saver(ppu_was_on, saver);
			saver = bool_saver(race_2006, saver);
			saver = bool_saver(show_bg_new, saver);
			saver = bool_saver(show_obj_new, saver);
			saver = bool_saver(do_vbl, saver);
			saver = bool_saver(do_active_sl, saver);
			saver = bool_saver(do_pre_vbl, saver);
			saver = bool_saver(nmi_destiny, saver);
			saver = bool_saver(evenOddDestiny, saver);
			saver = bool_saver(last_pipeline, saver);
			saver = bool_saver(ppu_Commit_Read, saver);
			saver = bool_saver(ppu_Can_Corrupt, saver);
			saver = bool_saver(ppu_Sprite_Draw_Glitch, saver);

			saver = byte_saver(ppu_OAM_Corrupt_Addr, saver);
			saver = byte_saver(VRAMBuffer, saver);
			saver = byte_saver(read_value, saver);
			saver = byte_saver(reg_2003, saver);
			saver = byte_saver(glitch_2007_iter, saver);
			saver = byte_saver(glitch_2007_addr, saver);
			saver = byte_saver(ppu_temp_oam_y, saver);
			saver = byte_saver(ppu_temp_oam_ind, saver);
			saver = byte_saver(ppu_BG_NT_Addr, saver);

			saver = byte_saver(ppu_Open_Bus, saver);

			saver = short_saver(ppu_Reg_v, saver);
			saver = short_saver(ppu_Reg_t, saver);
			saver = short_saver(ppu_Scroll_x, saver);
			saver = short_saver(ppu_Raster_Pos, saver);
			saver = short_saver(ppu_VRAM_Address, saver);
			saver = short_saver(ppu_BG_Pattern_0, saver);
			saver = short_saver(ppu_BG_Pattern_1, saver);
			saver = short_saver(ppu_Next_BG_Pt_0, saver);
			saver = short_saver(ppu_Next_BG_Pt_1, saver);

			saver = int_saver(cpu_step, saver);
			saver = int_saver(cpu_stepcounter, saver);
			saver = int_saver(ppuphase, saver);
			saver = int_saver(ppudead, saver);
			saver = int_saver(NMI_PendingInstructions, saver);
			saver = int_saver(intensity_lsl_6, saver);
			saver = int_saver(status_sl, saver);
			saver = int_saver(status_cycle, saver);
			saver = int_saver(spr_true_count, saver);
			saver = int_saver(soam_index, saver);
			saver = int_saver(oam_index, saver);
			saver = int_saver(yp, saver);
			saver = int_saver(yp_sp, saver);
			saver = int_saver(target, saver);
			saver = int_saver(spriteHeight, saver);
			saver = int_saver(install_2006, saver);
			saver = int_saver(install_2001, saver);
			saver = int_saver(start_up_offset, saver);
			saver = int_saver(NMI_offset, saver);
			saver = int_saver(yp_shift, saver);
			saver = int_saver(sprite_eval_cycle, saver);
			saver = int_saver(xt, saver);
			saver = int_saver(xp, saver);
			saver = int_saver(xstart, saver);
			saver = int_saver(s, saver);
			saver = int_saver(ppu_aux_index, saver);
			saver = int_saver(line, saver);
			saver = int_saver(patternNumber, saver);
			saver = int_saver(patternAddress, saver);
			saver = int_saver(ppu_Sprite_Draw_Cycle, saver);
			saver = int_saver(pixelcolor_latch_1, saver);
			saver = int_saver(pixelcolor_latch_2, saver);

			saver = long_saver(double_2007_read, saver);
			saver = long_saver(Total_PPU_Clock_Cycles, saver);

			saver = byte_array_saver(glitchy_reads_2003, saver, 8);
			saver = byte_array_saver(OAM, saver, 544);
			saver = byte_array_saver(PALRAM, saver, 0x200);
			saver = byte_array_saver(ppu_BG_Attr, saver, 3);
			saver = byte_array_saver(soam, saver, 256);

			saver = short_array_saver(xbuf, saver, 256 * 240);

			saver = int_array_saver(ppu_open_bus_decay_timer, saver, 8);

			for (int i = 0; i < 8; i++)
			{
				saver = bool_saver(ppu_Sprite_Shifters[i].X_Start, saver);
				saver = short_saver(ppu_Sprite_Shifters[i].X, saver);
				saver = byte_saver(ppu_Sprite_Shifters[i].Attr, saver);
				saver = byte_saver(ppu_Sprite_Shifters[i].Pattern_0, saver);
				saver = byte_saver(ppu_Sprite_Shifters[i].Pattern_1, saver);
			}

			return saver;
		}

		uint8_t* ppu_LoadState(uint8_t* loader)
		{
			loader = bool_loader(&ppu_HasClockPPU, loader);
			loader = bool_loader(&ppu_Chop_Dot, loader);
			loader = bool_loader(&ppu_IdleSynch, loader);
			loader = bool_loader(&ppu_Dot_Was_Skipped, loader);
			loader = bool_loader(&ppu_Toggle_w, loader);

			loader = bool_loader(&ppu_Color_Disable, loader);
			loader = bool_loader(&ppu_Show_BG_Leftmost, loader);
			loader = bool_loader(&ppu_Show_OBJ_Leftmost, loader);
			loader = bool_loader(&ppu_Show_BG, loader);
			loader = bool_loader(&ppu_Show_OBJ, loader);
			loader = bool_loader(&ppu_Intense_Green, loader);
			loader = bool_loader(&ppu_Intense_Blue, loader);
			loader = bool_loader(&ppu_Intense_Red, loader);

			loader = bool_loader(&ppu_Vram_Incr32, loader);
			loader = bool_loader(&ppu_OBJ_Pattern_High, loader);
			loader = bool_loader(&ppu_BG_Pattern_High, loader);
			loader = bool_loader(&ppu_OBJ_Size_16, loader);
			loader = bool_loader(&ppu_Layer, loader);
			loader = bool_loader(&ppu_Vblank_NMI_Gen, loader);

			loader = bool_loader(&ppu_Reg2002_objoverflow, loader);
			loader = bool_loader(&ppu_Reg2002_objhit, loader);
			loader = bool_loader(&ppu_Reg2002_vblank_active, loader);
			loader = bool_loader(&ppu_Reg2002_vblank_active_pending, loader);
			loader = bool_loader(&ppu_Reg2002_vblank_clear_pending, loader);

			loader = bool_loader(&sprite_eval_write, loader);
			loader = bool_loader(&sprite_zero_in_range, loader);
			loader = bool_loader(&sprite_zero_go, loader);
			loader = bool_loader(&ppu_was_on, loader);
			loader = bool_loader(&race_2006, loader);
			loader = bool_loader(&show_bg_new, loader);
			loader = bool_loader(&show_obj_new, loader);
			loader = bool_loader(&do_vbl, loader);
			loader = bool_loader(&do_active_sl, loader);
			loader = bool_loader(&do_pre_vbl, loader);
			loader = bool_loader(&nmi_destiny, loader);
			loader = bool_loader(&evenOddDestiny, loader);
			loader = bool_loader(&last_pipeline, loader);
			loader = bool_loader(&ppu_Commit_Read, loader);
			loader = bool_loader(&ppu_Can_Corrupt, loader);
			loader = bool_loader(&ppu_Sprite_Draw_Glitch, loader);

			loader = byte_loader(&ppu_OAM_Corrupt_Addr, loader);
			loader = byte_loader(&VRAMBuffer, loader);
			loader = byte_loader(&read_value, loader);
			loader = byte_loader(&reg_2003, loader);
			loader = byte_loader(&glitch_2007_iter, loader);
			loader = byte_loader(&glitch_2007_addr, loader);
			loader = byte_loader(&ppu_temp_oam_y, loader);
			loader = byte_loader(&ppu_temp_oam_ind, loader);
			loader = byte_loader(&ppu_BG_NT_Addr, loader);

			loader = byte_loader(&ppu_Open_Bus, loader);

			loader = short_loader(&ppu_Reg_v, loader);
			loader = short_loader(&ppu_Reg_t, loader);
			loader = short_loader(&ppu_Scroll_x, loader);
			loader = short_loader(&ppu_Raster_Pos, loader);
			loader = short_loader(&ppu_VRAM_Address, loader);
			loader = short_loader(&ppu_BG_Pattern_0, loader);
			loader = short_loader(&ppu_BG_Pattern_1, loader);
			loader = short_loader(&ppu_Next_BG_Pt_0, loader);
			loader = short_loader(&ppu_Next_BG_Pt_1, loader);

			loader = int_loader(&cpu_step, loader);
			loader = int_loader(&cpu_stepcounter, loader);
			loader = int_loader(&ppuphase, loader);
			loader = int_loader(&ppudead, loader);
			loader = int_loader(&NMI_PendingInstructions, loader);
			loader = int_loader(&intensity_lsl_6, loader);
			loader = int_loader(&status_sl, loader);
			loader = int_loader(&status_cycle, loader);
			loader = int_loader(&spr_true_count, loader);
			loader = int_loader(&soam_index, loader);
			loader = int_loader(&oam_index, loader);
			loader = int_loader(&yp, loader);
			loader = int_loader(&yp_sp, loader);
			loader = int_loader(&target, loader);
			loader = int_loader(&spriteHeight, loader);
			loader = int_loader(&install_2006, loader);
			loader = int_loader(&install_2001, loader);
			loader = int_loader(&start_up_offset, loader);
			loader = int_loader(&NMI_offset, loader);
			loader = int_loader(&yp_shift, loader);
			loader = int_loader(&sprite_eval_cycle, loader);
			loader = int_loader(&xt, loader);
			loader = int_loader(&xp, loader);
			loader = int_loader(&xstart, loader);
			loader = int_loader(&s, loader);
			loader = int_loader(&ppu_aux_index, loader);
			loader = int_loader(&line, loader);
			loader = int_loader(&patternNumber, loader);
			loader = int_loader(&patternAddress, loader);
			loader = int_loader(&ppu_Sprite_Draw_Cycle, loader);
			loader = int_loader(&pixelcolor_latch_1, loader);
			loader = int_loader(&pixelcolor_latch_2, loader);

			loader = long_loader(&double_2007_read, loader);
			loader = long_loader(&Total_PPU_Clock_Cycles, loader);

			loader = byte_array_loader(glitchy_reads_2003, loader, 8);
			loader = byte_array_loader(OAM, loader, 544);
			loader = byte_array_loader(PALRAM, loader, 0x200);
			loader = byte_array_loader(ppu_BG_Attr, loader, 3);
			loader = byte_array_loader(soam, loader, 256);

			loader = short_array_loader(xbuf, loader, 256 * 240);

			loader = int_array_loader(ppu_open_bus_decay_timer, loader, 8);

			for (int i = 0; i < 8; i++)
			{	
				loader = bool_loader(&ppu_Sprite_Shifters[i].X_Start, loader);
				loader = short_loader(&ppu_Sprite_Shifters[i].X, loader);
				loader = byte_loader(&ppu_Sprite_Shifters[i].Attr, loader);
				loader = byte_loader(&ppu_Sprite_Shifters[i].Pattern_0, loader);
				loader = byte_loader(&ppu_Sprite_Shifters[i].Pattern_1, loader);
			}

			return loader;
		}

	#pragma endregion

	};
}

#endif