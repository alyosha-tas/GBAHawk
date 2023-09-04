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
		public byte Read_Memory_8(uint addr)
		{
			byte ret = 0;

			if (addr >= 0x08000000)
			{
				if (addr < 0x0D000000)
				{
					addr -= 0x08000000;

					ret = ROM[addr];
				}
				else if (addr < 0x0E000000)
				{
					if (!Is_EEPROM)
					{
						addr -= 0x08000000;

						ret = ROM[addr];
					}
					else
					{
						if (EEPROM_Wiring)
						{
							ret = (byte)mapper.Mapper_EEPROM_Read();
						}
						else
						{
							if ((addr & 0xDFFFE00) == 0xDFFFE00)
							{
								ret = (byte)mapper.Mapper_EEPROM_Read();
							}
							else
							{
								addr -= 0x08000000;

								ret = ROM[addr];
							}
						}
					}				
				}
				else if (addr < 0x10000000)
				{
					ret = mapper.ReadMemory8(addr - 0x0E000000);
				}
				else
				{
					ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); // open bus
				}

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
			else if (addr >= 0x04000000)
			{
				if (addr >= 0x07000000)
				{
					ret = OAM[addr & 0x3FF];
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						// mirrors behave differently depending on mode
						if ((addr & 0x00008000) == 0x00008000)
						{
							if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
							{
								ret = VRAM[addr & 0x17FFF];
							}
							else
							{
								ret = 0;
							}						
						}
						else
						{
							ret = VRAM[addr & 0x17FFF];
						}					
					}
					else
					{
						ret = VRAM[addr & 0xFFFF];
					}

					ppu_VRAM_High_In_Use = false;
					ppu_VRAM_In_Use = false;
				}
				else if (addr >= 0x05000000)
				{
					ret = PALRAM[addr & 0x3FF];

					ppu_PALRAM_In_Use = false;
				}
				else
				{
					if (addr < 0x04000800)
					{
						ret = Read_Registers_8(addr - 0x04000000);
					}
					else if ((addr & 0x0400FFFF) == 0x04000800)
					{
						switch (addr & 3)
						{
							case 0: ret = (byte)(Memory_CTRL & 0xFF); break;
							case 1: ret = (byte)((Memory_CTRL >> 8) & 0xFF); break;
							case 2: ret = (byte)((Memory_CTRL >> 16) & 0xFF); break;
							default: ret = (byte)((Memory_CTRL >> 24) & 0xFF); break;
						}
					}
					else
					{
						ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); // open bus
					}
				}
			}
			else if (addr >= 0x03000000)
			{
				ret = IWRAM[addr & 0x7FFF];
			}
			else if (addr >= 0x02000000)
			{
				ret = WRAM[addr & 0x3FFFF];
			}
			else if (addr < 0x4000)
			{
				// BIOS is protected against reading from memory beyond the BIOS range
				if (cpu_Regs[15] > 0x4000)
				{
					if ((addr & 1) == 0)
					{
						ret = (byte)(Last_BIOS_Read & 0xFF);
					}
					else
					{
						ret = (byte)((Last_BIOS_Read >> 8) & 0xFF);
					}
				}
				else
				{
					ret = BIOS[addr];
				}			
			}
			else
			{
				ret = (byte)((cpu_Last_Bus_Value >> (8 * (int)(addr & 3))) & 0xFF); // open bus	
			}

			cpu_Last_Bus_Value &= (uint)~(0xFF << (((int)addr & 3) * 8));
			cpu_Last_Bus_Value |= ((uint)ret << (((int)addr & 3) * 8));

			return ret;
		}

		public ushort Read_Memory_16(uint addr)
		{
			ushort ret = 0;
			
			if (addr >= 0x08000000)
			{
				if (addr < 0x0D000000)
				{
					addr -= 0x08000000;
					// Forced Align
					addr &= 0xFFFFFFFE;

					ret = (ushort)((ROM[addr + 1] << 8) | ROM[addr]);

					// in ROM area, upper bits also used if bit 1 in the address is set
					if ((addr & 2) == 2) { cpu_Last_Bus_Value = (uint)(ret << 16);}
				}
				else if (addr < 0x0E000000)
				{
					if (!Is_EEPROM)
					{
						addr -= 0x08000000;
						// Forced Align
						addr &= 0xFFFFFFFE;

						ret = (ushort)((ROM[addr + 1] << 8) | ROM[addr]);
					}
					else
					{
						if (EEPROM_Wiring)
						{
							ret = (ushort)((cpu_Last_Bus_Value & 0xFFFE) | mapper.Mapper_EEPROM_Read());
						}
						else
						{
							if ((addr & 0xDFFFE00) == 0xDFFFE00)
							{
								ret = (ushort)((cpu_Last_Bus_Value & 0xFFFE) | mapper.Mapper_EEPROM_Read());
							}
							else
							{
								addr -= 0x08000000;
								// Forced Align
								addr &= 0xFFFFFFFE;

								ret = (ushort)((ROM[addr + 1] << 8) | ROM[addr]);
							}
						}
					}

					// in ROM area, upper bits also used if bit 1 in the address is set
					if ((addr & 2) == 2) { cpu_Last_Bus_Value = (uint)(ret << 16); }
				}
				else if (addr < 0x10000000)
				{
					ret = mapper.ReadMemory16(addr - 0x0E000000);
				}
				else
				{
					ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); // open bus
				}

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
			else if (addr >= 0x04000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFE;

				if (addr >= 0x07000000)
				{
					ret = (ushort)((OAM[(addr & 0x3FF) + 1] << 8) | OAM[addr & 0x3FF]);
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						// mirrors behave differently depending on mode
						if ((addr & 0x00008000) == 0x00008000)
						{
							if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
							{
								ret = (ushort)((VRAM[(addr & 0x17FFF) + 1] << 8) | VRAM[addr & 0x17FFF]);
							}
							else
							{
								ret = 0;
							}		
						}
						else
						{
							ret = (ushort)((VRAM[(addr & 0x17FFF) + 1] << 8) | VRAM[addr & 0x17FFF]);
						}					
					}
					else
					{
						ret = (ushort)((VRAM[(addr & 0xFFFF) + 1] << 8) | VRAM[addr & 0xFFFF]);
					}

					ppu_VRAM_High_In_Use = false;
					ppu_VRAM_In_Use = false;
				}
				else if (addr >= 0x05000000)
				{
					ret = (ushort)((PALRAM[(addr & 0x3FF) + 1] << 8) | PALRAM[addr & 0x3FF]);

					ppu_PALRAM_In_Use = false;
				}
				else
				{
					if (addr < 0x04000800)
					{
						// Forced Align
						addr &= 0xFFFFFFFE;

						ret = Read_Registers_16(addr - 0x04000000);
					}
					else if ((addr & 0x0400FFFF) == 0x04000800)
					{
						// Forced Align
						addr &= 0xFFFFFFFE;

						switch (addr & 3)
						{
							case 0: ret = (ushort)(Memory_CTRL & 0xFFFF); break;
							default: ret = (ushort)((Memory_CTRL >> 16) & 0xFFFF); break;
						}
					}
					else
					{
						ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); // open bus
					}
				}
			}
			else if (addr >= 0x03000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFE;

				ret = (ushort)((IWRAM[(addr & 0x7FFF) + 1] << 8) | IWRAM[addr & 0x7FFF]);
			}
			else if (addr >= 0x02000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFE;

				ret = (ushort)((WRAM[(addr & 0x3FFFF) + 1] << 8) | WRAM[addr & 0x3FFFF]);
			}
			else if (addr < 0x4000)
			{
				// BIOS is protected against reading from memory beyond the BIOS range
				if (cpu_Regs[15] > 0x4000)
				{
					ret = (ushort)(Last_BIOS_Read & 0xFFFF);
				}
				else
				{
					// Forced Align
					addr &= 0xFFFFFFFE;

					ret = (ushort)((BIOS[addr + 1] << 8) | BIOS[addr]);
				}			
			}
			else
			{
				ret = (ushort)(cpu_Last_Bus_Value & 0xFFFF); // open bus
			}

			cpu_Last_Bus_Value &= 0xFFFF0000;
			cpu_Last_Bus_Value |= ret;

			return ret;
		}

		public uint Read_Memory_32(uint addr)
		{
			uint ret = 0;
			
			if (addr >= 0x08000000)
			{
				if (addr < 0x0D000000)
				{
					addr -= 0x08000000;
					// Forced Align
					addr &= 0xFFFFFFFC;

					ret = (uint)((ROM[addr + 3] << 24) | (ROM[addr + 2] << 16) | (ROM[addr + 1] << 8) | ROM[addr]);
				}
				else if (addr < 0x0E000000)
				{
					if (!Is_EEPROM)
					{
						addr -= 0x08000000;
						// Forced Align
						addr &= 0xFFFFFFFC;

						ret = (uint)((ROM[addr + 3] << 24) | (ROM[addr + 2] << 16) | (ROM[addr + 1] << 8) | ROM[addr]);
					}
					else
					{
						if (EEPROM_Wiring)
						{
							ret = (uint)((cpu_Last_Bus_Value & 0xFFFFFFFE) | mapper.Mapper_EEPROM_Read());
						}
						else
						{
							if ((addr & 0xDFFFE00) == 0xDFFFE00)
							{
								ret = (uint)((cpu_Last_Bus_Value & 0xFFFFFFFE) | mapper.Mapper_EEPROM_Read());
							}
							else
							{
								addr -= 0x08000000;
								// Forced Align
								addr &= 0xFFFFFFFC;

								ret = (uint)((ROM[addr + 3] << 24) | (ROM[addr + 2] << 16) | (ROM[addr + 1] << 8) | ROM[addr]);
							}
						}
					}
				}
				else if (addr < 0x10000000)
				{
					ret = mapper.ReadMemory32(addr - 0x0E000000);
				}
				else
				{
					ret = cpu_Last_Bus_Value; // open bus			
				}

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
			else if (addr >= 0x04000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFC;

				if (addr >= 0x07000000)
				{
					ret = (uint)((OAM[(addr & 0x3FF) + 3] << 24) | (OAM[(addr & 0x3FF) + 2] << 16) | (OAM[(addr & 0x3FF) + 1] << 8) | OAM[addr & 0x3FF]);
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						// mirrors behave differently depending on mode
						if ((addr & 0x00008000) == 0x00008000)
						{
							if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
							{
								ret = (uint)((VRAM[(addr & 0x17FFF) + 3] << 24) | (VRAM[(addr & 0x17FFF) + 2] << 16) | (VRAM[(addr & 0x17FFF) + 1] << 8) | VRAM[addr & 0x17FFF]);
							}
							else
							{
								ret = 0;
							}							
						}
						else
						{
							ret = (uint)((VRAM[(addr & 0x17FFF) + 3] << 24) | (VRAM[(addr & 0x17FFF) + 2] << 16) | (VRAM[(addr & 0x17FFF) + 1] << 8) | VRAM[addr & 0x17FFF]);
						}					
					}
					else
					{
						ret = (uint)((VRAM[(addr & 0xFFFF) + 3] << 24) | (VRAM[(addr & 0xFFFF) + 2] << 16) | (VRAM[(addr & 0xFFFF) + 1] << 8) | VRAM[addr & 0xFFFF]);
					}

					ppu_VRAM_High_In_Use = false;
					ppu_VRAM_In_Use = false;
				}
				else if (addr >= 0x05000000)
				{
					ret = (uint)((PALRAM[(addr & 0x3FF) + 3] << 24) | (PALRAM[(addr & 0x3FF) + 2] << 16) | (PALRAM[(addr & 0x3FF) + 1] << 8) | PALRAM[addr & 0x3FF]);

					ppu_PALRAM_In_Use = false;
				}
				else
				{
					if (addr < 0x04000800)
					{
						// Forced Align
						addr &= 0xFFFFFFFC;

						ret = Read_Registers_32(addr - 0x04000000);
					}
					else if ((addr & 0x0400FFFF) == 0x04000800)
					{
						// Forced Align
						addr &= 0xFFFFFFFC;

						ret = Memory_CTRL;
					}
					else
					{
						ret = cpu_Last_Bus_Value; // open bus
					}
				}
			}
			else if (addr >= 0x03000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFC;

				ret = (uint)((IWRAM[(addr & 0x7FFF) + 3] << 24) | (IWRAM[(addr & 0x7FFF) + 2] << 16) | (IWRAM[(addr & 0x7FFF) + 1] << 8) | IWRAM[addr & 0x7FFF]);
			}
			else if (addr >= 0x02000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFC;

				ret = (uint)((WRAM[(addr & 0x3FFFF) + 3] << 24) | (WRAM[(addr & 0x3FFFF) + 2] << 16) | (WRAM[(addr & 0x3FFFF) + 1] << 8) | WRAM[addr & 0x3FFFF]);
			}
			else if (addr < 0x4000)
			{
				// BIOS is protected against reading from memory beyond the BIOS range
				if (cpu_Regs[15] > 0x4000)
				{
					ret = Last_BIOS_Read;
				}
				else
				{
					// Forced Align
					addr &= 0xFFFFFFFC;

					Last_BIOS_Read = (uint)((BIOS[addr + 3] << 24) | (BIOS[addr + 2] << 16) | (BIOS[addr + 1] << 8) | BIOS[addr]);

					ret = Last_BIOS_Read;
				}			
			}
			else
			{
				ret = cpu_Last_Bus_Value; // open bus
			}

			cpu_Last_Bus_Value = ret;
			
			return ret;
		}

		public void Write_Memory_8(uint addr, byte value)
		{
			cpu_Last_Bus_Value &= 0xFFFFFF00;
			cpu_Last_Bus_Value |= value;

			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					WRAM[addr & 0x3FFFF] = value;
				}
			}
			else if (addr < 0x04000000)
			{
				IWRAM[addr & 0x7FFF] = value;
			}
			else if (addr < 0x05000000)
			{
				if (addr < 0x04000800)
				{
					Write_Registers_8(addr - 0x04000000, value);
				}
				else if ((addr & 0x0400FFFF) == 0x04000800)
				{
					switch (addr & 3)
					{
						case 0x00: Update_Memory_CTRL((uint)((Memory_CTRL & 0xFFFFFF00) | value)); break;
						case 0x01: Update_Memory_CTRL((uint)((Memory_CTRL & 0xFFFF00FF) | (value << 8))); break;
						case 0x02: Update_Memory_CTRL((uint)((Memory_CTRL & 0xFF00FFFF) | (value << 16))); break;
						default: Update_Memory_CTRL((uint)((Memory_CTRL & 0x00FFFFFF) | (value << 24))); break;
					}
				}
			}
			else if (addr < 0x06000000)
			{
				// 8 bit writes to PALRAM stored as halfword
				PALRAM[addr & 0x3FF] = value;
				PALRAM[(addr + 1) & 0x3FF] = value;

				ppu_PALRAM_In_Use = false;
			}
			else if (addr < 0x07000000)
			{
				if ((addr & 0x00010000) == 0x00010000)
				{
					// 8 bit writes ignored depending on mode
					// Seems like it has to do with what region is also used by sprites
					// bitmap modes (3-5) allow writes up to 0x14000, as this is reserved for BGs
					// other modes do not allow writes above 0x10000, as all of this is reserved for sprites
					// This effects the Quake demo

					// mirrors behave differently depending on mode
					if ((addr & 0x00008000) == 0x00008000)
					{
						if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
						{
							if ((addr & 0x17FFF) < 0x14000)
							{
								// 8 bit writes stored as halfword (needs more research)
								VRAM[addr & 0x17FFF] = value;
								VRAM[(addr + 1) & 0x17FFF] = value;
							}
						}
					}
					else
					{
						if (ppu_BG_Mode >= 3)
						{
							if ((addr & 0x17FFF) < 0x14000)
							{
								// 8 bit writes stored as halfword (needs more research)
								VRAM[addr & 0x17FFF] = value;
								VRAM[(addr + 1) & 0x17FFF] = value;
							}
						}
					}
				}
				else
				{
					// 8 bit writes stored as halfword (needs more research)
					VRAM[addr & 0xFFFF] = value;
					VRAM[(addr + 1) & 0xFFFF] = value;
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
			}
			else if (addr < 0x08000000)
			{
				// 8 bit writes to OAM ignored
				// OAM[addr & 0x3FF] = value;
			}
			else if (addr < 0x0D000000)
			{
				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;

				mapper.WriteROM8(addr, value);
			}
			else if (addr < 0x0E000000)
			{
				if (Is_EEPROM)
				{
					if (EEPROM_Wiring)
					{
						mapper.Mapper_EEPROM_Write(value);
					}
					else
					{
						if ((addr & 0xDFFFE00) == 0xDFFFE00)
						{
							mapper.Mapper_EEPROM_Write(value);
						}
					}
				}

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
			else if ((addr >= 0x0E000000) && (addr < 0x10000000))
			{
				mapper.WriteMemory8(addr - 0x0E000000, value);

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
		}

		public void Write_Memory_16(uint addr, ushort value)
		{
			cpu_Last_Bus_Value &= 0xFFFF0000;
			cpu_Last_Bus_Value |= value;

			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					// Forced Align
					addr &= 0xFFFFFFFE;

					WRAM[addr & 0x3FFFF] = (byte)(value & 0xFF);
					WRAM[(addr & 0x3FFFF) + 1] = (byte)((value >> 8) & 0xFF);
				}
			}
			else if (addr < 0x04000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFE;

				IWRAM[addr & 0x7FFF] = (byte)(value & 0xFF);
				IWRAM[(addr & 0x7FFF) + 1] = (byte)((value >> 8) & 0xFF);
			}
			else if (addr < 0x05000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFE;

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
				// Forced Align
				addr &= 0xFFFFFFFE;

				PALRAM[addr & 0x3FF] = (byte)(value & 0xFF);
				PALRAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);

				ppu_PALRAM_In_Use = false;
			}
			else if (addr < 0x07000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFE;

				if ((addr & 0x00010000) == 0x00010000)
				{
					// mirrors behave differently depending on mode
					if ((addr & 0x00008000) == 0x00008000)
					{
						if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
							{
							VRAM[addr & 0x17FFF] = (byte)(value & 0xFF);
							VRAM[(addr & 0x17FFF) + 1] = (byte)((value >> 8) & 0xFF);
						}
					}
					else
					{
						VRAM[addr & 0x17FFF] = (byte)(value & 0xFF);
						VRAM[(addr & 0x17FFF) + 1] = (byte)((value >> 8) & 0xFF);
					}				
				}
				else
				{
					VRAM[addr & 0xFFFF] = (byte)(value & 0xFF);
					VRAM[(addr & 0xFFFF) + 1] = (byte)((value >> 8) & 0xFF);
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
			}
			else if (addr < 0x08000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFE;

				OAM[addr & 0x3FF] = (byte)(value & 0xFF);
				OAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);
			}
			else if (addr < 0x0D000000)
			{
				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
				mapper.WriteROM16(addr, value);
			}
			else if (addr < 0x0E000000)
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

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
			else if ((addr >= 0x0E000000) && (addr < 0x10000000))
			{
				mapper.WriteMemory16(addr - 0x0E000000, value);

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
		}

		public void Write_Memory_32(uint addr, uint value)
		{
			cpu_Last_Bus_Value = value;		
			
			if (addr < 0x03000000)
			{
				if (addr >= 0x02000000)
				{
					// Forced Align
					addr &= 0xFFFFFFFC;

					WRAM[addr & 0x3FFFF] = (byte)(value & 0xFF);
					WRAM[(addr & 0x3FFFF) + 1] = (byte)((value >> 8) & 0xFF);
					WRAM[(addr & 0x3FFFF) + 2] = (byte)((value >> 16) & 0xFF);
					WRAM[(addr & 0x3FFFF) + 3] = (byte)((value >> 24) & 0xFF);
				}
			}
			else if (addr < 0x04000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFC;

				IWRAM[addr & 0x7FFF] = (byte)(value & 0xFF);
				IWRAM[(addr & 0x7FFF) + 1] = (byte)((value >> 8) & 0xFF);
				IWRAM[(addr & 0x7FFF) + 2] = (byte)((value >> 16) & 0xFF);
				IWRAM[(addr & 0x7FFF) + 3] = (byte)((value >> 24) & 0xFF);
			}
			else if (addr < 0x05000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFC;

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
				// Forced Align
				addr &= 0xFFFFFFFC;

				PALRAM[addr & 0x3FF] = (byte)(value & 0xFF);
				PALRAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);
				PALRAM[(addr & 0x3FF) + 2] = (byte)((value >> 16) & 0xFF);
				PALRAM[(addr & 0x3FF) + 3] = (byte)((value >> 24) & 0xFF);

				ppu_PALRAM_In_Use = false;
				PALRAM_32_Check = false;
			}
			else if (addr < 0x07000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFC;

				if ((addr & 0x00010000) == 0x00010000)
				{
					// mirrors behave differently depending on mode
					if ((addr & 0x00008000) == 0x00008000)
					{
						if ((ppu_BG_Mode < 3) || ((addr & 0x00004000) == 0x00004000))
						{
							VRAM[addr & 0x17FFF] = (byte)(value & 0xFF);
							VRAM[(addr & 0x17FFF) + 1] = (byte)((value >> 8) & 0xFF);
							VRAM[(addr & 0x17FFF) + 2] = (byte)((value >> 16) & 0xFF);
							VRAM[(addr & 0x17FFF) + 3] = (byte)((value >> 24) & 0xFF);
						}
					}
					else
					{
						VRAM[addr & 0x17FFF] = (byte)(value & 0xFF);
						VRAM[(addr & 0x17FFF) + 1] = (byte)((value >> 8) & 0xFF);
						VRAM[(addr & 0x17FFF) + 2] = (byte)((value >> 16) & 0xFF);
						VRAM[(addr & 0x17FFF) + 3] = (byte)((value >> 24) & 0xFF);
					}
				}
				else
				{
					VRAM[addr & 0xFFFF] = (byte)(value & 0xFF);
					VRAM[(addr & 0xFFFF) + 1] = (byte)((value >> 8) & 0xFF);
					VRAM[(addr & 0xFFFF) + 2] = (byte)((value >> 16) & 0xFF);
					VRAM[(addr & 0xFFFF) + 3] = (byte)((value >> 24) & 0xFF);
				}

				ppu_VRAM_High_In_Use = false;
				ppu_VRAM_In_Use = false;
				VRAM_32_Check = false;
			}
			else if (addr < 0x08000000)
			{
				// Forced Align
				addr &= 0xFFFFFFFC;

				OAM[addr & 0x3FF] = (byte)(value & 0xFF);
				OAM[(addr & 0x3FF) + 1] = (byte)((value >> 8) & 0xFF);
				OAM[(addr & 0x3FF) + 2] = (byte)((value >> 16) & 0xFF);
				OAM[(addr & 0x3FF) + 3] = (byte)((value >> 24) & 0xFF);
			}
			else if (addr < 0x0D000000)
			{
				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;

				mapper.WriteROM32(addr, value);
			}
			else if (addr < 0x0E000000)
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

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
			else if ((addr >= 0x0E000000) && (addr < 0x10000000))
			{
				mapper.WriteMemory32(addr - 0x0E000000, value);

				// ROM access complete, re-enable prefetcher
				pre_Fetch_Cnt_Inc = 1;
			}
		}

		public byte Peek_Memory_8(uint addr)
		{
			if (addr >= 0x08000000)
			{
				if (addr < 0x0D000000)
				{
					return ROM[addr - 0x08000000];
				}
				else if (addr < 0x0E000000)
				{
					if (!Is_EEPROM)
					{
						return ROM[addr - 0x08000000];
					}
					else
					{
						if (EEPROM_Wiring)
						{
							return (byte)mapper.Mapper_EEPROM_Read();
						}
						else
						{
							if ((addr & 0xDFFFE00) == 0xDFFFE00)
							{
								return (byte)mapper.Mapper_EEPROM_Read();
							}
							else
							{
								return ROM[addr - 0x08000000];
							}
						}
					}
				}
				else if (addr < 0x10000000)
				{
					return mapper.PeekMemory(addr - 0x0E000000);
				}

				return (byte)(cpu_Last_Bus_Value & 0xFF); // open bus
			}
			else if (addr >= 0x05000000)
			{
				if (addr >= 0x07000000)
				{
					return OAM[addr & 0x3FF];
				}
				else if (addr >= 0x06000000)
				{
					if ((addr & 0x00010000) == 0x00010000)
					{
						return VRAM[addr & 0x17FFF];
					}
					else
					{
						return VRAM[addr & 0xFFFF];
					}
				}
				else
				{
					return PALRAM[addr & 0x3FF];
				}
			}
			else if (addr >= 0x04000000)
			{
				if (addr < 0x04000800)
				{
					return Peek_Registers_8(addr - 0x04000000);
				}
				else if ((addr & 0x0400FFFF) == 0x04000800)
				{
					switch (addr & 3)
					{
						case 0: return (byte)(Memory_CTRL & 0xFF);
						case 1: return (byte)((Memory_CTRL >> 8) & 0xFF);
						case 2: return (byte)((Memory_CTRL >> 16) & 0xFF);
						default: return (byte)((Memory_CTRL >> 24) & 0xFF);
					}
				}

				return (byte)(cpu_Last_Bus_Value & 0xFF); // open bus
			}
			else if (addr >= 0x03000000)
			{
				return IWRAM[addr & 0x7FFF];
			}
			else if (addr >= 0x02000000)
			{
				return WRAM[addr & 0x3FFFF];
			}

			if (addr < 0x4000)
			{
				return BIOS[addr];
			}

			return (byte)(cpu_Last_Bus_Value & 0xFF); // open bus
		}
	}

#pragma warning restore CS0675 // Bitwise-or operator used on a sign-extended operand
}
