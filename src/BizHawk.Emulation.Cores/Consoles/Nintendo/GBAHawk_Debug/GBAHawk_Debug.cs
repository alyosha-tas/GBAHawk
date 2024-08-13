﻿using System;

using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using System.Runtime.InteropServices;

using BizHawk.Common.ReflectionExtensions;
using System.Security.Cryptography;

/*
	GBA Emulator
	NOTES: 
	RAM disabling not implemented, check if used by any games

	Open bus behaviour needs to be done more carefully

	EEPROM accesses only emulated at 0xDxxxxxx, check if any games use lower range

	Stop mode is disabled on GBP, will need to implement this when gamecube detection is implemented

	What is a good way to handle different FLASH types?

	TODO: Check timing of 32 bit reads / writes to GPIO

	TODO: Investigate interaction between halt and DMA
*/

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	[Core(CoreNames.GBAHawk_Debug, "")]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBAHawk_Debug : IEmulator, ISaveRam, IInputPollable, IRegionable, IGBAGPUViewable, 
	ISettable<GBAHawk_Debug.GBAHawk_Debug_Settings, GBAHawk_Debug.GBAHawk_Debug_SyncSettings>
	{
		internal static class RomChecksums
		{

		}

		public uint Memory_CTRL, ROM_Length;

		public uint Last_BIOS_Read;

		public int WRAM_Waits, SRAM_Waits;
		
		public int ROM_Waits_0_N, ROM_Waits_1_N, ROM_Waits_2_N, ROM_Waits_0_S, ROM_Waits_1_S, ROM_Waits_2_S;

		public ushort INT_EN, INT_Flags, INT_Master, Wait_CTRL;

		public ushort INT_Flags_Gather, INT_Flags_Use;

		public ushort controller_state, controller_state_old;

		public byte Post_Boot, Halt_CTRL;

		public bool All_RAM_Disable, WRAM_Enable;

		public bool INT_Master_On;

		public bool Reset_RTC;

		public int ext_num = 0; // zero here means disconnected

		public bool is_linked_system = false;

		// memory domains
		public byte[] WRAM = new byte[0x40000];
		public byte[] IWRAM = new byte[0x8000];
		public byte[] PALRAM = new byte[0x400];
		public byte[] VRAM = new byte[0x18000];
		public byte[] OAM = new byte[0x400];

		public byte[] BIOS;
		public byte[] ROM = new byte[0x6000000];

		public byte[] cart_RAM;
		public bool has_bat;
		public bool use_sram;
		public bool Is_EEPROM;
		public bool EEPROM_Wiring; // when true, can access anywhere in 0xDxxxxxx range, otheriwse only 0xDFFFFE0

		public int Frame_Count = 0;

		public bool Use_MT;

		public bool GBP_Mode_Enabled;
		public bool GBP_Screen_Detection;
		public int GBP_Screen_Count;

		public static readonly byte[] multi_boot_check = { 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x6E, 0x27, 0x74, 0x20, 0x61, 0x20, 0x52, 0x4F, 0x4D };

		public MapperBase mapper;

		private readonly ITraceable _tracer;

		[CoreConstructor(VSystemID.Raw.GBA)]
		public GBAHawk_Debug(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ GBAHawk_Debug_Settings settings, GBAHawk_Debug_SyncSettings syncSettings, bool subframe = false)
		{
			var ser = new BasicServiceProvider(this);
			
			_ = PutSettings(settings ?? new GBAHawk_Debug_Settings());
			_syncSettings = (GBAHawk_Debug_SyncSettings)syncSettings ?? new GBAHawk_Debug_SyncSettings();

			use_sram = _syncSettings.Use_SRAM;

			// Load up a BIOS and initialize the correct PPU
			BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			bool is_multi_boot = true;

			if (rom.Length < 16) { throw new Exception("ROM too small"); }

			for (int j = 0; j < 16; j++)
			{
				if (rom[j] != multi_boot_check[j]) { is_multi_boot = false; }
			}

			if (is_multi_boot)
			{
				// replace with empty ROM
				for (int j = 0; j < 0x6000000; j += 2)
				{
					ROM[j] = (byte)((j & 0xFF) >> 1);
					ROM[j + 1] = (byte)(((j >> 8) & 0xFF) >> 1);
				}

				Console.WriteLine("No ROM inserted to console");
			}

			// TODO: Better manage small rom sizes (ex in various test roms.)
			// the mgba test quite expects ROM to not be mirrored
			// but the GBA Tests memory test expects it to be mirrored
			// it probably depends on the cart, GBA TEK only specifies the case where no cart is inserted.
			// for testing purposes divide the cases with a hash check
			else if (rom.Length > 0x6000000)
			{
				throw new Exception("Over size ROM?");			
			}
			else
			{
				ROM_Length = (uint)rom.Length;
				Buffer.BlockCopy(rom, 0, ROM, 0, rom.Length);

				if ((romHashSHA1 != "SHA1:5F989B9A4017F16A431F76FD78A95E9799AA8FCA") && // GBA Suite Memory test
					(romHashSHA1 != "SHA1:D015A5039FF5D08EEBA3DDB16470EAAB259631D0"))   // Broken Circle
				{
					// fill unused ROM area (assuming the ROM chip doesn't respond)
					// for now mirror across 32MB boundaries, but might need to be more precise for smaller ROMs (do they exist?)
					if (rom.Length < 0x6000000)
					{
						int ofst_base = rom.Length & 0xF000000;

						if (rom.Length > ofst_base)
						{ 
							ofst_base += 0x1000000;
						}

						if (ofst_base < 0x6000000)
						{
							for (int i = 0; i < (0x6000000 - ofst_base); i += 2)
							{
								ROM[i + ofst_base] = (byte)((i & 0xFF) >> 1);
								ROM[i + ofst_base + 1] = (byte)(((i >> 8) & 0xFF) >> 1);
							}
						}
					}
				}
				else
				{
					// mirror the rom accross the whole region (might need different increment sizes for different ROMs)
					for (int i = 0; i < rom.Length; i++)
					{
						ROM[i + 0x2000000] = rom[i];
						ROM[i + 0x4000000] = rom[i];
					}
				}			
			}

			var mppr = Setup_Mapper(romHashMD5, romHashSHA1);
			
			if (cart_RAM != null) 
			{ 
				// initialize SRAM to 0xFF;
				if (mppr == "SRAM")
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}

				// initialize EEPROM to 0xFF;
				if (mppr == "EEPROM")
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}

				// initialize Flash to 0x0;
				// jsmolka test ROM says it should be 0xFF, but this doesn't work with ex. Mario vs Donkey Kong
				// if the erase function takes a non-negligable amount of time
				if (mppr == "FLASH")
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
			}

			mapper.Core = this;

			mapper.ROM_C4 = ROM[0xC4];
			mapper.ROM_C5 = ROM[0xC5];
			mapper.ROM_C6 = ROM[0xC6];
			mapper.ROM_C7 = ROM[0xC7];
			mapper.ROM_C8 = ROM[0xC8];
			mapper.ROM_C9 = ROM[0xC9];

			if (mapper is MapperEEPROM_Tilt)
			{
				_controllerDeck = new(typeof(StandardTilt).DisplayName(), subframe);
			}
			else if (mapper is MapperEEPROM_Solar)
			{
				_controllerDeck = new(typeof(StandardSolar).DisplayName(), subframe);
			}
			else if (mapper is MapperSRAMGyro)
			{
				_controllerDeck = new(typeof(StandardZGyro).DisplayName(), subframe);
			}
			else
			{
				_controllerDeck = new(GBA_ControllerDeck.DefaultControllerName, subframe);
			}
			
			ser.Register<IVideoProvider>(this);
			ser.Register<ISoundProvider>(this);
			ServiceProvider = ser;

			_ = PutSettings(settings ?? new GBAHawk_Debug_Settings());
			_syncSettings = (GBAHawk_Debug_SyncSettings)syncSettings ?? new GBAHawk_Debug_SyncSettings();

			_tracer = new TraceBuffer(TraceHeader);
			ser.Register<ITraceable>(_tracer);
			ser.Register<IStatable>(new StateSerializer(SyncState));
			SetupMemoryDomains();

			// only reset here, not in pressing power button
			CycleCount = 0;
			Clock_Update_Cycle = 0;

			// only reset RTC on initialize
			Reset_RTC = true;
			HardReset();
			Reset_RTC = false;

			mapper.RTC_Functional = true;

			if (_syncSettings.RTCInitialState == GBAHawk_Debug_SyncSettings.InitRTCState.Reset_Bad_Batt)
			{
				mapper.RTC_Functional = false;
			}
			else if (_syncSettings.RTCInitialState == GBAHawk_Debug_SyncSettings.InitRTCState.RTC_Set)
			{
				// all games seem to use 24 hour mode,, so use this to represent set time
				mapper.Reg_Ctrl = 0x40;

				// parse the date and time into the regs
				DateTime temp = _syncSettings.RTCInitialTime;

				// if year outside range of RTC, just leave the initial values
				if ((temp.Year < 2100) && (temp.Year >= 2000))
				{
					mapper.Reg_Year = mapper.To_BCD((byte)(temp.Year - 2000));
					mapper.Reg_Month = mapper.To_BCD((byte)temp.Month);
					mapper.Reg_Day = mapper.To_BCD((byte)temp.Day);
					mapper.Reg_Week = mapper.To_BCD((byte)temp.DayOfWeek);
					mapper.Reg_Minute = mapper.To_BCD((byte)temp.Minute);
					mapper.Reg_Second = mapper.To_BCD((byte)temp.Second);

					mapper.Reg_Hour = mapper.To_BCD((byte)temp.Hour);

					if (temp.Hour >= 12)
					{
						mapper.Reg_Hour |= 0x80;
					}
				}
			}

			DeterministicEmulation = true;

			Mem_Domains.vram = Marshal.AllocHGlobal(VRAM.Length + 1);
			Mem_Domains.oam = Marshal.AllocHGlobal(OAM.Length + 1);
			Mem_Domains.mmio = Marshal.AllocHGlobal(0x60 + 1);
			Mem_Domains.palram = Marshal.AllocHGlobal(PALRAM.Length + 1);
		}

		public ulong TotalExecutedCycles => CycleCount;

		public DisplayType Region => DisplayType.NTSC;

		private readonly GBA_ControllerDeck _controllerDeck;

		public void HardReset()
		{
			delays_to_process = false;

			IRQ_Write_Delay = IRQ_Write_Delay_2 = IRQ_Write_Delay_3 = false;

			Halt_Enter = Halt_Leave = false;

			IRQ_Delays = Misc_Delays = VRAM_32_Delay = PALRAM_32_Delay = false;

			VRAM_32_Check = PALRAM_32_Check = false;

			FIFO_DMA_A_Delay = FIFO_DMA_B_Delay = false;

			DMA_Any_Start = DMA_Any_IRQ = false;

			for (int i = 0; i < 4; i++) { DMA_IRQ_Delay[i] = false; DMA_Start_Delay[i] = false; }

			controller_state = controller_state_old = 0x3FF;

			Memory_CTRL = 0;

			Last_BIOS_Read = 0;

			WRAM_Waits = SRAM_Waits = 0;

			FIFO_DMA_A_cd = FIFO_DMA_B_cd = 0;

			Halt_Enter_cd = Halt_Leave_cd = Halt_Held_CPU_Instr = 0;

			ROM_Waits_0_N = ROM_Waits_1_N = ROM_Waits_2_N = ROM_Waits_0_S = ROM_Waits_1_S = ROM_Waits_2_S = 0;

			INT_EN = INT_Flags = INT_Master = Wait_CTRL = 0;

			INT_Flags_Gather = INT_Flags_Use = 0;

			Post_Boot = Halt_CTRL = 0;

			All_RAM_Disable = WRAM_Enable = false;

			INT_Master_On = false;

			tim_Reset();
			ppu_Reset();
			snd_Reset();
			ser_Reset();
			cpu_Reset();
			dma_Reset();
			pre_Reset();
			mapper.Reset();

			vid_buffer = new int[VirtualWidth * VirtualHeight];

			for (int i = 0; i < vid_buffer.Length; i++)
			{
				vid_buffer[i] = unchecked((int)0xFFF8F8F8);
			}

			for (int i = 0; i < IWRAM.Length; i++)
			{
				IWRAM[i] = 0;
			}

			for (int i = 0; i < WRAM.Length; i++)
			{
				WRAM[i] = 0;
			}

			// default memory config hardware initialized
			Update_Memory_CTRL(0x0D000020);

			GBP_Mode_Enabled = false;
			GBP_Screen_Detection = false;
			GBP_Screen_Count = 0;
		}

		// GBA PPU Viewer
		public Action _scanlineCallback;
		public int _scanlineCallbackLine = 0;

		public void SetScanlineCallback(Action callback, int line)
		{
			_scanlineCallback = callback;
			_scanlineCallbackLine = line;

			if (line == -2)
			{
				_scanlineCallback();
			}
		}

		GBAGPUMemoryAreas Mem_Domains = new GBAGPUMemoryAreas();

		public GBAGPUMemoryAreas GetMemoryAreas()
		{
			Marshal.Copy(VRAM, 0, Mem_Domains.vram, VRAM.Length);
			Marshal.Copy(OAM, 0, Mem_Domains.oam, OAM.Length);
			Marshal.Copy(PALRAM, 0, Mem_Domains.palram, PALRAM.Length);

			byte[] temp = new byte[0x60];

			for (uint i = 0; i < 0x60; i++)
			{
				temp[i] = ppu_Read_Reg_8(i);
			}

			Marshal.Copy(temp, 0, Mem_Domains.mmio, temp.Length);

			return Mem_Domains;
		}

		public string Setup_Mapper(string romHashMD5, string romHashSHA1)
		{
			string mppr = "";
			has_bat = false;
			Is_EEPROM = false;
			EEPROM_Wiring = false;
			int size_f = 0;

			// check for SRAM
			for (int i = 0; i < ROM.Length; i += 4)
			{
				if (ROM[i] == 0x53)
				{
					if ((ROM[i + 1] == 0x52) && (ROM[i + 2] == 0x41))
					{
						if ((ROM[i + 3] == 0x4D) && (ROM[i + 4] == 0x5F))
						{
							Console.WriteLine("using SRAM mapper");
							mppr = "SRAM";
							break;
						}
					}
				}

				if (ROM[i] == 0x45)
				{
					if ((ROM[i + 1] == 0x45) && (ROM[i + 2] == 0x50))
					{
						if ((ROM[i + 3] == 0x52) && (ROM[i + 4] == 0x4F) && (ROM[i + 5] == 0x4D))
						{
							Console.WriteLine("using EEPROM mapper");
							mppr = "EEPROM";
							break;
						}
					}
				}

				if (ROM[i] == 0x46)
				{
					if ((ROM[i + 1] == 0x4C) && (ROM[i + 2] == 0x41))
					{
						if ((ROM[i + 3] == 0x53) && (ROM[i + 4] == 0x48))
						{
							if ((ROM[i + 5] == 0x5F) && (ROM[i + 6] == 0x56))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = "FLASH";
								size_f = 64;

								break;
							}
							if ((ROM[i + 5] == 0x35) && (ROM[i + 6] == 0x31) && (ROM[i + 7] == 0x32))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = "FLASH";
								size_f = 64;

								break;
							}
							if ((ROM[i + 5] == 0x31) && (ROM[i + 6] == 0x4D))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = "FLASH";
								size_f = 128;

								break;
							}
						}
					}
				}
			}

			// SHA1:AC6D8FD4A1FB5234A889EE092CBE7774DAC21F0E


			// hash checks for individual games / homebrew / test roms
			if ((romHashSHA1 == "SHA1:C67E0A5E26EA5EBA2BC11C99D003027A96E44060") || // Aging cart test
				(romHashSHA1 == "SHA1:AC6D8FD4A1FB5234A889EE092CBE7774DAC21F0E") || // VRAM access test
				(romHashSHA1 == "SHA1:41D39A0C34F72469DD3FBCC90190605B8ADA93E6") || // Another World
				(romHashSHA1 == "SHA1:270C426705DF767A4AD2DC69D039842442F779B2") || // Anguna
				(romHashSHA1 == "SHA1:9B02C4BFD99CCD913A5D7EE7CF269EBC689E1FDE"))   // Higurashi no Nakukoroni (fixed header)
			{
				Console.WriteLine("using SRAM mapper");
				mppr = "SRAM";
			}

			if (mppr == "")
			{
				mppr = "NROM";

				if (romHashSHA1 == "SHA1:3714D1222E5C2B2734996ACE9F9BC49B35656171")
				{
					mapper = new MapperDefaultRTC();
				}
				else
				{
					mapper = new MapperDefault();
				}	
			}
			else if (mppr == "SRAM")
			{
				has_bat = true;
				cart_RAM = new byte[0x8000];

				if ((romHashSHA1 == "SHA1:A389FA50E2E842B264B980CBE30E980C69D93A5B") || // Mawaru - Made in Wario (JPN)
					(romHashSHA1 == "SHA1:F0102D0D6F7596FE853D5D0A94682718278E083A"))	// Warioware Twisted (USA)
				{
					mapper = new MapperSRAMGyro();
				}
				else
				{
					mapper = new MapperSRAM();
				}
			}
			else if (mppr == "EEPROM")
			{
				// assume 512 byte saves, use hash check to pick out 8k versions
				has_bat = true;
				Is_EEPROM = true;

				if (ROM_Length <= 0x1000000)
				{
					EEPROM_Wiring = true;
				}
				else
				{
					EEPROM_Wiring = false;
				}

				if ((romHashSHA1 == "SHA1:947498CB1DB918D305500257E8223DEEADDF561D") || // Yoshi USA
					(romHashSHA1 == "SHA1:A3F2035CA2BDC2BC59E9E46EFBB6187705EBE3D1") || // Yoshi Japan
					(romHashSHA1 == "SHA1:045BE1369964F141009F3701839EC0A8DCCB25C1") || // Yoshi EU
					(romHashSHA1 == "SHA1:40CB751D119A49BE0CD44CF0491C93EBC8795EF0"))   // koro koro puzzle
				{
					Console.WriteLine("Using Tilt Controls");

					cart_RAM = new byte[0x200];
					mapper = new MapperEEPROM_Tilt();
				}
				else if ((romHashSHA1 == "SHA1:F91126CD3A1BF7BF5F770D3A70229171D0D5A6EE") || // Boktai Beta
						 (romHashSHA1 == "SHA1:64F7BF0F0560F6E94DA33B549D3206678B29F557") || // Boktai EU
						 (romHashSHA1 == "SHA1:7164326283DF46A3941EC7B6CECA889CBC40E660") || // Boktai USA
						 (romHashSHA1 == "SHA1:C51AD84E9403DB94CD18A14AC72F8367B52A0D7F") || // Boktai JPN
						 (romHashSHA1 == "SHA1:CD10D8ED82F4DAF4072774F70D015E39A5D32D0B") || // Boktai 2 USA
						 (romHashSHA1 == "SHA1:EEACDF5A9D3D2173A4A96689B72DC6B7AD92153C") || // Boktai 2 EU
						 (romHashSHA1 == "SHA1:54A4DCDECA2EE9A22559EB104B88586386639097") || // Boktai 2 JPN
						 (romHashSHA1 == "SHA1:1A81843C3070DECEA4CBCA20C4563541400B2437") || // Boktai 2 JPN Rev 1
						 (romHashSHA1 == "SHA1:2651C5E6875AC60ABFF734510D152166D211C87C"))   // Boktai 3
				{
					Console.WriteLine("Using Solar Sensor");

					cart_RAM = new byte[0x2000];
					mapper = new MapperEEPROM_Solar();
				}
				else if (GBACommonFunctions.EEPROM_64K_check(romHashSHA1))
				{
					cart_RAM = new byte[0x2000];
					mapper = new MapperEEPROM();
				}
				else
				{
					cart_RAM = new byte[0x200];
					mapper = new MapperEEPROM();
				}
			}
			else if (mppr == "FLASH")
			{
				has_bat = true;

				if (GBACommonFunctions.pokemon_check(romHashSHA1) ||
					(romHashSHA1 == "SHA1:4DCD7CEE46D3A5E848A22EB371BEBBBC2FB8D488")) // Sennen Kozoku
				{
					cart_RAM = new byte[0x20000];

					mapper = new MapperFLASH_RTC();
				}
				else
				{
					if (size_f == 64)
					{
						cart_RAM = new byte[0x10000];
					}
					else
					{
						cart_RAM = new byte[0x20000];
					}

					mapper = new MapperFLASH();
				}		
			}

			return mppr;
		}
	}
}
