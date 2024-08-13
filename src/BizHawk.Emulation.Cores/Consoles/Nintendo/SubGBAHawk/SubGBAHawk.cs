using System;
using System.Text;
using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using BizHawk.Common.ReflectionExtensions;

/*
	GBA Emulator
	NOTES: 
	RAM disabling not implemented, check if used by any games

	Open bus behaviour needs to be done more carefully

	EEPROM accesses only emulated at 0xDxxxxxx, check if any games use lower range
*/

namespace BizHawk.Emulation.Cores.Nintendo.SubGBA
{
	[Core(CoreNames.SubGBAHawk, "", isReleased: true)]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class SubGBAHawk : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable, IRegionable, IGBAGPUViewable,
								ISettable<SubGBAHawk.SubGBASettings, SubGBAHawk.SubGBASyncSettings>
	{
		public byte[] BIOS;

		public readonly byte[] ROM = new byte[0x6000000];

		public uint ROM_Length;

		public ushort controller_state;
		public ushort Acc_X_state;
		public ushort Acc_Y_state;
		public byte Solar_state;

		public byte[] cart_RAM;
		public bool has_bat;
		public bool use_sram;
		int mapper;

		[CoreConstructor(VSystemID.Raw.GBA)]
		public SubGBAHawk(CoreComm comm, GameInfo game, byte[] rom, SubGBAHawk.SubGBASettings settings, SubGBAHawk.SubGBASyncSettings syncSettings)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (SubGBASettings)settings ?? new SubGBASettings();
			SyncSettings = (SubGBASyncSettings)syncSettings ?? new SubGBASyncSettings();

			use_sram = SyncSettings.Use_SRAM;

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			// TODO: Better manage small rom sizes (ex in various test roms.)
			// the mgba test quite expects ROM to not be mirrored
			// but the GBA Tests memory test expects it to be mirrored
			// it probably depends on the cart, GBA TEK only specifies the case where no cart is inserted.
			// for testing purposes divide the cases with a hash check
			if (rom.Length > 0x6000000)
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
					// for now mirror across 2MB boundaries, but might need to be more precise for smaller ROMs (do they exist?)
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

			mapper = Setup_Mapper(romHashMD5, romHashSHA1);

			if (cart_RAM != null)
			{
				// initialize SRAM to 0xFF;
				if ((mapper == 2) || (mapper == 3))
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
				// initialize EEPROM to 0xFF;
				if ((mapper == 4) || (mapper == 5) || (mapper == 6))
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
				// initialize Flash to 0xF;
				if ((mapper == 7) || (mapper == 8))
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
			}

			// Load up a BIOS and initialize the correct PPU
			BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			GBA_Pntr = LibSubGBAHawk.GBA_create();

			LibSubGBAHawk.GBA_load_bios(GBA_Pntr, BIOS);

			bool rtc_working = true;

			byte temp_year = 0;
			byte temp_month = 1;
			byte temp_day = 1;
			byte temp_week = 0;
			byte temp_hour = 0;
			byte temp_minute = 0;
			byte temp_second = 0;
			byte temp_ctrl = 0;

			if (SyncSettings.RTCInitialState == SubGBASyncSettings.InitRTCState.Reset_Bad_Batt)
			{
				rtc_working = false;
			}
			else if (SyncSettings.RTCInitialState == SubGBASyncSettings.InitRTCState.RTC_Set)
			{
				// all games seem to use 24 hour mode,, so use this to represent set time
				temp_ctrl = 0x40;

				// parse the date and time into the regs
				DateTime temp = SyncSettings.RTCInitialTime;

				// if year outside range of RTC, just leave the initial values
				if ((temp.Year < 2100) && (temp.Year >= 2000))
				{
					temp_year = To_BCD((byte)(temp.Year - 2000));
					temp_month = To_BCD((byte)temp.Month);
					temp_day = To_BCD((byte)temp.Day);
					temp_week = To_BCD((byte)temp.DayOfWeek);
					temp_minute = To_BCD((byte)temp.Minute);
					temp_second = To_BCD((byte)temp.Second);

					temp_hour = To_BCD((byte)temp.Hour);

					if (temp.Hour >= 12)
					{
						temp_hour |= 0x80;
					}
				}
			}

			ulong date_time = 0;

			date_time |= temp_second;
			date_time |= ((ulong)temp_minute << 8);
			date_time |= ((ulong)temp_hour << 16);
			date_time |= ((ulong)temp_week << 24);
			date_time |= ((ulong)temp_day << 32);
			date_time |= ((ulong)temp_month << 40);
			date_time |= ((ulong)temp_year << 48);
			date_time |= ((ulong)temp_ctrl << 56);

			Console.WriteLine("Mapper: " + mapper);
			LibSubGBAHawk.GBA_load(GBA_Pntr, ROM, (uint)ROM_Length, mapper, date_time, rtc_working, SyncSettings.EEPROMOffset);

			if (cart_RAM != null) { LibSubGBAHawk.GBA_create_SRAM(GBA_Pntr, cart_RAM, (uint)cart_RAM.Length); }

			blip_L.SetRates(4194304 * 4, 44100);
			blip_R.SetRates(4194304 * 4, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			SetupMemoryDomains();

			Header_Length = LibSubGBAHawk.GBA_getheaderlength(GBA_Pntr);
			Disasm_Length = LibSubGBAHawk.GBA_getdisasmlength(GBA_Pntr);
			Reg_String_Length = LibSubGBAHawk.GBA_getregstringlength(GBA_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibSubGBAHawk.GBA_getheader(GBA_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			if (mapper == 3)
			{
				_controllerDeck = new(typeof(StandardZGyro).DisplayName(), true);
			}
			else if (mapper == 5)
			{
				_controllerDeck = new(typeof(StandardTilt).DisplayName(), true);
			}
			else if (mapper == 6)
			{
				_controllerDeck = new(typeof(StandardSolar).DisplayName(), true);
			}
			else
			{
				_controllerDeck = new(GBA_ControllerDeck.DefaultControllerName, true);
			}

			Mem_Domains.vram = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 0);
			Mem_Domains.oam = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 1);
			Mem_Domains.palram = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 2);
			Mem_Domains.mmio = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 3);

			GBA_message = null;

			LibSubGBAHawk.GBA_setmessagecallback(GBA_Pntr, GBA_message);
		}

		public byte To_BCD(byte in_byte)
		{
			byte tens_cnt = 0;

			while (in_byte >= 10)
			{
				tens_cnt += 1;
				in_byte -= 10;
			}

			return (byte)((tens_cnt << 4) | in_byte);
		}

		public int Setup_Mapper(string romHashMD5, string romHashSHA1)
		{
			int size_f = 0;

			int mppr = 0;
			has_bat = false;

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
							mppr = 2;
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
							mppr = 4;
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
								mppr = 7;
								size_f = 64;

								break;
							}
							if ((ROM[i + 5] == 0x35) && (ROM[i + 6] == 0x31) && (ROM[i + 7] == 0x32))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = 7;
								size_f = 64;

								break;
							}
							if ((ROM[i + 5] == 0x31) && (ROM[i + 6] == 0x4D))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = 7;
								size_f = 128;

								break;
							}
						}
					}
				}
			}

			// hash checks for individual games / homebrew / test roms
			if ((romHashSHA1 == "SHA1:C67E0A5E26EA5EBA2BC11C99D003027A96E44060") || // Aging cart test
				(romHashSHA1 == "SHA1:AC6D8FD4A1FB5234A889EE092CBE7774DAC21F0E") || // VRAM access test
				(romHashSHA1 == "SHA1:41D39A0C34F72469DD3FBCC90190605B8ADA93E6") || // Another World
				(romHashSHA1 == "SHA1:270C426705DF767A4AD2DC69D039842442F779B2") || // Anguna
				(romHashSHA1 == "SHA1:9B02C4BFD99CCD913A5D7EE7CF269EBC689E1FDE"))   // Higurashi no Nakukoroni (fixed header)
			{
				Console.WriteLine("using SRAM mapper");
				mppr = 2;
			}

			if (romHashSHA1 == "SHA1:3714D1222E5C2B2734996ACE9F9BC49B35656171")
			{
				mppr = 1;
			}
			else if (mppr == 2)
			{
				has_bat = true;
				cart_RAM = new byte[0x8000];

				if ((romHashSHA1 == "SHA1:A389FA50E2E842B264B980CBE30E980C69D93A5B") || // Mawaru - Made in Wario (JPN)
					(romHashSHA1 == "SHA1:F0102D0D6F7596FE853D5D0A94682718278E083A"))   // Warioware Twisted (USA)
				{
					mppr = 3;
				}
			}
			else if (mppr == 4)
			{
				has_bat = true;

				// assume 512 byte saves, use hash check to pick out 8k versions
				if ((romHashSHA1 == "SHA1:947498CB1DB918D305500257E8223DEEADDF561D") || // Yoshi USA
					(romHashSHA1 == "SHA1:A3F2035CA2BDC2BC59E9E46EFBB6187705EBE3D1") || // Yoshi Japan
					(romHashSHA1 == "SHA1:045BE1369964F141009F3701839EC0A8DCCB25C1") || // Yoshi EU
					(romHashSHA1 == "SHA1:40CB751D119A49BE0CD44CF0491C93EBC8795EF0"))   // koro koro puzzle
				{
					Console.WriteLine("Using Tilt Controls");

					cart_RAM = new byte[0x200];
					mppr = 5;
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
					mppr = 6;
				}
				else if (GBACommonFunctions.EEPROM_64K_check(romHashSHA1))
				{
					cart_RAM = new byte[0x2000];
				}
				else
				{
					cart_RAM = new byte[0x200];
				}
			}
			else if (mppr == 7)
			{
				has_bat = true;

				if (GBACommonFunctions.pokemon_check(romHashSHA1) ||
					(romHashSHA1 == "SHA1:4DCD7CEE46D3A5E848A22EB371BEBBBC2FB8D488")) // Sennen Kozoku
				{
					cart_RAM = new byte[0x20000];

					mppr = 8;
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
				}
			}

			return mppr;
		}

		public ulong TotalExecutedCycles => 0;

		public void HardReset()
		{
			GBP_Mode_Enabled = false;
			GBP_Screen_Detection = false;
			GBP_Screen_Count = 0;

			LibSubGBAHawk.GBA_Hard_Reset(GBA_Pntr);
		}

		private IntPtr GBA_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GBA_core = new byte[0xA0000];

		private readonly GBA_ControllerDeck _controllerDeck;

		private int _frame = 0;

		public bool GBP_Mode_Enabled;
		public bool GBP_Screen_Detection;
		public int GBP_Screen_Count;

		public DisplayType Region => DisplayType.NTSC;

		private readonly ITraceable Tracer;

		private LibSubGBAHawk.TraceCallback tracecb;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		private void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			LibSubGBAHawk.GBA_getdisassembly(GBA_Pntr, new_d, t, Disasm_Length);
			LibSubGBAHawk.GBA_getregisterstate(GBA_Pntr, new_r, t, Reg_String_Length);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(80), registerInfo: new_r.ToString()));
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
			else
			{
				LibSubGBAHawk.GBA_setscanlinecallback(GBA_Pntr, _scanlineCallback, _scanlineCallbackLine);
			}
		}

		GBAGPUMemoryAreas Mem_Domains = new GBAGPUMemoryAreas();

		public GBAGPUMemoryAreas GetMemoryAreas()
		{
			Mem_Domains.vram = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 0);
			Mem_Domains.oam = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 1);
			Mem_Domains.palram = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 2);
			Mem_Domains.mmio = LibSubGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 3);

			return Mem_Domains;
		}

		private LibSubGBAHawk.MessageCallback GBA_message;

		private void GetMessage()
		{
			StringBuilder new_m = new StringBuilder(200);

			LibSubGBAHawk.GBA_getmessage(GBA_Pntr, new_m, 200);

			Console.WriteLine(new_m);
		}
	}
}
