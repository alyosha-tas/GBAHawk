using System;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	public partial class GBAHawk_Debug
	{
		public byte Read_Registers_8(uint addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_8(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_8(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_8(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_8(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_8(addr);
			}

			byte ret = 0;

			switch (addr)
			{
				case 0x200: ret = (byte)(INT_EN & 0xFF); break;
				case 0x201: ret = (byte)((INT_EN & 0xFF00) >> 8); break;
				case 0x202: ret = (byte)(INT_Flags & 0xFF); break;
				case 0x203: ret = (byte)((INT_Flags & 0xFF00) >> 8); break;
				case 0x204: ret = (byte)(Wait_CTRL & 0xFF); break;
				case 0x205: ret = (byte)((Wait_CTRL & 0xFF00) >> 8); break;
				case 0x206: ret = 0; break;
				case 0x207: ret = 0; break;
				case 0x208: ret = (byte)(INT_Master & 0xFF); break;
				case 0x209: ret = (byte)((INT_Master & 0xFF00) >> 8); break;
				case 0x20A: ret = 0; break;
				case 0x20B: ret = 0; break;

				case 0x300: ret = Post_Boot; break;
				case 0x301: ret = Halt_CTRL; break;
				case 0x302: ret = 0; break;
				case 0x303: ret = 0; break;

				default: ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); break; // open bus;
			}

			return ret;
		}

		public ushort Read_Registers_16(uint addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_16(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_16(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_16(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_16(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_16(addr);
			}

			ushort ret = 0;

			switch (addr)
			{
				case 0x200: ret = INT_EN; break;
				case 0x202: ret = INT_Flags; break;
				case 0x204: ret = Wait_CTRL; break;
				case 0x206: ret = 0; break;
				case 0x208: ret = INT_Master; break;
				case 0x20A: ret = 0; break;

				case 0x300: ret = (ushort)((Halt_CTRL << 8) | Post_Boot); break;
				case 0x302: ret = 0; break;

				default: ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); break; // open bus
			}

			return ret;
		}

		public uint Read_Registers_32(uint addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_32(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_32(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_32(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_32(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_32(addr);
			}

			uint ret = 0;

			switch (addr)
			{
				case 0x200: ret = (uint)((INT_Flags << 16) | INT_EN); break;
				case 0x204: ret = (uint)(0x00000000 | Wait_CTRL); break;

				case 0x208: ret = (uint)(0x00000000 | INT_Master); break;

				case 0x300: ret = (uint)(0x00000000 | (Halt_CTRL << 8) | Post_Boot); break;

				default: ret = cpu_Last_Bus_Value; break; // open bus
			}

			return ret;
		}

		public void Write_Registers_8(uint addr, byte value)
		{
			if (addr < 0x60)
			{
				ppu_Write_Reg_8(addr, value);
			}
			else if (addr < 0xB0)
			{
				snd_Write_Reg_8(addr, value);
			}
			else if (addr < 0x100)
			{
				dma_Write_Reg_8(addr, value);
			}
			else if (addr < 0x120)
			{
				tim_Write_Reg_8(addr, value);
			}
			else if (addr < 0x200)
			{
				ser_Write_Reg_8(addr, value);
			}
			else
			{
				switch (addr)
				{
					case 0x200: Update_INT_EN((ushort)((INT_EN & 0xFF00) | value)); break;
					case 0x201: Update_INT_EN((ushort)((INT_EN & 0x00FF) | (value << 8))); break;
					case 0x202: Update_INT_Flags((ushort)((INT_Flags & 0xFF00) | value)); break;
					case 0x203: Update_INT_Flags((ushort)((INT_Flags & 0x00FF) | (value << 8))); break;
					case 0x204: Update_Wait_CTRL((ushort)((Wait_CTRL & 0xFF00) | value)); break;
					case 0x205: Update_Wait_CTRL((ushort)((Wait_CTRL & 0x00FF) | (value << 8))); break;

					case 0x208: Update_INT_Master((ushort)((INT_Master & 0xFF00) | value)); break;
					case 0x209: Update_INT_Master((ushort)((INT_Master & 0x00FF) | (value << 8))); break;

					case 0x300: Update_Post_Boot(value); break;
					case 0x301: Update_Halt_CTRL(value); break;
				}
			}
		}

		public void Write_Registers_16(uint addr, ushort value)
		{
			if (addr < 0x60)
			{
				ppu_Write_Reg_16(addr, value);
			}
			else if (addr < 0xB0)
			{
				snd_Write_Reg_16(addr, value);
			}
			else if (addr < 0x100)
			{
				dma_Write_Reg_16(addr, value);
			}
			else if (addr < 0x120)
			{
				tim_Write_Reg_16(addr, value);
			}
			else if (addr < 0x200)
			{
				ser_Write_Reg_16(addr, value);
			}
			else
			{
				switch (addr)
				{
					case 0x200: Update_INT_EN(value); break;
					case 0x202: Update_INT_Flags(value); break;
					case 0x204: Update_Wait_CTRL(value); break;

					case 0x208: Update_INT_Master(value); break;

					case 0x300: Update_Post_Boot((byte)(value & 0xFF)); Update_Halt_CTRL((byte)((value >> 8) & 0xFF)); break;
				}
			}			
		}

		public void Write_Registers_32(uint addr, uint value)
		{
			if (addr < 0x60)
			{
				ppu_Write_Reg_32(addr, value);
			}
			else if (addr < 0xB0)
			{
				snd_Write_Reg_32(addr, value);
			}
			else if (addr < 0x100)
			{
				dma_Write_Reg_32(addr, value);
			}
			else if (addr < 0x120)
			{
				tim_Write_Reg_32(addr, value);
			}
			else if (addr < 0x200)
			{
				ser_Write_Reg_32(addr, value);
			}
			else
			{
				switch (addr)
				{
					case 0x200: Update_INT_EN((ushort)(value & 0xFFFF));
								Update_INT_Flags((ushort)((value >> 16) & 0xFFFF)); break;
					case 0x204: Update_Wait_CTRL((ushort)(value & 0xFFFF)); break;

					case 0x208: Update_INT_Master((ushort)(value & 0xFFFF)); break;

					case 0x300: Update_Post_Boot((byte)(value & 0xFF)); Update_Halt_CTRL((byte)((value >> 8) & 0xFF)); break;
				}
			}
		}

		public void Update_INT_EN(ushort value)
		{
			// changes to IRQ that happen due to writes should take place in 3 cycles
			delays_to_process = true;
			IRQ_Write_Delay_3 = true;
			IRQ_Delays = true;

			INT_EN = value;

			//Console.WriteLine("en " + value + " " + CycleCount);
		}

		public void Update_INT_Flags(ushort value)
		{
			// writing one acknowledges interrupt at that bit
			for (int i = 0; i < 14; i++)
			{
				if ((value & (1 << i)) == (1 << i))
				{
					INT_Flags &= (ushort)(~(1 << i));

					// if a flag is set on the same cycle a write occurs, it is cleared
					INT_Flags_Gather &= (ushort)(~(1 << i));
				}
			}

			// changes to IRQ that happen due to writes should take place in 3 cycles
			delays_to_process = true;
			IRQ_Write_Delay_3 = true;
			IRQ_Delays = true;
		}

		public void Update_INT_Master(ushort value)
		{
			INT_Master_On = (value & 1) == 1;

			// changes to IRQ that happen due to writes should take place in 3 cycles
			delays_to_process = true;
			IRQ_Write_Delay_3 = true;
			IRQ_Delays = true;

			INT_Master = value;
		}

		public void Update_Wait_CTRL(ushort value)
		{
			switch (value & 3)
			{
				case 0: SRAM_Waits = 4; break;
				case 1: SRAM_Waits = 3; break;
				case 2: SRAM_Waits = 2; break;
				case 3: SRAM_Waits = 8; break;
			}

			switch ((value >> 2) & 3)
			{
				case 0: ROM_Waits_0_N = 4; break;
				case 1: ROM_Waits_0_N = 3; break;
				case 2: ROM_Waits_0_N = 2; break;
				case 3: ROM_Waits_0_N = 8; break;
			}

			switch ((value >> 4) & 1)
			{
				case 0: ROM_Waits_0_S = 2; break;
				case 1: ROM_Waits_0_S = 1; break;
			}

			switch ((value >> 5) & 3)
			{
				case 0: ROM_Waits_1_N = 4; break;
				case 1: ROM_Waits_1_N = 3; break;
				case 2: ROM_Waits_1_N = 2; break;
				case 3: ROM_Waits_1_N = 8; break;
			}

			switch ((value >> 7) & 1)
			{
				case 0: ROM_Waits_1_S = 4; break;
				case 1: ROM_Waits_1_S = 1; break;
			}

			switch ((value >> 8) & 3)
			{
				case 0: ROM_Waits_2_N = 4; break;
				case 1: ROM_Waits_2_N = 3; break;
				case 2: ROM_Waits_2_N = 2; break;
				case 3: ROM_Waits_2_N = 8; break;
			}

			switch ((value >> 10) & 1)
			{
				case 0: ROM_Waits_2_S = 8; break;
				case 1: ROM_Waits_2_S = 1; break;
			}

			pre_Reg_Write(value);

			//Console.WriteLine("Prefetch enabled: " + pre_Enable +  " ctrl value: " + value + " cycles: " + TotalExecutedCycles);

			Wait_CTRL = (ushort)(value & 0x5FFF);
		}

		public void Update_Memory_CTRL(uint value)
		{
			All_RAM_Disable = (value & 1) == 1;

			WRAM_Enable = (value & 0x20) == 0x20;

			WRAM_Waits = (int)(((~value) >> 24) & 0xF);

			Memory_CTRL = value;

			Console.WriteLine("wram: " + WRAM_Waits);
		}

		public void Update_Post_Boot(byte value)
		{
			if (Post_Boot == 0)
			{
				Post_Boot = value;
			}
		}

		public void Update_Halt_CTRL(byte value)
		{
			// appears to only be addressable from BIOS, see haltcnt.gba, timing of first test indicates no halting, which is written from outside BIOS
			if (cpu_Regs[15] <= 0x3FFF)
			{
				if ((value & 0x80) == 0)
				{
					Halt_Enter = true;
					Halt_Enter_cd = 2;
					IRQ_Delays = true;
					delays_to_process = true;
				}
				else
				{
					if (!is_linked_system && !_syncSettings.Use_GBP)
					{
						stopped = true;
						// use this to end the frame
						VBlank_Rise = true;
					}
				}
			}
		}


		public byte Peek_Registers_8(uint addr)
		{
			if (addr < 0x60)
			{
				return ppu_Read_Reg_8(addr);
			}
			else if (addr < 0xB0)
			{
				return snd_Read_Reg_8(addr);
			}
			else if (addr < 0x100)
			{
				return dma_Read_Reg_8(addr);
			}
			else if (addr < 0x120)
			{
				return tim_Read_Reg_8(addr);
			}
			else if (addr < 0x200)
			{
				return ser_Read_Reg_8(addr);
			}

			byte ret = 0;

			switch (addr)
			{
				case 0x200: ret = (byte)(INT_EN & 0xFF); break;
				case 0x201: ret = (byte)((INT_EN & 0xFF00) >> 8); break;
				case 0x202: ret = (byte)(INT_Flags & 0xFF); break;
				case 0x203: ret = (byte)((INT_Flags & 0xFF00) >> 8); break;
				case 0x204: ret = (byte)(Wait_CTRL & 0xFF); break;
				case 0x205: ret = (byte)((Wait_CTRL & 0xFF00) >> 8); break;

				case 0x208: ret = (byte)(INT_Master & 0xFF); break;
				case 0x209: ret = (byte)((INT_Master & 0xFF00) >> 8); break;

				case 0x300: ret = Post_Boot; break;
				case 0x301: ret = Halt_CTRL; break;

				default: ret = 0xFF; break;
			}

			return ret;
		}
	}
}
