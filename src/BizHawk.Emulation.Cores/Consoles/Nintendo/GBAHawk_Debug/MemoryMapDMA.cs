using BizHawk.Emulation.Common;
using System;

/*
	$10000000-$FFFFFFFF    Unmapped
	$0E010000-$0FFFFFFF    Unmapped
	$0E000000-$0E00FFFF    SRAM
	$0C000000-$0DFFFFFF    ROM - Wait State 2
	$0A000000-$0BFFFFFF    ROM - Wait State 1
	$08000000-$09FFFFFF    ROM - Wait State 0
	$07000400-$07FFFFFF    Unmapped
	$07000000-$070003FF    OAM
	$06018000-$06FFFFFF    Unmapped
	$06000000-$06017FFF    VRAM
	$05000400-$05FFFFFF    Unmapped
	$05000000-$050003FF    Palette RAM
	$04XX0800-$04XX0800    Mem Control
	$04000000-$040007FF    I/O Regs
	$03008000-$03FFFFFF    Unmapped
	$03000000-$03007FFF    IWRAM
	$02040000-$02FFFFFF    Unmapped
	$02000000-$0203FFFF    WRAM
	$00004000-$01FFFFFF    Unmapped
	$00000000-$00003FFF    BIOS
*/

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
#pragma warning disable CS0675 // Bitwise-or operator used on a sign-extended operand

	public partial class GBAHawk_Debug
	{
		public void Read_Memory_16_DMA(uint addr, int chan)
		{
			ushort ret = 0;

			// DMA always force aligned
			addr &= 0xFFFFFFFE;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0D000000)
				{
					addr -= 0x08000000;
					ret = (ushort)((ROM[addr + 1] << 8) | ROM[addr]);
					dma_Last_Bus_Value[chan] = (uint)((ret << 16) | ret);
				}
				else if (addr < 0x0E000000)
				{
					if (!Is_EEPROM)
					{
						addr -= 0x08000000;
						ret = (ushort)((ROM[addr + 1] << 8) | ROM[addr]);
					}
					else
					{
						if (EEPROM_Wiring)
						{
							ret = (ushort)((dma_Last_Bus_Value[chan] & 0xFFFE) | mapper.Mapper_EEPROM_Read());
						}
						else
						{
							if ((addr & 0xDFFFE00) == 0xDFFFE00)
							{
								ret = (ushort)((dma_Last_Bus_Value[chan] & 0xFFFE) | mapper.Mapper_EEPROM_Read());
							}
							else
							{
								addr -= 0x08000000;
								ret = (ushort)((ROM[addr + 1] << 8) | ROM[addr]);
							}
						}
					}

					dma_Last_Bus_Value[chan] = (uint)((ret << 16) | ret);
				}
				else if (addr < 0x10000000)
				{
					ret = mapper.ReadMemory16(addr - 0x0E000000);
					dma_Last_Bus_Value[chan] = (uint)((ret << 16) | ret);
				}
			}
			else if (addr >= 0x04000000)
			{
				if (addr >= 0x07000000)
				{
					ret = (ushort)((OAM[(addr & 0x3FF) + 1] << 8) | OAM[addr & 0x3FF]);

					ppu_OAM_In_Use = false;
					ppu_Memory_In_Use = false;
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						ret = (ushort)((VRAM[(addr & 0x17FFF) + 1] << 8) | VRAM[addr & 0x17FFF]);
					}
					else
					{
						ret = (ushort)((VRAM[(addr & 0xFFFF) + 1] << 8) | VRAM[addr & 0xFFFF]);
					}

					ppu_VRAM_In_Use = false;
					ppu_Memory_In_Use = false;
				}
				else if (addr >= 0x05000000)
				{
					ret = (ushort)((PALRAM[(addr & 0x3FF) + 1] << 8) | PALRAM[addr & 0x3FF]);

					ppu_PALRAM_In_Use = false;
					ppu_Memory_In_Use = false;
				}
				else
				{
					if (addr < 0x04000800)
					{
						ret = Read_Registers_16(addr - 0x04000000);
					}
					else if ((addr & 0x0400FFFF) == 0x04000800)
					{
						switch (addr & 3)
						{
							case 0: ret = (ushort)(Memory_CTRL & 0xFFFF); break;
							default: ret = (ushort)((Memory_CTRL >> 16) & 0xFFFF); break;
						}
					}
				}

				dma_Last_Bus_Value[chan] = (uint)((ret << 16) | ret);
			}
			else if (addr >= 0x03000000)
			{
				ret = (ushort)((IWRAM[(addr & 0x7FFF) + 1] << 8) | IWRAM[addr & 0x7FFF]);
				dma_Last_Bus_Value[chan] = (uint)((ret << 16) | ret);
			}
			else if (addr >= 0x02000000)
			{
				ret = (ushort)((WRAM[(addr & 0x3FFFF) + 1] << 8) | WRAM[addr & 0x3FFFF]);
				dma_Last_Bus_Value[chan] = (uint)((ret << 16) | ret);
			}

			// DMA cannot access BIOS, nothing here	
		}

		public void Read_Memory_32_DMA(uint addr, int chan)
		{
			// DMA always force aligned
			addr &= 0xFFFFFFFC;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0D000000)
				{
					addr -= 0x08000000;
					dma_Last_Bus_Value[chan] = (uint)((ROM[addr + 3] << 24) |
													  (ROM[addr + 2] << 16) |
													  (ROM[addr + 1] << 8) |
													   ROM[addr]);
				}
				else if (addr < 0x0E000000)
				{
					if (!Is_EEPROM)
					{
						addr -= 0x08000000;
						dma_Last_Bus_Value[chan] = (uint)((ROM[addr + 3] << 24) |
														  (ROM[addr + 2] << 16) |
														  (ROM[addr + 1] << 8) |
														   ROM[addr]);
					}
					else
					{
						if (EEPROM_Wiring)
						{
							dma_Last_Bus_Value[chan] = (uint)((dma_Last_Bus_Value[chan] & 0xFFFFFFFE) | mapper.Mapper_EEPROM_Read());
						}
						else
						{
							if ((addr & 0xDFFFE00) == 0xDFFFE00)
							{
								dma_Last_Bus_Value[chan] = (uint)((dma_Last_Bus_Value[chan] & 0xFFFFFFFE) | mapper.Mapper_EEPROM_Read());
							}
							else
							{
								addr -= 0x08000000;
								dma_Last_Bus_Value[chan] = (uint)((ROM[addr + 3] << 24) |
																  (ROM[addr + 2] << 16) |
																  (ROM[addr + 1] << 8) |
																   ROM[addr]);
							}
						}
					}
				}
				else if (addr < 0x10000000)
				{
					dma_Last_Bus_Value[chan] = mapper.ReadMemory32(addr - 0x0E000000);
				}
			}
			else if (addr >= 0x04000000)
			{
				if (addr >= 0x07000000)
				{
					dma_Last_Bus_Value[chan] = (uint)((OAM[(addr & 0x3FF) + 3] << 24) |
													  (OAM[(addr & 0x3FF) + 2] << 16) |
													  (OAM[(addr & 0x3FF) + 1] << 8) |
													   OAM[addr & 0x3FF]);

					ppu_OAM_In_Use = false;
					ppu_Memory_In_Use = false;
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						dma_Last_Bus_Value[chan] = (uint)((VRAM[(addr & 0x17FFF) + 3] << 24) |
														  (VRAM[(addr & 0x17FFF) + 2] << 16) |
														  (VRAM[(addr & 0x17FFF) + 1] << 8) |
														   VRAM[addr & 0x17FFF]);
					}
					else
					{
						dma_Last_Bus_Value[chan] = (uint)((VRAM[(addr & 0xFFFF) + 3] << 24) |
														  (VRAM[(addr & 0xFFFF) + 2] << 16) |
														  (VRAM[(addr & 0xFFFF) + 1] << 8) |
														   VRAM[addr & 0xFFFF]);
					}

					ppu_VRAM_In_Use = false;
					ppu_Memory_In_Use = false;
				}
				else if (addr >= 0x05000000)
				{
					dma_Last_Bus_Value[chan] = (uint)((PALRAM[(addr & 0x3FF) + 3] << 24) |
													  (PALRAM[(addr & 0x3FF) + 2] << 16) |
													  (PALRAM[(addr & 0x3FF) + 1] << 8) |
													   PALRAM[addr & 0x3FF]);

					ppu_PALRAM_In_Use = false;
					ppu_Memory_In_Use = false;
				}
				else
				{
					if (addr < 0x04000800)
					{
						dma_Last_Bus_Value[chan] = Read_Registers_32(addr - 0x04000000);
					}
					else if ((addr & 0x0400FFFF) == 0x04000800)
					{
						dma_Last_Bus_Value[chan] = Memory_CTRL;
					}
				}
			}
			else if (addr >= 0x03000000)
			{
				dma_Last_Bus_Value[chan] = (uint)((IWRAM[(addr & 0x7FFF) + 3] << 24) |
												  (IWRAM[(addr & 0x7FFF) + 2] << 16) |
												  (IWRAM[(addr & 0x7FFF) + 1] << 8) |
												   IWRAM[addr & 0x7FFF]);
			}
			else if (addr >= 0x02000000)
			{
				dma_Last_Bus_Value[chan] = (uint)((WRAM[(addr & 0x3FFFF) + 3] << 24) |
												  (WRAM[(addr & 0x3FFFF) + 2] << 16) |
												  (WRAM[(addr & 0x3FFFF) + 1] << 8) |
												   WRAM[addr & 0x3FFFF]);
			}

			// DMA cannot access the BIOS, so nothing here	
		}

		public void Write_Memory_16_DMA(uint addr, ushort value, int chan)
		{
			// DMA always force aligned
			addr &= 0xFFFFFFFE;
		
			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					WRAM[addr & 0x3FFFF] = (byte)(value & 0xFF);
					WRAM[(addr & 0x3FFFF) + 1] = (byte)((value >> 8) & 0xFF);
				}
			}
			else if (addr < 0x04000000)
			{
				IWRAM[addr & 0x7FFF] = (byte)(value & 0xFF);
				IWRAM[(addr & 0x7FFF) + 1] = (byte)((value >> 8) & 0xFF);
			}
			else if (addr < 0x05000000)
			{
				if (addr < 0x04000800)
				{
					Write_Registers_16(addr - 0x04000000, value);
				}
				else if ((addr & 0x0400FFFF) == 0x04000800)
				{
					switch (addr & 3)
					{
						case 0x00: Update_Memory_CTRL((uint)((Memory_CTRL & 0xFFFF0000) | value)); break;
						default: Update_Memory_CTRL((uint)((Memory_CTRL & 0x0000FFFF) | (value << 16))); break;
					}
				}
			}
			else if (addr < 0x06000000)
			{
				PALRAM[addr & 0x3FF] = (byte)(value & 0xFF);
				PALRAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);

				ppu_PALRAM_In_Use = false;
				ppu_Memory_In_Use = false;
			}
			else if (addr < 0x07000000)
			{
				if ((addr & 0x00010000) == 0x00010000)
				{
					VRAM[addr & 0x17FFF] = (byte)(value & 0xFF);
					VRAM[(addr & 0x17FFF) + 1] = (byte)((value >> 8) & 0xFF);
				}
				else
				{
					VRAM[addr & 0xFFFF] = (byte)(value & 0xFF);
					VRAM[(addr & 0xFFFF) + 1] = (byte)((value >> 8) & 0xFF);
				}

				ppu_VRAM_In_Use = false;
				ppu_Memory_In_Use = false;
			}
			else if (addr < 0x08000000)
			{
				OAM[addr & 0x3FF] = (byte)(value & 0xFF);
				OAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);

				// if writing to OAM outside of VBlank, update rotation parameters
				if (((ppu_STAT & 0x1) == 0) && !ppu_Forced_Blank)
				{
					ppu_Calculate_Sprites_Pixels(addr & 0x3FF, false);
				}

				ppu_OAM_In_Use = false;
				ppu_Memory_In_Use = false;
			}
			else if ((addr >= 0x0D000000) && (addr < 0x0E000000))
			{
				if (Is_EEPROM)
				{
					if (EEPROM_Wiring)
					{
						mapper.Mapper_EEPROM_Write((byte)value);
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							mapper.Mapper_EEPROM_Write((byte)value);
						}
					}
				}
			}
			else if ((addr >= 0x0E000000) && (addr < 0x10000000))
			{
				mapper.WriteMemory16(addr - 0x0E000000, value);
			}
		}

		public void Write_Memory_32_DMA(uint addr, uint value, int chan)
		{
			// DMA always force aligned
			addr &= 0xFFFFFFFC;
			
			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					WRAM[addr & 0x3FFFF] = (byte)(value & 0xFF);
					WRAM[(addr & 0x3FFFF) + 1] = (byte)((value >> 8) & 0xFF);
					WRAM[(addr & 0x3FFFF) + 2] = (byte)((value >> 16) & 0xFF);
					WRAM[(addr & 0x3FFFF) + 3] = (byte)((value >> 24) & 0xFF);
				}
			}
			else if (addr < 0x04000000)
			{
				IWRAM[addr & 0x7FFF] = (byte)(value & 0xFF);
				IWRAM[(addr & 0x7FFF) + 1] = (byte)((value >> 8) & 0xFF);
				IWRAM[(addr & 0x7FFF) + 2] = (byte)((value >> 16) & 0xFF);
				IWRAM[(addr & 0x7FFF) + 3] = (byte)((value >> 24) & 0xFF);
			}
			else if (addr < 0x05000000)
			{
				if (addr < 0x04000800)
				{
					Write_Registers_32(addr - 0x04000000, value);
				}
				else if ((addr & 0x0400FFFF) == 0x04000800)
				{
					Update_Memory_CTRL(value);
				}
			}
			else if (addr < 0x06000000)
			{
				PALRAM[addr & 0x3FF] = (byte)(value & 0xFF);
				PALRAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);
				PALRAM[(addr & 0x3FF) + 2] = (byte)((value >> 16) & 0xFF);
				PALRAM[(addr & 0x3FF) + 3] = (byte)((value >> 24) & 0xFF);

				ppu_PALRAM_In_Use = false;
				ppu_Memory_In_Use = false;
			}
			else if (addr < 0x07000000)
			{
				if ((addr & 0x00010000) == 0x00010000)
				{
					VRAM[addr & 0x17FFF] = (byte)(value & 0xFF);
					VRAM[(addr & 0x17FFF) + 1] = (byte)((value >> 8) & 0xFF);
					VRAM[(addr & 0x17FFF) + 2] = (byte)((value >> 16) & 0xFF);
					VRAM[(addr & 0x17FFF) + 3] = (byte)((value >> 24) & 0xFF);
				}
				else
				{
					VRAM[addr & 0xFFFF] = (byte)(value & 0xFF);
					VRAM[(addr & 0xFFFF) + 1] = (byte)((value >> 8) & 0xFF);
					VRAM[(addr & 0xFFFF) + 2] = (byte)((value >> 16) & 0xFF);
					VRAM[(addr & 0xFFFF) + 3] = (byte)((value >> 24) & 0xFF);
				}

				ppu_VRAM_In_Use = false;
				ppu_Memory_In_Use = false;
			}
			else if (addr < 0x08000000)
			{
				OAM[addr & 0x3FF] = (byte)(value & 0xFF);
				OAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);
				OAM[(addr & 0x3FF) + 2] = (byte)((value >> 16) & 0xFF);
				OAM[(addr & 0x3FF) + 3] = (byte)((value >> 24) & 0xFF);

				// if writing to OAM outside of VBlank, update rotation parameters
				if (((ppu_STAT & 0x1) == 0) && !ppu_Forced_Blank)
				{
					ppu_Calculate_Sprites_Pixels(addr & 0x3FF, false);
					ppu_Calculate_Sprites_Pixels((addr + 2) & 0x3FF, false);
				}

				ppu_OAM_In_Use = false;
				ppu_Memory_In_Use = false;
			}
			else if ((addr >= 0x0D000000) && (addr < 0x0E000000))
			{
				if (Is_EEPROM)
				{
					if (EEPROM_Wiring)
					{
						mapper.Mapper_EEPROM_Write((byte)value);
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							mapper.Mapper_EEPROM_Write((byte)value);
						}
					}
				}
			}
			else if ((addr >= 0x0E000000) && (addr < 0x10000000))
			{
				mapper.WriteMemory32(addr - 0x0E000000, value);
			}
		}
	}

#pragma warning restore CS0675 // Bitwise-or operator used on a sign-extended operand
}
