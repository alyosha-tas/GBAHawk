using BizHawk.Common.NumberExtensions;
using BizHawk.Emulation.Common;
using System;
using System.Runtime.InteropServices;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	public partial class GBHawkOld : IEmulator, IVideoProvider
	{
		public void do_frame(IController controller)
		{
			for (int i = 0; i < 70224; i++)
			{
				// These things do not change speed in GBC double speed mode
				audio.tick();
				ppu.tick();
				if (Use_MT) { mapper.Mapper_Tick(); }

				// These things all tick twice as fast in GBC double speed mode
				// Note that DMA is halted when the CPU is halted
				if (double_speed)
				{
					if (ppu.DMA_start && !cpu.halted && !cpu.stopped) { ppu.DMA_tick(); }
					serialport.serial_transfer_tick();

					// check state before changes from cpu writes
					DIV_edge_old = (timer.divider_reg & 0x2000) == 0x2000;

					timer.tick();
					cpu.ExecuteOne();
					timer.divider_reg++;

					DIV_falling_edge |= DIV_edge_old & ((timer.divider_reg & 0x2000) == 0);

					if (delays_to_process) { process_delays(); }

					REG_FF0F_OLD = REG_FF0F;
				}

				if (ppu.DMA_start && !cpu.halted && !cpu.stopped) { ppu.DMA_tick(); }
				serialport.serial_transfer_tick();

				// check state before changes from cpu writes
				DIV_edge_old = double_speed ? ((timer.divider_reg & 0x2000) == 0x2000) : ((timer.divider_reg & 0x1000) == 0x1000);

				timer.tick();
				cpu.ExecuteOne();
				timer.divider_reg++;

				DIV_falling_edge |= DIV_edge_old & (double_speed ? ((timer.divider_reg & 0x2000) == 0) : ((timer.divider_reg & 0x1000) == 0));

				if (delays_to_process) { process_delays(); }

				CycleCount++;
				cpu.test_cycles++;

				REG_FF0F_OLD = REG_FF0F;
			}

			// turn off the screen so the image doesnt persist
			// but don't turn off blank_frame yet, it still needs to be true until the next VBL
			// GBC games need to clear slow enough that games that turn the screen off briefly for cinematics still look smooth
			if (ppu.clear_screen)
			{
				clear_screen_func();
			}
		}

		public void on_vblank()
		{
			vblank_rise = true;
			
			// subframe / multi cores handle these things on their own
			if (!is_subframe_core && !is_multi_core)
			{
				_islag = false;

				controller_was_checked = true;

				// update the controller state on VBlank
				GetControllerState(frame_controller);

				// check if controller state caused interrupt
				do_controller_check();

				// send the image on VBlank
				SendVideoBuffer();

				if (_settings.VBL_sync)
				{
					for (int j = 0; j < 0x8000; j++) { RAM_vbls[j] = RAM[j]; }
					for (int j = 0; j < 0x4000; j++) { VRAM_vbls[j] = VRAM[j]; }
					for (int j = 0; j < 0x80; j++) { ZP_RAM_vbls[j] = ZP_RAM[j]; }
					for (int j = 0; j < 0xA0; j++) { OAM_vbls[j] = OAM[j]; }

					if (cart_RAM != null)
					{
						for (int j = 0; j < cart_RAM.Length; j++) { cart_RAM_vbls[j] = cart_RAM[j]; }
					}
				}

				vblank_rise = false;
			}
			else if (is_multi_core)
			{
				controller_state = multi_core_controller_byte;

				do_controller_check();

				SendVideoBuffer();
			}
			else
			{
				SendVideoBuffer();

				vblank_rise = false;
			}
		}

		public void do_single_step()
		{
			// These things do not change speed in GBC double spped mode
			audio.tick();
			ppu.tick();
			if (Use_MT) { mapper.Mapper_Tick(); }

			// These things all tick twice as fast in GBC double speed mode
			// Note that DMA is halted when the CPU is halted
			if (double_speed)
			{
				if (ppu.DMA_start && !cpu.halted && !cpu.stopped) { ppu.DMA_tick(); }
				serialport.serial_transfer_tick();

				// check state before changes from cpu writes
				DIV_edge_old = (timer.divider_reg & 0x2000) == 0x2000;

				timer.tick();
				cpu.ExecuteOne();
				timer.divider_reg++;

				DIV_falling_edge |= DIV_edge_old & ((timer.divider_reg & 0x2000) == 0);

				if (delays_to_process) { process_delays(); }

				REG_FF0F_OLD = REG_FF0F;
			}

			if (ppu.DMA_start && !cpu.halted && !cpu.stopped) { ppu.DMA_tick(); }
			serialport.serial_transfer_tick();

			// check state before changes from cpu writes
			DIV_edge_old = double_speed ? ((timer.divider_reg & 0x2000) == 0x2000) : ((timer.divider_reg & 0x1000) == 0x1000);

			timer.tick();
			cpu.ExecuteOne();
			timer.divider_reg++;

			DIV_falling_edge |= DIV_edge_old & (double_speed ? ((timer.divider_reg & 0x2000) == 0) : ((timer.divider_reg & 0x1000) == 0));

			if (delays_to_process) { process_delays(); }

			REG_FF0F_OLD = REG_FF0F;
		}

	}
}
