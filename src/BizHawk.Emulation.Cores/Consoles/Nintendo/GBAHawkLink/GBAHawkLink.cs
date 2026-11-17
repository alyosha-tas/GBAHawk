using System;
using System.Text;
using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA;

/*
	GBA Emulator
	NOTES: 
	RAM disabling not implemented, check if used by any games

	EEPROM accesses only emulated at 0xDxxxxxx, check if any games use lower range
*/

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	[Core(CoreNames.GBAHawkLink, isReleased: true)]
	public partial class GBAHawkLink : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable,
								ISettable<GBAHawkLink.GBALinkSettings, GBAHawkLink.GBALinkSyncSettings>
	{
		public byte[] BIOS;

		public readonly byte[][] ROMS = new byte[4][];

		public uint[] ROMS_Length = new uint[4];

		public static readonly byte[] multi_boot_check = {0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x6E, 0x27, 0x74, 0x20, 0x61, 0x20, 0x52, 0x4F, 0x4D };

		public ushort controller_state_A, controller_state_B, controller_state_C, controller_state_D;
		public ushort Acc_X_state_A, Acc_X_state_B, Acc_X_state_C, Acc_X_state_D;
		public ushort Acc_Y_state_A, Acc_Y_state_B, Acc_Y_state_C, Acc_Y_state_D;
		public byte Solar_state_A = 0x50;
		public byte Solar_state_B = 0x50;
		public byte Solar_state_C = 0x50;
		public byte Solar_state_D = 0x50;

		public ushort[] Flash_Type_64_Values = new ushort[4];
		public ushort[] Flash_Type_128_Values = new ushort[4];
		public short[] EEPROM_Offsets = new short[4];
		public short[] Flash_Write_Offsets = new short[4];
		public int[] Flash_Sector_Erase_Offsets = new int[4];
		public int[] Flash_Chip_Erase_Offsets = new int[4];
		public bool[] Use_GBPs = new bool[4];

		public byte[][] cart_RAMS = new byte[4][];
		public uint[] Cart_RAM_Sizes = new uint[4];
		public bool[] has_bats = new bool[4];

		public int[] mappers = new int[4];

		public int Num_ROMS = 0;
		public bool Is_OneScreenMode = false;
		public bool _cablediscosignal, _cableconnected;
		public bool _cableconnected_AC, _cableconnected_BC, _cableconnected_AB;
		public bool _cableconnected_UD, _cableconnected_LR, _cableconnected_X, _cableconnected_4x;

		public int Actual_Width, Actual_Height;

		[CoreConstructor(VSystemID.Raw.GBAL)]
		public GBAHawkLink(CoreLoadParameters<GBAHawkLink.GBALinkSettings, GBAHawkLink.GBALinkSyncSettings> lp)
		{
			Num_ROMS = lp.Roms.Count;

			if ((Num_ROMS < 2) || (Num_ROMS > 4))
			{
				throw new Exception("Invalid number of ROMs");
			}

			ServiceProvider = new BasicServiceProvider(this);
			Settings = (GBALinkSettings)lp.Settings ?? new GBALinkSettings();
			SyncSettings = (GBALinkSyncSettings)lp.SyncSettings ?? new GBALinkSyncSettings();

			Is_OneScreenMode = SyncSettings.OneScreenMode;

			_cablediscosignal = false;
			_cableconnected = false;

			_cableconnected_AC = _cableconnected_BC = _cableconnected_AB = false;
			_cableconnected_UD = _cableconnected_LR = _cableconnected_X = _cableconnected_4x = false;

			for (int i = 0; i < Num_ROMS; i++)
			{
				video_buffers[i] = new int[240 * 160];
			}

			if (SyncSettings.OneScreenMode)
			{
				Actual_Width = 240;
				Actual_Height = 160;
			}
			else if (Num_ROMS == 2)
			{
				Actual_Width = 240 * 2 + 2;
				Actual_Height = 160;
			}
			else
			{
				Actual_Width = 240 * 2 + 2;
				Actual_Height = 160 * 2 + 2;
			}

			_vidbuffer = new int[Actual_Width * Actual_Height];

			// multi boot identified by special ROM
			bool[] is_multi_boot = new bool[4];

			is_multi_boot[0] = is_multi_boot[1] = is_multi_boot[2] = is_multi_boot[3] = true;

			ROMS[0] = new byte[0x6000000];
			ROMS[1] = new byte[0x6000000];
			ROMS[2] = new byte[0x6000000];
			ROMS[3] = new byte[0x6000000];

			int mirror_mode = 2;

			string[] controller_strings = new string[4];

			ulong[] date_times = new ulong[4];

			bool[] rtc_workings = new bool[4];

			GBALinkSyncSettings.InitRTCState[] RTC_Inits = new GBALinkSyncSettings.InitRTCState[4];
			DateTime temp_date_time = new DateTime();

			for (int i = 0; i < Num_ROMS; i++)
			{
				var rom = lp.Roms[i].RomData;

				var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
				Console.WriteLine(romHashSHA1);

				for (int j = 0; j < 16; j++)
				{
					if (lp.Roms[i].RomData[j] != multi_boot_check[j]) { is_multi_boot[i] = false; }
				}

				mirror_mode = GBACommonFunctions.Setup_Mirroring(romHashSHA1);

				if (is_multi_boot[i])
				{
					// replace with empty ROM
					for (int j = 0; j < 0x6000000; j += 2)
					{
						ROMS[i][j] = (byte)((j & 0xFF) >> 1);
						ROMS[i][j + 1] = (byte)(((j >> 8) & 0xFF) >> 1);
					}

					Console.WriteLine("No ROM inserted to console " + i);
				}
				else if (rom.Length > 0x6000000)
				{
					throw new Exception("Over size ROM?");
				}
				else
				{
					ROMS_Length[i] = (uint)rom.Length;
					Buffer.BlockCopy(rom, 0, ROMS[i], 0, rom.Length);

					if ((mirror_mode == 0) || (mirror_mode == 2))
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
								for (int j = 0; j < (0x6000000 - ofst_base); j += 2)
								{
									ROMS[i][j + ofst_base] = (byte)((j & 0xFF) >> 1);
									ROMS[i][j + ofst_base + 1] = (byte)(((j >> 8) & 0xFF) >> 1);
								}
							}
						}
					}
					else
					{
						// mirror the rom accross the whole region (might need different increment sizes for different ROMs)
						if (rom.Length <= 0x1000000)
						{
							for (int j = 0; j < rom.Length; j++)
							{
								ROMS[i][j + 0x1000000] = rom[j];
								ROMS[i][j + 0x2000000] = rom[j];
								ROMS[i][j + 0x3000000] = rom[j];
								ROMS[i][j + 0x4000000] = rom[j];
								ROMS[i][j + 0x5000000] = rom[j];
							}
						}
						else if (rom.Length <= 0x2000000)
						{
							for (int j = 0; j < rom.Length; j++)
							{
								ROMS[i][j + 0x2000000] = rom[j];
								ROMS[i][j + 0x4000000] = rom[j];
							}
						}

						// don't mirror at all for larger ROMs
					}
				}

				GBACommonFunctions.Setup_Mapper(romHashSHA1, ROMS[i], out mappers[i], out has_bats[i], out Cart_RAM_Sizes[i]);

				// alternate setup for video mapper
				if (mappers[i] == 9)
				{
					// registers return 0xFF;
					for (int j = 0x800000; j < 0x800200; j++)
					{
						ROMS[i][j] = 0xFF;
					}

					// blank values
					for (int j = 0x800200; j < 0x1000000; j += 2)
					{
						ROMS[i][j] = 0xAD;
						ROMS[i][j + 1] = 0xDE;
					}

					// rest of ROM is unmapped
					for (int j = 0; j < 0x5000000; j += 2)
					{
						ROMS[i][j + 0x1000000] = (byte)((j & 0xFF) >> 1);
						ROMS[i][j + 0x1000000 + 1] = (byte)(((j >> 8) & 0xFF) >> 1);
					}
				}

				if (Cart_RAM_Sizes[i] != 0)
				{
					cart_RAMS[i] = new byte[Cart_RAM_Sizes[i]];

					Console.WriteLine("SRAM Size: " + Cart_RAM_Sizes[i]);
				}

				if (cart_RAMS[i] != null)
				{
					// initialize SRAM to 0xFF;
					if ((mappers[i] == 2) || (mappers[i] == 3))
					{
						for (int j = 0; j < cart_RAMS[i].Length; j++)
						{
							cart_RAMS[i][j] = 0xFF;
						}
					}
					// initialize EEPROM to 0xFF;
					if ((mappers[i] == 4) || (mappers[i] == 5) || (mappers[i] == 6))
					{
						for (int j = 0; j < cart_RAMS[i].Length; j++)
						{
							cart_RAMS[i][j] = 0xFF;
						}
					}
					// initialize Flash to 0xFF;
					if ((mappers[i] == 7) || (mappers[i] == 8))
					{
						for (int j = 0; j < cart_RAMS[i].Length; j++)
						{
							cart_RAMS[i][j] = 0xFF;
						}
					}
				}

				// Setup the controllers
				string cntrllr = GBA_ControllerDeck.DefaultControllerName;

				if (mappers[i] == 3)
				{
					cntrllr = typeof(StandardZGyro).DisplayName();
				}
				else if (mappers[i] == 5)
				{
					cntrllr = typeof(StandardTilt).DisplayName();
				}
				else if (mappers[i] == 6)
				{
					if (SyncSettings.Use_Discrete_Solar_A)
					{
						cntrllr = typeof(DiscreteSolar).DisplayName();
					}
					else
					{
						cntrllr = typeof(StandardSolar).DisplayName();
					}
				}

				controller_strings[i] = cntrllr;

				// set up RTC
				date_times[i] = 0;

				rtc_workings[i] = true;

				byte temp_year = 0;
				byte temp_month = 1;
				byte temp_day = 1;
				byte temp_week = 0;
				byte temp_hour = 0;
				byte temp_minute = 0;
				byte temp_second = 0;
				byte temp_ctrl = 0;

				if (RTC_Inits[i] == GBALinkSyncSettings.InitRTCState.Reset_Bad_Batt)
				{
					rtc_workings[i] = false;
				}
				else if (RTC_Inits[i] == GBALinkSyncSettings.InitRTCState.RTC_Set)
				{
					// all games seem to use 24 hour mode,, so use this to represent set time
					temp_ctrl = 0x40;

					// parse the date and time into the regs
					if (i == 0) { temp_date_time = SyncSettings.RTCInitialTime_A; }
					if (i == 1) { temp_date_time = SyncSettings.RTCInitialTime_B; }
					if (i == 2) { temp_date_time = SyncSettings.RTCInitialTime_C; }
					if (i == 3) { temp_date_time = SyncSettings.RTCInitialTime_D; }

					// if year outside range of RTC, just leave the initial values
					if ((temp_date_time.Year < 2100) && (temp_date_time.Year >= 2000))
					{
						temp_year = To_BCD((byte)(temp_date_time.Year - 2000));
						temp_month = To_BCD((byte)temp_date_time.Month);
						temp_day = To_BCD((byte)temp_date_time.Day);
						temp_week = To_BCD((byte)temp_date_time.DayOfWeek);
						temp_minute = To_BCD((byte)temp_date_time.Minute);
						temp_second = To_BCD((byte)temp_date_time.Second);

						temp_hour = To_BCD((byte)temp_date_time.Hour);

						if (temp_date_time.Hour >= 12)
						{
							temp_hour |= 0x80;
						}
					}
				}

				date_times[i] |= temp_second;
				date_times[i] |= ((ulong)temp_minute << 8);
				date_times[i] |= ((ulong)temp_hour << 16);
				date_times[i] |= ((ulong)temp_week << 24);
				date_times[i] |= ((ulong)temp_day << 32);
				date_times[i] |= ((ulong)temp_month << 40);
				date_times[i] |= ((ulong)temp_year << 48);
				date_times[i] |= ((ulong)temp_ctrl << 56);
			}

			Put_Settings_To_Array();

			// Load up a BIOS and initialize the correct PPU
			BIOS = lp.Comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			GBA_Pntr = LibGBAHawkLink.GBALink_create((uint)Num_ROMS);

			LibGBAHawkLink.GBALink_load_bios(GBA_Pntr, BIOS);

			rumblecbA = MakeRumbleA;
			rumblecbB = MakeRumbleB;
			rumblecbC = MakeRumbleC;
			rumblecbD = MakeRumbleD;

			LibGBAHawkLink.GBA_setrumblecallback(GBA_Pntr, rumblecbA, rumblecbB, rumblecbC, rumblecbD);

			GBA_message_L = GetMessage_L;
			GBA_message_A = GetMessage_A;
			GBA_message_B = GetMessage_B;
			GBA_message_C = GetMessage_C;
			GBA_message_D = GetMessage_D;

			LibGBAHawkLink.GBALink_setmessagecallback(GBA_Pntr, GBA_message_L, GBA_message_A, GBA_message_B, GBA_message_C, GBA_message_D);

			Console.WriteLine("Mapper: " + mappers[0] + " " + +mappers[1] + " " + +mappers[2] + " " + +mappers[3]);

			byte[] vid_rom = new byte[0x4000000];

			for (int i = 0; i < Num_ROMS; i++)
			{
				LibGBAHawkLink.GBALink_load(GBA_Pntr, ROMS[i], (uint)ROMS_Length[i], mappers[i],
												  date_times[i], rtc_workings[i], EEPROM_Offsets[i],
												  Flash_Type_64_Values[i], Flash_Type_128_Values[i],
												  Flash_Write_Offsets[i], Flash_Sector_Erase_Offsets[i],
												  Flash_Chip_Erase_Offsets[i], Use_GBPs[i], (uint)i);

				if (cart_RAMS[i] != null) { LibGBAHawkLink.GBALink_create_SRAM(GBA_Pntr, cart_RAMS[i], (uint)cart_RAMS[i].Length, (uint)i); }

				// video sent to core garaunteed to be 0x4000000 bytes
				if (mappers[i] == 9)
				{
					var rom = lp.Roms[i].RomData;

					if (rom.Length < 0x4000000)
					{
						Buffer.BlockCopy(rom, 0, vid_rom, 0, rom.Length);

						LibGBAHawkLink.GBALink_load_video(GBA_Pntr, vid_rom, (uint)i);
					}
					else
					{
						Buffer.BlockCopy(rom, 0, vid_rom, 0, 0x4000000);

						LibGBAHawkLink.GBALink_load_video(GBA_Pntr, vid_rom, (uint)i);
					}
				}
			}
			
			blip_L_0.SetRates(4194304 * 4, 44100);
			blip_R_0.SetRates(4194304 * 4, 44100);
			blip_L_1.SetRates(4194304 * 4, 44100);
			blip_R_1.SetRates(4194304 * 4, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			SetupMemoryDomains();

			Header_Length = LibGBAHawkLink.GBALink_getheaderlength(GBA_Pntr);
			Disasm_Length = LibGBAHawkLink.GBALink_getdisasmlength(GBA_Pntr);
			Reg_String_Length = LibGBAHawkLink.GBALink_getregstringlength(GBA_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibGBAHawkLink.GBALink_getheader(GBA_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			_controllerDeck = new(controller_strings, Num_ROMS);

			HardReset();
		}

		public void Put_Settings_To_Array()
		{
			if (SyncSettings.Flash_Type_64_A == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Atmel) { Flash_Type_64_Values[0] = 0x3D1F; }
			if (SyncSettings.Flash_Type_64_A == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Macronix) { Flash_Type_64_Values[0] = 0x1CC2; }
			if (SyncSettings.Flash_Type_64_A == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Panasonic) { Flash_Type_64_Values[0] = 0x1B32; }
			if (SyncSettings.Flash_Type_64_A == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.SST) { Flash_Type_64_Values[0] = 0xD4BF; }

			if (SyncSettings.Flash_Type_128_A == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Macronix) { Flash_Type_128_Values[0] = 0x09C2; }
			if (SyncSettings.Flash_Type_128_A == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Sanyo) { Flash_Type_128_Values[0] = 0x1362; }

			if (SyncSettings.Flash_Type_64_B == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Atmel) { Flash_Type_64_Values[1] = 0x3D1F; }
			if (SyncSettings.Flash_Type_64_B == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Macronix) { Flash_Type_64_Values[1] = 0x1CC2; }
			if (SyncSettings.Flash_Type_64_B == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Panasonic) { Flash_Type_64_Values[1] = 0x1B32; }
			if (SyncSettings.Flash_Type_64_B == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.SST) { Flash_Type_64_Values[1] = 0xD4BF; }

			if (SyncSettings.Flash_Type_128_B == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Macronix) { Flash_Type_128_Values[1] = 0x09C2; }
			if (SyncSettings.Flash_Type_128_B == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Sanyo) { Flash_Type_128_Values[1] = 0x1362; }

			if (SyncSettings.Flash_Type_64_C == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Atmel) { Flash_Type_64_Values[2] = 0x3D1F; }
			if (SyncSettings.Flash_Type_64_C == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Macronix) { Flash_Type_64_Values[2] = 0x1CC2; }
			if (SyncSettings.Flash_Type_64_C == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Panasonic) { Flash_Type_64_Values[2] = 0x1B32; }
			if (SyncSettings.Flash_Type_64_C == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.SST) { Flash_Type_64_Values[2] = 0xD4BF; }

			if (SyncSettings.Flash_Type_128_C == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Macronix) { Flash_Type_128_Values[2] = 0x09C2; }
			if (SyncSettings.Flash_Type_128_C == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Sanyo) { Flash_Type_128_Values[2] = 0x1362; }

			if (SyncSettings.Flash_Type_64_D == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Atmel) { Flash_Type_64_Values[3] = 0x3D1F; }
			if (SyncSettings.Flash_Type_64_D == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Macronix) { Flash_Type_64_Values[3] = 0x1CC2; }
			if (SyncSettings.Flash_Type_64_D == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Panasonic) { Flash_Type_64_Values[3] = 0x1B32; }
			if (SyncSettings.Flash_Type_64_D == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.SST) { Flash_Type_64_Values[3] = 0xD4BF; }

			if (SyncSettings.Flash_Type_128_D == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Macronix) { Flash_Type_128_Values[3] = 0x09C2; }
			if (SyncSettings.Flash_Type_128_D == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Sanyo) { Flash_Type_128_Values[3] = 0x1362; }

			EEPROM_Offsets[0] = SyncSettings.EEPROMOffset_A;
			EEPROM_Offsets[1] = SyncSettings.EEPROMOffset_B;
			EEPROM_Offsets[2] = SyncSettings.EEPROMOffset_C;
			EEPROM_Offsets[3] = SyncSettings.EEPROMOffset_D;

			Flash_Write_Offsets[0] = SyncSettings.FlashWriteOffset_A;
			Flash_Write_Offsets[1] = SyncSettings.FlashWriteOffset_B;
			Flash_Write_Offsets[2] = SyncSettings.FlashWriteOffset_C;
			Flash_Write_Offsets[3] = SyncSettings.FlashWriteOffset_D;

			Flash_Sector_Erase_Offsets[0] = SyncSettings.FlashSectorEraseOffset_A;
			Flash_Sector_Erase_Offsets[1] = SyncSettings.FlashSectorEraseOffset_B;
			Flash_Sector_Erase_Offsets[2] = SyncSettings.FlashSectorEraseOffset_C;
			Flash_Sector_Erase_Offsets[3] = SyncSettings.FlashSectorEraseOffset_D;

			Flash_Chip_Erase_Offsets[0] = SyncSettings.FlashSectorEraseOffset_A;
			Flash_Chip_Erase_Offsets[1] = SyncSettings.FlashSectorEraseOffset_B;
			Flash_Chip_Erase_Offsets[2] = SyncSettings.FlashSectorEraseOffset_C;
			Flash_Chip_Erase_Offsets[3] = SyncSettings.FlashSectorEraseOffset_D;

			Use_GBPs[0] = SyncSettings.Use_GBP_A;
			Use_GBPs[1] = SyncSettings.Use_GBP_B;
			Use_GBPs[2] = SyncSettings.Use_GBP_C;
			Use_GBPs[3] = SyncSettings.Use_GBP_D;
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

		public ulong TotalExecutedCycles => 0;

		public void HardReset()
		{
			GBP_Mode_Enabled_A = GBP_Mode_Enabled_B = GBP_Mode_Enabled_C = GBP_Mode_Enabled_D = false;
			GBP_Screen_Detection_A = GBP_Screen_Detection_B = GBP_Screen_Detection_C = GBP_Screen_Detection_D = false;
			GBP_Screen_Count_A = GBP_Screen_Count_B = GBP_Screen_Count_C = GBP_Screen_Count_D = 0;

			LibGBAHawkLink.GBALink_Hard_Reset(GBA_Pntr);
		}

		private IntPtr GBA_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GBA_core = new byte[0xB0000 * 4];

		private readonly GBALink_ControllerDeck _controllerDeck;

		private int _frame = 0;

		public bool GBP_Mode_Enabled_A, GBP_Mode_Enabled_B, GBP_Mode_Enabled_C, GBP_Mode_Enabled_D;
		public bool GBP_Screen_Detection_A, GBP_Screen_Detection_B, GBP_Screen_Detection_C, GBP_Screen_Detection_D;
		public int GBP_Screen_Count_A, GBP_Screen_Count_B, GBP_Screen_Count_C, GBP_Screen_Count_D;

		private readonly ITraceable Tracer;

		private LibGBAHawkLink.TraceCallback tracecb;

		private LibGBAHawkLink.RumbleCallback rumblecbA;
		private LibGBAHawkLink.RumbleCallback rumblecbB;
		private LibGBAHawkLink.RumbleCallback rumblecbC;
		private LibGBAHawkLink.RumbleCallback rumblecbD;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		private void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			uint tracer_core = (uint)Settings.TraceSet;

			LibGBAHawkLink.GBALink_getdisassembly(GBA_Pntr, new_d, t, Disasm_Length, tracer_core);
			LibGBAHawkLink.GBALink_getregisterstate(GBA_Pntr, new_r, t, Reg_String_Length, tracer_core);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(80), registerInfo: new_r.ToString()));
		}

		private void MakeRumbleA(bool rumble_on) { if (Controller != null) { Controller.SetHapticChannelStrength("P1 Rumble", rumble_on ? 255 : 0); } }
		private void MakeRumbleB(bool rumble_on) { if (Controller != null) { Controller.SetHapticChannelStrength("P2 Rumble", rumble_on ? 255 : 0); } }
		private void MakeRumbleC(bool rumble_on) { if (Controller != null) { Controller.SetHapticChannelStrength("P3 Rumble", rumble_on ? 255 : 0); } }
		private void MakeRumbleD(bool rumble_on) { if (Controller != null) { Controller.SetHapticChannelStrength("P4 Rumble", rumble_on ? 255 : 0); } }

		private LibGBAHawkLink.MessageCallback GBA_message_L;
		private LibGBAHawkLink.MessageCallback GBA_message_A;
		private LibGBAHawkLink.MessageCallback GBA_message_B;
		private LibGBAHawkLink.MessageCallback GBA_message_C;
		private LibGBAHawkLink.MessageCallback GBA_message_D;

		private void GetMessage_L(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBAHawkLink.GBALink_getmessage(GBA_Pntr, new_m, 5);

			Console.WriteLine(new_m);
		}

		private void GetMessage_A(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBAHawkLink.GBALink_getmessage(GBA_Pntr, new_m, 0);

			Console.WriteLine(new_m);
		}

		private void GetMessage_B(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBAHawkLink.GBALink_getmessage(GBA_Pntr, new_m, 1);

			Console.WriteLine(new_m);
		}

		private void GetMessage_C(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBAHawkLink.GBALink_getmessage(GBA_Pntr, new_m, 2);

			Console.WriteLine(new_m);
		}

		private void GetMessage_D(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBAHawkLink.GBALink_getmessage(GBA_Pntr, new_m, 3);

			Console.WriteLine(new_m);
		}
	}
}
