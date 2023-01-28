﻿using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		private void SyncState(Serializer ser)
		{
			ser.BeginSection("Gameboy Advance");
			cpu_SyncState(ser);
			mapper.SyncState(ser);
			tim_SyncState(ser);
			ppu_SyncState(ser);
			ser_SyncState(ser);
			snd_SyncState(ser);
			dma_SyncState(ser);
			pre_SyncState(ser);

			ser.Sync(nameof(Lag_Count), ref Lag_Count);
			ser.Sync(nameof(Frame_Count), ref Frame_Count);
			ser.Sync(nameof(Is_Lag), ref Is_Lag);
			_controllerDeck.SyncState(ser);

			ser.Sync(nameof(Memory_CTRL), ref Memory_CTRL);
			ser.Sync(nameof(Last_BIOS_Read), ref Last_BIOS_Read);

			ser.Sync(nameof(WRAM_Waits), ref WRAM_Waits);
			ser.Sync(nameof(SRAM_Waits), ref SRAM_Waits);

			ser.Sync(nameof(ROM_Waits_0_N), ref ROM_Waits_0_N);
			ser.Sync(nameof(ROM_Waits_1_N), ref ROM_Waits_1_N);
			ser.Sync(nameof(ROM_Waits_2_N), ref ROM_Waits_2_N);
			ser.Sync(nameof(ROM_Waits_0_S), ref ROM_Waits_0_S);
			ser.Sync(nameof(ROM_Waits_1_S), ref ROM_Waits_1_S);
			ser.Sync(nameof(ROM_Waits_2_S), ref ROM_Waits_2_S);

			ser.Sync(nameof(INT_EN), ref INT_EN);
			ser.Sync(nameof(INT_Flags), ref INT_Flags);
			ser.Sync(nameof(INT_Master), ref INT_Master);
			ser.Sync(nameof(Wait_CTRL), ref Wait_CTRL);

			ser.Sync(nameof(Post_Boot), ref Post_Boot);
			ser.Sync(nameof(Halt_CTRL), ref Halt_CTRL);

			ser.Sync(nameof(All_RAM_Disable), ref All_RAM_Disable);
			ser.Sync(nameof(WRAM_Enable), ref WRAM_Enable);
			ser.Sync(nameof(INT_Master_On), ref INT_Master_On);

			ser.Sync(nameof(controller_state), ref controller_state);
			ser.Sync(nameof(Acc_X_state), ref Acc_X_state);
			ser.Sync(nameof(Acc_Y_state), ref Acc_Y_state);
			ser.Sync(nameof(VBlank_Rise), ref VBlank_Rise);
			ser.Sync(nameof(controller_was_checked), ref controller_was_checked);
			ser.Sync(nameof(delays_to_process), ref delays_to_process);
			ser.Sync(nameof(IRQ_Write_Delay), ref IRQ_Write_Delay);
			ser.Sync(nameof(IRQ_Write_Delay_2), ref IRQ_Write_Delay_2);
			ser.Sync(nameof(IRQ_Write_Delay_3), ref IRQ_Write_Delay_3);
			ser.Sync(nameof(CycleCount), ref CycleCount);

			// memory domains
			ser.Sync(nameof(WRAM), ref WRAM, false);
			ser.Sync(nameof(IWRAM), ref IWRAM, false);
			ser.Sync(nameof(PALRAM), ref PALRAM, false);
			ser.Sync(nameof(VRAM), ref VRAM, false);
			ser.Sync(nameof(OAM), ref OAM, false);

			ser.Sync(nameof(Use_MT), ref Use_MT);
			ser.Sync(nameof(addr_access), ref addr_access);

			ser.Sync(nameof(frame_buffer), ref frame_buffer, false);
			ser.Sync(nameof(vid_buffer), ref vid_buffer, false);

			// probably a better way to do this
			if (cart_RAM != null) { ser.Sync(nameof(cart_RAM), ref cart_RAM, false); }

			ser.Sync(nameof(WRAM_vbls), ref WRAM_vbls, false);
			ser.Sync(nameof(IWRAM_vbls), ref IWRAM_vbls, false);
			ser.Sync(nameof(PALRAM_vbls), ref PALRAM_vbls, false);
			ser.Sync(nameof(VRAM_vbls), ref VRAM_vbls, false);
			ser.Sync(nameof(OAM_vbls), ref OAM_vbls, false);

			if (cart_RAM != null) { ser.Sync(nameof(cart_RAM_vbls), ref cart_RAM_vbls, false); }

			ser.EndSection();
		}
	}
}