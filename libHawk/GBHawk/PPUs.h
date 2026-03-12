#ifndef PPUS_H
#define PPUS_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace std;

//Core_Message_String->assign("Bank: " + to_string(PRG_Bank) + " Mask: " + to_string(PRG_Mask) + " len: " + to_string(*Core_Cycle_Count));

//MessageCallback(Core_Message_String->length());

namespace GBHawk
{
	class PPUs
	{
	public:
	#pragma region ppu base

		bool HDMA_active;
		bool clear_screen;
		bool rendering_complete;
		bool DMA_start;
		bool DMA_bus_control;
		bool LYC_INT;
		bool HBL_INT;
		bool VBL_INT;
		bool OAM_INT;
		bool stat_line;
		bool stat_line_old;
		bool LCD_was_off;
		bool no_scan;
		bool DMA_OAM_access;
		bool OAM_access_read;
		bool OAM_access_write;
		bool VRAM_access_read;
		bool VRAM_access_write;
		bool VRAM_access_read_PPU;
		bool VRAM_access_write_PPU;
		bool fetch_sprite;
		bool going_to_fetch;
		bool first_fetch;
		bool pre_render;
		bool pre_render_2;
		bool latch_new_data;
		bool no_sprites;
		bool window_pre_render;
		bool window_started;
		bool window_is_reset;
		bool blank_frame;
		bool window_latch;
		bool was_pre_render;
		bool pal_change_blocked; // in compatability mode, you can change palette values but not displayed color
		bool glitch_state; // writing to STAT to enable HBL interrupt won't trigger it if the ppu just turned on
		bool in_vbl; // writes to turn on mode 2 stat interrupts can trigger vbl stat at the start of vbl

		// register variables
		uint8_t LCDC;
		uint8_t STAT;
		uint8_t scroll_y;
		uint8_t scroll_x;
		uint8_t LY;
		uint8_t LY_actual;
		uint8_t LY_inc;
		uint8_t LYC;
		uint8_t DMA_addr;
		uint8_t BGP;
		uint8_t obj_pal_0;
		uint8_t obj_pal_1;
		uint8_t window_y;
		uint8_t window_x;
		uint8_t window_y_read;
		uint8_t window_x_read;
		uint8_t DMA_byte;

		// TODO: need a test ROM for the details here
		uint32_t bus_address;
		uint32_t DMA_clock;
		uint32_t DMA_inc;
		uint32_t cycle;
		uint32_t OAM_scan_index;
		uint32_t SL_sprites_index;
		uint32_t write_sprite;
		uint32_t read_case;
		uint32_t internal_cycle;
		uint32_t y_tile;
		uint32_t y_scroll_offset;
		uint32_t x_tile;
		uint32_t x_scroll_offset;
		uint32_t tile_byte;
		uint32_t sprite_fetch_counter;
		uint32_t temp_fetch;
		uint32_t tile_inc;
		uint32_t latch_counter;
		uint32_t render_counter;
		uint32_t render_offset;
		uint32_t scroll_offset;
		uint32_t pixel_counter;
		uint32_t pixel;
		uint32_t sl_use_index;
		uint32_t evaled_sprites;
		uint32_t sprite_ordered_index;
		uint32_t consecutive_sprite;
		uint32_t last_eval;
		uint32_t window_counter;
		uint32_t window_tile_inc;
		uint32_t window_y_tile;
		uint32_t window_x_tile;
		uint32_t window_y_tile_inc;
		uint32_t window_x_latch;
		uint32_t window_y_latch;
		uint32_t hbl_countdown;
		uint32_t sprite_scroll_offset;
		uint32_t read_case_prev;
		uint32_t LYC_offset; // in double speed mode it appears timing changes for LYC int
		uint32_t LY_153_change; // the timing of LYC chaning to 153 looks like it varies with speed mode
		uint32_t total_counter;

		uint8_t sprite_attr_list[160] = { };
		uint8_t sprite_pixel_list[160] = { };
		uint8_t sprite_present_list[160] = { };
		uint8_t tile_data[3] = { };
		uint8_t tile_data_latch[3] = { };
		uint8_t sprite_data[2] = { };
		uint8_t sprite_sel[2] = { };

		uint32_t color_palette[4] = { };
		uint32_t BG_palette[32] = { };
		uint32_t OBJ_palette[32] = { };
		uint32_t SL_sprites[40] = { };
		uint32_t SL_sprites_ordered[40] = { }; // (x_end, data_low, data_high, attr)



		// These variables only relate to GBC PPUs
		bool BG_bytes_inc;
		bool OBJ_bytes_inc;
		bool VRAM_access_read_HDMA;
		bool VRAM_access_write_HDMA;
		bool HDMA_can_start;
		// GBC specific glitch
		bool LCDC_Bit_4_glitch;
		bool BG_V_flip;
		bool HDMA_mode;
		bool HDMA_run_once;
		bool HBL_HDMA_go;
		bool HBL_test;

		uint8_t BG_bytes_index;
		uint8_t OBJ_bytes_index;
		uint8_t BG_transfer_byte;
		uint8_t OBJ_transfer_byte;
		uint8_t LYC_t;
		uint8_t LY_read;
		// HDMA is unique to GBC, do it as part of the PPU tick
		uint8_t HDMA_src_hi;
		uint8_t HDMA_src_lo;
		uint8_t HDMA_dest_hi;
		uint8_t HDMA_dest_lo;
		uint8_t HDMA_byte;
		uint32_t HDMA_tick;
		// the first read on GBA (and first two on GBC) encounter access glitches if the source address is VRAM
		uint8_t HDMA_VRAM_access_glitch;

		uint16_t cur_DMA_src;
		uint16_t cur_DMA_dest;

		// controls for tile attributes
		uint32_t VRAM_sel;
		uint32_t LYC_cd;
		uint32_t HDMA_length;
		uint32_t HDMA_countdown;
		uint32_t HBL_HDMA_count;
		uint32_t last_HBL;

		// individual byte used in palette colors
		uint8_t BG_bytes[64] = { };
		uint8_t OBJ_bytes[64] = { };



		// Helper functions
		inline bool LCDC_Bit(uint8_t bit) { return (LCDC & (0x1 << bit)) == (0x1 << bit); }

		inline bool STAT_Bit(uint8_t bit) { return (STAT & (0x1 << bit)) == (0x1 << bit); }

		inline uint8_t BG_pal_ret() { return (uint8_t)(((BG_bytes_inc ? 1 : 0) << 7) | (BG_bytes_index & 0x3F) | 0x40); }

		inline uint8_t OBJ_pal_ret() { return (uint8_t)(((OBJ_bytes_inc ? 1 : 0) << 7) | (OBJ_bytes_index & 0x3F) | 0x40); }

		inline uint8_t HDMA_ctrl() { return (uint8_t)(((HDMA_active ? 0 : 1) << 7) | ((HDMA_length >> 4) - 1)); }



		virtual uint8_t ReadReg(uint16_t addr)
		{
			return 0;
		}

		virtual void WriteReg(uint16_t addr, uint8_t value)
		{

		}

		virtual void tick()
		{

		}

		// might be needed, not sure yet
		virtual void latch_delay()
		{

		}

		virtual void render(uint16_t render_cycle)
		{

		}

		virtual void process_sprite()
		{

		}

		// normal DMA moves twice as fast in double speed mode on GBC
		// So give it it's own function so we can seperate it from PPU tick
		virtual void DMA_tick()
		{

		}

		virtual void OAM_scan(uint16_t OAM_cycle)
		{

		}

		virtual void Reset()
		{

		}

		// order sprites according to x coordinate
		// note that for sprites of equal x coordinate, priority goes to first on the list
		virtual void reorder_and_assemble_sprites()
		{

		}


		uint8_t Ex_RAM[0x400] = { };

		// always 32 k available, but most games only have access to 8
		uint8_t VRAM[0x8000] = { };

		// used by ex mmc3 in 4 screen mode, replaces internal CIRAM
		uint8_t EXT_CIRAM[0x2000] = { };

		bool* Core_show_bg_new = nullptr;

		bool* Core_show_obj_new = nullptr;

		bool* Core_ppu_OBJ_Size_16 = nullptr;

		uint64_t* Core_Cycle_Count = nullptr;

		uint64_t* Core_Clock_Update_Cycle = nullptr;

		uint8_t* Core_Cart_RAM = nullptr;

		uint32_t* Core_Cart_RAM_Length = nullptr;

		uint8_t* Core_ROM[4] = { nullptr, nullptr, nullptr, nullptr };

		uint8_t* Core_ROM_Base = nullptr;

		uint8_t* Core_CIRAM_Base = nullptr;

		uint32_t* Core_ROM_Length = nullptr;

		uint32_t* Core_status_sl = nullptr;

		uint32_t* Core_status_cycle = nullptr;

		uint32_t* Core_PPU_Phase = nullptr;

		uint8_t* Core_DB = nullptr;

		uint8_t* Core_CHR_ROM = nullptr;

		uint8_t* Core_CIRAM[4] = { nullptr, nullptr, nullptr, nullptr };

		uint32_t* Core_CHR_ROM_Length = nullptr;

		string* Core_Message_String = nullptr;

		void (*RumbleCallback)(bool);

		void (*MessageCallback)(int);

		PPUs()
		{
			Reset();
		}


	#pragma endregion

	#pragma region State Save / Load

		uint8_t* SaveState(uint8_t* saver)
		{
			saver = bool_saver(HDMA_active, saver);
			saver = bool_saver(clear_screen, saver);
			saver = bool_saver(rendering_complete, saver);
			saver = bool_saver(DMA_start, saver);
			saver = bool_saver(DMA_bus_control, saver);
			saver = bool_saver(LYC_INT, saver);
			saver = bool_saver(HBL_INT, saver);
			saver = bool_saver(VBL_INT, saver);
			saver = bool_saver(OAM_INT, saver);
			saver = bool_saver(stat_line, saver);
			saver = bool_saver(stat_line_old, saver);
			saver = bool_saver(LCD_was_off, saver);
			saver = bool_saver(no_scan, saver);
			saver = bool_saver(DMA_OAM_access, saver);
			saver = bool_saver(OAM_access_read, saver);
			saver = bool_saver(OAM_access_write, saver);
			saver = bool_saver(VRAM_access_read, saver);
			saver = bool_saver(VRAM_access_write, saver);
			saver = bool_saver(VRAM_access_read_PPU, saver);
			saver = bool_saver(VRAM_access_write_PPU, saver);
			saver = bool_saver(fetch_sprite, saver);
			saver = bool_saver(going_to_fetch, saver);
			saver = bool_saver(first_fetch, saver);
			saver = bool_saver(pre_render, saver);
			saver = bool_saver(pre_render_2, saver);
			saver = bool_saver(latch_new_data, saver);
			saver = bool_saver(no_sprites, saver);
			saver = bool_saver(window_pre_render, saver);
			saver = bool_saver(window_started, saver);
			saver = bool_saver(window_is_reset, saver);
			saver = bool_saver(blank_frame, saver);
			saver = bool_saver(window_latch, saver);
			saver = bool_saver(was_pre_render, saver);
			saver = bool_saver(pal_change_blocked, saver); // in compatability mode, you can change palette values but not displayed color
			saver = bool_saver(glitch_state, saver); // writing to STAT to enable HBL interrupt won't trigger it if the ppu just turned on
			saver = bool_saver(in_vbl, saver); // writes to turn on mode 2 stat interrupts can trigger vbl stat at the start of vbl

			// register variables
			saver = byte_saver(LCDC, saver);
			saver = byte_saver(STAT, saver);
			saver = byte_saver(scroll_y, saver);
			saver = byte_saver(scroll_x, saver);
			saver = byte_saver(LY, saver);
			saver = byte_saver(LY_actual, saver);
			saver = byte_saver(LY_inc, saver);
			saver = byte_saver(LYC, saver);
			saver = byte_saver(DMA_addr, saver);
			saver = byte_saver(BGP, saver);
			saver = byte_saver(obj_pal_0, saver);
			saver = byte_saver(obj_pal_1, saver);
			saver = byte_saver(window_y, saver);
			saver = byte_saver(window_x, saver);
			saver = byte_saver(window_y_read, saver);
			saver = byte_saver(window_x_read, saver);
			saver = byte_saver(DMA_byte, saver);

			// TODO: need a test ROM for the details here
			saver = int_saver(bus_address, saver);
			saver = int_saver(DMA_clock, saver);
			saver = int_saver(DMA_inc, saver);
			saver = int_saver(cycle, saver);
			saver = int_saver(OAM_scan_index, saver);
			saver = int_saver(SL_sprites_index, saver);
			saver = int_saver(write_sprite, saver);
			saver = int_saver(read_case, saver);
			saver = int_saver(internal_cycle, saver);
			saver = int_saver(y_tile, saver);
			saver = int_saver(y_scroll_offset, saver);
			saver = int_saver(x_tile, saver);
			saver = int_saver(x_scroll_offset, saver);
			saver = int_saver(tile_byte, saver);
			saver = int_saver(sprite_fetch_counter, saver);
			saver = int_saver(temp_fetch, saver);
			saver = int_saver(tile_inc, saver);
			saver = int_saver(latch_counter, saver);
			saver = int_saver(render_counter, saver);
			saver = int_saver(render_offset, saver);
			saver = int_saver(scroll_offset, saver);
			saver = int_saver(pixel_counter, saver);
			saver = int_saver(pixel, saver);
			saver = int_saver(sl_use_index, saver);
			saver = int_saver(evaled_sprites, saver);
			saver = int_saver(sprite_ordered_index, saver);
			saver = int_saver(consecutive_sprite, saver);
			saver = int_saver(last_eval, saver);
			saver = int_saver(window_counter, saver);
			saver = int_saver(window_tile_inc, saver);
			saver = int_saver(window_y_tile, saver);
			saver = int_saver(window_x_tile, saver);
			saver = int_saver(window_y_tile_inc, saver);
			saver = int_saver(window_x_latch, saver);
			saver = int_saver(window_y_latch, saver);
			saver = int_saver(hbl_countdown, saver);
			saver = int_saver(sprite_scroll_offset, saver);
			saver = int_saver(read_case_prev, saver);
			saver = int_saver(LYC_offset, saver); // in double speed mode it appears timing changes for LYC int
			saver = int_saver(LY_153_change, saver); // the timing of LYC chaning to 153 looks like it varies with speed mode
			saver = int_saver(total_counter, saver);

			saver = byte_array_saver(sprite_attr_list, saver, 160);
			saver = byte_array_saver(sprite_pixel_list, saver, 160);
			saver = byte_array_saver(sprite_present_list, saver, 160);
			saver = byte_array_saver(tile_data, saver, 3);
			saver = byte_array_saver(tile_data_latch, saver, 3);
			saver = byte_array_saver(sprite_data, saver, 2);
			saver = byte_array_saver(sprite_sel, saver, 2);

			saver = int_array_saver(color_palette, saver, 4);
			saver = int_array_saver(BG_palette, saver, 32);
			saver = int_array_saver(OBJ_palette, saver, 32);
			saver = int_array_saver(SL_sprites, saver, 40);
			saver = int_array_saver(SL_sprites_ordered, saver, 40);

			return saver;
		}

		uint8_t* LoadState(uint8_t* loader)
		{
			loader = bool_loader(&HDMA_active, loader);
			loader = bool_loader(&clear_screen, loader);
			loader = bool_loader(&rendering_complete, loader);
			loader = bool_loader(&DMA_start, loader);
			loader = bool_loader(&DMA_bus_control, loader);
			loader = bool_loader(&LYC_INT, loader);
			loader = bool_loader(&HBL_INT, loader);
			loader = bool_loader(&VBL_INT, loader);
			loader = bool_loader(&OAM_INT, loader);
			loader = bool_loader(&stat_line, loader);
			loader = bool_loader(&stat_line_old, loader);
			loader = bool_loader(&LCD_was_off, loader);
			loader = bool_loader(&no_scan, loader);
			loader = bool_loader(&DMA_OAM_access, loader);
			loader = bool_loader(&OAM_access_read, loader);
			loader = bool_loader(&OAM_access_write, loader);
			loader = bool_loader(&VRAM_access_read, loader);
			loader = bool_loader(&VRAM_access_write, loader);
			loader = bool_loader(&VRAM_access_read_PPU, loader);
			loader = bool_loader(&VRAM_access_write_PPU, loader);
			loader = bool_loader(&fetch_sprite, loader);
			loader = bool_loader(&going_to_fetch, loader);
			loader = bool_loader(&first_fetch, loader);
			loader = bool_loader(&pre_render, loader);
			loader = bool_loader(&pre_render_2, loader);
			loader = bool_loader(&latch_new_data, loader);
			loader = bool_loader(&no_sprites, loader);
			loader = bool_loader(&window_pre_render, loader);
			loader = bool_loader(&window_started, loader);
			loader = bool_loader(&window_is_reset, loader);
			loader = bool_loader(&blank_frame, loader);
			loader = bool_loader(&window_latch, loader);
			loader = bool_loader(&was_pre_render, loader);
			loader = bool_loader(&pal_change_blocked, loader); // in compatability mode, you can change palette values but not displayed color
			loader = bool_loader(&glitch_state, loader); // writing to STAT to enable HBL interrupt won't trigger it if the ppu just turned on
			loader = bool_loader(&in_vbl, loader); // writes to turn on mode 2 stat interrupts can trigger vbl stat at the start of vbl

			// register variables
			loader = byte_loader(&LCDC, loader);
			loader = byte_loader(&STAT, loader);
			loader = byte_loader(&scroll_y, loader);
			loader = byte_loader(&scroll_x, loader);
			loader = byte_loader(&LY, loader);
			loader = byte_loader(&LY_actual, loader);
			loader = byte_loader(&LY_inc, loader);
			loader = byte_loader(&LYC, loader);
			loader = byte_loader(&DMA_addr, loader);
			loader = byte_loader(&BGP, loader);
			loader = byte_loader(&obj_pal_0, loader);
			loader = byte_loader(&obj_pal_1, loader);
			loader = byte_loader(&window_y, loader);
			loader = byte_loader(&window_x, loader);
			loader = byte_loader(&window_y_read, loader);
			loader = byte_loader(&window_x_read, loader);
			loader = byte_loader(&DMA_byte, loader);

			// TODO: need a test ROM for the details here
			loader = int_loader(&bus_address, loader);
			loader = int_loader(&DMA_clock, loader);
			loader = int_loader(&DMA_inc, loader);
			loader = int_loader(&cycle, loader);
			loader = int_loader(&OAM_scan_index, loader);
			loader = int_loader(&SL_sprites_index, loader);
			loader = int_loader(&write_sprite, loader);
			loader = int_loader(&read_case, loader);
			loader = int_loader(&internal_cycle, loader);
			loader = int_loader(&y_tile, loader);
			loader = int_loader(&y_scroll_offset, loader);
			loader = int_loader(&x_tile, loader);
			loader = int_loader(&x_scroll_offset, loader);
			loader = int_loader(&tile_byte, loader);
			loader = int_loader(&sprite_fetch_counter, loader);
			loader = int_loader(&temp_fetch, loader);
			loader = int_loader(&tile_inc, loader);
			loader = int_loader(&latch_counter, loader);
			loader = int_loader(&render_counter, loader);
			loader = int_loader(&render_offset, loader);
			loader = int_loader(&scroll_offset, loader);
			loader = int_loader(&pixel_counter, loader);
			loader = int_loader(&pixel, loader);
			loader = int_loader(&sl_use_index, loader);
			loader = int_loader(&evaled_sprites, loader);
			loader = int_loader(&sprite_ordered_index, loader);
			loader = int_loader(&consecutive_sprite, loader);
			loader = int_loader(&last_eval, loader);
			loader = int_loader(&window_counter, loader);
			loader = int_loader(&window_tile_inc, loader);
			loader = int_loader(&window_y_tile, loader);
			loader = int_loader(&window_x_tile, loader);
			loader = int_loader(&window_y_tile_inc, loader);
			loader = int_loader(&window_x_latch, loader);
			loader = int_loader(&window_y_latch, loader);
			loader = int_loader(&hbl_countdown, loader);
			loader = int_loader(&sprite_scroll_offset, loader);
			loader = int_loader(&read_case_prev, loader);
			loader = int_loader(&LYC_offset, loader); // in double speed mode it appears timing changes for LYC int
			loader = int_loader(&LY_153_change, loader); // the timing of LYC chaning to 153 looks like it varies with speed mode
			loader = int_loader(&total_counter, loader);
	
			loader = byte_array_loader(sprite_attr_list, loader, 160);
			loader = byte_array_loader(sprite_pixel_list, loader, 160);
			loader = byte_array_loader(sprite_present_list, loader, 160);
			loader = byte_array_loader(tile_data, loader, 3);
			loader = byte_array_loader(tile_data_latch, loader, 3);
			loader = byte_array_loader(sprite_data, loader, 2);
			loader = byte_array_loader(sprite_sel, loader, 2);

			loader = int_array_loader(color_palette, loader, 4);
			loader = int_array_loader(BG_palette, loader, 32);
			loader = int_array_loader(OBJ_palette, loader, 32);
			loader = int_array_loader(SL_sprites, loader, 40);
			loader = int_array_loader(SL_sprites_ordered, loader, 40);

			return loader;
		}

		uint8_t* bool_saver(bool to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save ? 1 : 0); saver++;

			return saver;
		}

		uint8_t* byte_saver(uint8_t to_save, uint8_t* saver)
		{
			*saver = to_save; saver++;

			return saver;
		}

		uint8_t* short_saver(uint16_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;

			return saver;
		}

		uint8_t* int_saver(uint32_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;

			return saver;
		}

		uint8_t* byte_array_saver(uint8_t* to_save, uint8_t* saver, uint32_t length)
		{
			for (uint32_t i = 0; i < length; i++) { *saver = to_save[i]; saver++; }

			return saver;
		}

		uint8_t* int_array_saver(uint32_t* to_save, uint8_t* saver, uint32_t length)
		{
			for (uint32_t i = 0; i < length; i++)
			{
				*saver = (uint8_t)(to_save[i] & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 8) & 0xFF); saver++;
				*saver = (uint8_t)((to_save[i] >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save[i] >> 24) & 0xFF); saver++;
			}

			return saver;
		}

		uint8_t* long_saver(uint64_t to_save, uint8_t* saver)
		{
			*saver = (uint8_t)(to_save & 0xFF); saver++; *saver = (uint8_t)((to_save >> 8) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 16) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 24) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 32) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 40) & 0xFF); saver++;
			*saver = (uint8_t)((to_save >> 48) & 0xFF); saver++; *saver = (uint8_t)((to_save >> 56) & 0xFF); saver++;

			return saver;
		}

		uint8_t* bool_loader(bool* to_load, uint8_t* loader)
		{
			to_load[0] = *loader == 1; loader++;

			return loader;
		}

		uint8_t* byte_loader(uint8_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;

			return loader;
		}

		uint8_t* short_loader(uint16_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++;
			to_load[0] |= ((uint16_t)(*loader) << 8); loader++;

			return loader;
		}

		uint8_t* int_loader(uint32_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (uint32_t)(*loader << 8); loader++;
			to_load[0] |= (uint32_t)(*loader << 16); loader++; to_load[0] |= (uint32_t)(*loader << 24); loader++;

			return loader;
		}

		uint8_t* long_loader(uint64_t* to_load, uint8_t* loader)
		{
			to_load[0] = *loader; loader++; to_load[0] |= (uint64_t)(*loader) << 8; loader++;
			to_load[0] |= (uint64_t)(*loader) << 16; loader++; to_load[0] |= (uint64_t)(*loader) << 24; loader++;
			to_load[0] |= (uint64_t)(*loader) << 32; loader++; to_load[0] |= (uint64_t)(*loader) << 40; loader++;
			to_load[0] |= (uint64_t)(*loader) << 48; loader++; to_load[0] |= (uint64_t)(*loader) << 56; loader++;

			return loader;
		}

		uint8_t* byte_array_loader(uint8_t* to_load, uint8_t* loader, uint32_t length)
		{
			for (uint32_t i = 0; i < length; i++) { to_load[i] = *loader; loader++; }

			return loader;
		}

		uint8_t* int_array_loader(uint32_t* to_load, uint8_t* loader, uint32_t length)
		{
			for (uint32_t i = 0; i < length; i++)
			{
				to_load[i] = *loader; loader++; to_load[i] |= ((uint32_t)(*loader) << 8); loader++;
				to_load[i] |= ((uint32_t)(*loader) << 16); loader++; to_load[i] |= ((uint32_t)(*loader) << 24); loader++;
			}

			return loader;
		}

	#pragma endregion

	};
}

#endif
