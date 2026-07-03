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
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBAHawkLink : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable, IRegionable,
								ISettable<GBAHawkLink.GBALinkSettings, GBAHawkLink.GBALinkSyncSettings>
	{
		public byte[] BIOS;

		public readonly byte[][] ROMS = new byte[2][];

		public uint[] ROMS_Length = new uint[2];

		public static readonly byte[] multi_boot_check = {0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x6E, 0x27, 0x74, 0x20, 0x61, 0x20, 0x52, 0x4F, 0x4D };

		public ushort controller_state_1, controller_state_2;
		public ushort Acc_X_state_1, Acc_X_state_2;
		public ushort Acc_Y_state_1, Acc_Y_state_2;
		public byte Solar_state_1 = 0x50;
		public byte Solar_state_2 = 0x50;

		public ushort Flash_Type_64_Value_L = 0;
		public ushort Flash_Type_128_Value_L = 0;
		public ushort Flash_Type_64_Value_R = 0;
		public ushort Flash_Type_128_Value_R = 0;

		public byte[][] cart_RAMS = new byte[2][];
		public uint[] Cart_RAM_Sizes = new uint[2];
		public bool[] has_bats = new bool[2];

		public int[] mappers = new int[2];

		[CoreConstructor(VSystemID.Raw.GBAL)]
		public GBAHawkLink(CoreLoadParameters<GBAHawkLink.GBALinkSettings, GBAHawkLink.GBALinkSyncSettings> lp)
		{
			if (lp.Roms.Count != 2)
				throw new InvalidOperationException("Wrong number of roms");
			
			// multi boot identified by special ROM
			bool[] is_multi_boot = new bool[2];

			is_multi_boot[0] = is_multi_boot[1] = true;

			ServiceProvider = new BasicServiceProvider(this);
			Settings = (GBALinkSettings)lp.Settings ?? new GBALinkSettings();
			SyncSettings = (GBALinkSyncSettings)lp.SyncSettings ?? new GBALinkSyncSettings();

			ROMS[0] = new byte[0x6000000];
			ROMS[1] = new byte[0x6000000];

			for (int i = 0; i < 2; i++)
			{
				var rom = lp.Roms[i].RomData;

				var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
				Console.WriteLine(romHashMD5);
				var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
				Console.WriteLine(romHashSHA1);

				for (int j = 0; j < 16; j++)
				{
					if (lp.Roms[i].RomData[j] != multi_boot_check[j]) { is_multi_boot[i] = false; }
				}

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

					// some roms expect a different repetition pattern (probably depends on chip size)
					if ((romHashSHA1 == "SHA1:676357A271981699D97CFE1730F0AB3D07CE1F69") || // gba ldm-stm
						(romHashSHA1 == "SHA1:D336E8B65FBC4DE054FCEDE16CFE1B1978EAAEE4"))   // armfuck
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
					else if ((romHashSHA1 != "SHA1:5F989B9A4017F16A431F76FD78A95E9799AA8FCA") && // GBA Suite Memory test
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
						for (int j = 0; j < rom.Length; j++)
						{
							ROMS[i][j + 0x2000000] = rom[j];
							ROMS[i][j + 0x4000000] = rom[j];
						}
					}
				}

				GBACommonFunctions.Setup_Mapper(romHashMD5, romHashSHA1, ROMS[i], out mappers[i], out has_bats[i], out Cart_RAM_Sizes[i]);

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
			}

			if (SyncSettings.Flash_Type_64_L == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Atmel) { Flash_Type_64_Value_L = 0x3D1F; }
			if (SyncSettings.Flash_Type_64_L == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Macronix) { Flash_Type_64_Value_L = 0x1CC2; }
			if (SyncSettings.Flash_Type_64_L == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Panasonic) { Flash_Type_64_Value_L = 0x1B32; }
			if (SyncSettings.Flash_Type_64_L == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.SST) { Flash_Type_64_Value_L = 0xD4BF; }

			if (SyncSettings.Flash_Type_128_L == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Macronix) { Flash_Type_128_Value_L = 0x09C2; }
			if (SyncSettings.Flash_Type_128_L == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Sanyo) { Flash_Type_128_Value_L = 0x1362; }

			if (SyncSettings.Flash_Type_64_R == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Atmel) { Flash_Type_64_Value_R = 0x3D1F; }
			if (SyncSettings.Flash_Type_64_R == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Macronix) { Flash_Type_64_Value_R = 0x1CC2; }
			if (SyncSettings.Flash_Type_64_R == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.Panasonic) { Flash_Type_64_Value_R = 0x1B32; }
			if (SyncSettings.Flash_Type_64_R == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType64.SST) { Flash_Type_64_Value_R = 0xD4BF; }

			if (SyncSettings.Flash_Type_128_R == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Macronix) { Flash_Type_128_Value_R = 0x09C2; }
			if (SyncSettings.Flash_Type_128_R == GBALink.GBAHawkLink.GBALinkSyncSettings.FlashChipType128.Sanyo) { Flash_Type_128_Value_R = 0x1362; }

			// Load up a BIOS and initialize the correct PPU
			BIOS = lp.Comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			GBA_Pntr = LibGBAHawkLink.GBALink_create();

			LibGBAHawkLink.GBALink_load_bios(GBA_Pntr, BIOS);

			// load 0 RTC
			bool rtc_working_0 = true;

			byte temp_year_0 = 0;
			byte temp_month_0 = 1;
			byte temp_day_0 = 1;
			byte temp_week_0 = 0;
			byte temp_hour_0 = 0;
			byte temp_minute_0 = 0;
			byte temp_second_0 = 0;
			byte temp_ctrl_0 = 0;

			if (SyncSettings.RTCInitialState_L == GBALinkSyncSettings.InitRTCState.Reset_Bad_Batt)
			{
				rtc_working_0 = false;
			}
			else if (SyncSettings.RTCInitialState_L == GBALinkSyncSettings.InitRTCState.RTC_Set)
			{
				// all games seem to use 24 hour mode,, so use this to represent set time
				temp_ctrl_0 = 0x40;

				// parse the date and time into the regs
				DateTime temp_0 = SyncSettings.RTCInitialTime_L;

				// if year outside range of RTC, just leave the initial values
				if ((temp_0.Year < 2100) && (temp_0.Year >= 2000))
				{
					temp_year_0 = To_BCD((byte)(temp_0.Year - 2000));
					temp_month_0 = To_BCD((byte)temp_0.Month);
					temp_day_0 = To_BCD((byte)temp_0.Day);
					temp_week_0 = To_BCD((byte)temp_0.DayOfWeek);
					temp_minute_0 = To_BCD((byte)temp_0.Minute);
					temp_second_0 = To_BCD((byte)temp_0.Second);

					temp_hour_0 = To_BCD((byte)temp_0.Hour);

					if (temp_0.Hour >= 12)
					{
						temp_hour_0 |= 0x80;
					}
				}
			}

			ulong date_time_0 = 0;

			date_time_0 |= temp_second_0;
			date_time_0 |= ((ulong)temp_minute_0 << 8);
			date_time_0 |= ((ulong)temp_hour_0 << 16);
			date_time_0 |= ((ulong)temp_week_0 << 24);
			date_time_0 |= ((ulong)temp_day_0 << 32);
			date_time_0 |= ((ulong)temp_month_0 << 40);
			date_time_0 |= ((ulong)temp_year_0 << 48);
			date_time_0 |= ((ulong)temp_ctrl_0  << 56);

			// load 1 RTC
			bool rtc_working_1 = true;

			byte temp_year_1 = 0;
			byte temp_month_1 = 1;
			byte temp_day_1 = 1;
			byte temp_week_1 = 0;
			byte temp_hour_1 = 0;
			byte temp_minute_1 = 0;
			byte temp_second_1 = 0;
			byte temp_ctrl_1 = 0;

			if (SyncSettings.RTCInitialState_R == GBALinkSyncSettings.InitRTCState.Reset_Bad_Batt)
			{
				rtc_working_1 = false;
			}
			else if (SyncSettings.RTCInitialState_R == GBALinkSyncSettings.InitRTCState.RTC_Set)
			{
				// all games seem to use 24 hour mode,, so use this to represent set time
				temp_ctrl_1 = 0x40;

				// parse the date and time into the regs
				DateTime temp_1 = SyncSettings.RTCInitialTime_R;

				// if year outside range of RTC, just leave the initial values
				if ((temp_1.Year < 2100) && (temp_1.Year >= 2000))
				{
					temp_year_1 = To_BCD((byte)(temp_1.Year - 2000));
					temp_month_1 = To_BCD((byte)temp_1.Month);
					temp_day_1 = To_BCD((byte)temp_1.Day);
					temp_week_1 = To_BCD((byte)temp_1.DayOfWeek);
					temp_minute_1 = To_BCD((byte)temp_1.Minute);
					temp_second_1 = To_BCD((byte)temp_1.Second);

					temp_hour_1 = To_BCD((byte)temp_1.Hour);

					if (temp_1.Hour >= 12)
					{
						temp_hour_1 |= 0x80;
					}
				}
			}

			ulong date_time_1 = 0;

			date_time_1 |= temp_second_1;
			date_time_1 |= ((ulong)temp_minute_1 << 8);
			date_time_1 |= ((ulong)temp_hour_1 << 16);
			date_time_1 |= ((ulong)temp_week_1 << 24);
			date_time_1 |= ((ulong)temp_day_1 << 32);
			date_time_1 |= ((ulong)temp_month_1 << 40);
			date_time_1 |= ((ulong)temp_year_1 << 48);
			date_time_1 |= ((ulong)temp_ctrl_1 << 56);

			rumblecb0 = MakeRumble0;
			rumblecb1 = MakeRumble1;

			LibGBAHawkLink.GBA_setrumblecallback(GBA_Pntr, rumblecb0, 0);
			LibGBAHawkLink.GBA_setrumblecallback(GBA_Pntr, rumblecb1, 1);

			GBA_message_L = GetMessage_L;
			GBA_message_R = GetMessage_R;

			LibGBAHawkLink.GBALink_setmessagecallback(GBA_Pntr, GBA_message_L, GBA_message_R);

			Console.WriteLine("Mapper: " + mappers[0] + " " + +mappers[1]);

			LibGBAHawkLink.GBALink_load(GBA_Pntr, ROMS[0], (uint)ROMS_Length[0], mappers[0],
												  ROMS[1], (uint)ROMS_Length[1], mappers[1],
												  date_time_0, rtc_working_0, date_time_1, rtc_working_1,
												  SyncSettings.EEPROMOffset_L, SyncSettings.EEPROMOffset_R,
												  Flash_Type_64_Value_L, Flash_Type_64_Value_R,
												  Flash_Type_128_Value_L, Flash_Type_128_Value_R,
												  SyncSettings.FlashWriteOffset_L, SyncSettings.FlashWriteOffset_R,
												  SyncSettings.FlashSectorEraseOffset_L, SyncSettings.FlashSectorEraseOffset_R,
												  SyncSettings.FlashChipEraseOffset_L, SyncSettings.FlashChipEraseOffset_R,
												  SyncSettings.Use_GBP_L, SyncSettings.Use_GBP_R);

			if (cart_RAMS[0] != null) { LibGBAHawkLink.GBALink_create_SRAM(GBA_Pntr, cart_RAMS[0], (uint)cart_RAMS[0].Length, 0); }
			if (cart_RAMS[1] != null) { LibGBAHawkLink.GBALink_create_SRAM(GBA_Pntr, cart_RAMS[1], (uint)cart_RAMS[1].Length, 1); }

			blip_L.SetRates(4194304 * 4, 44100);
			blip_R.SetRates(4194304 * 4, 44100);

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

			string cntrllr1 = GBA_ControllerDeck.DefaultControllerName;
			string cntrllr2 = GBA_ControllerDeck.DefaultControllerName;

			if (mappers[0] == 3)
			{
				cntrllr1 = typeof(StandardZGyro).DisplayName();
			}
			else if (mappers[0] == 5)
			{
				cntrllr1 = typeof(StandardTilt).DisplayName();
			}
			else if (mappers[0] == 6)
			{
				if (SyncSettings.Use_Discrete_Solar_L)
				{
					cntrllr1 = typeof(DiscreteSolar).DisplayName();
				}
				else
				{
					cntrllr1 = typeof(StandardSolar).DisplayName();
				}
			}

			if (mappers[1] == 3)
			{
				cntrllr2 = typeof(StandardZGyro).DisplayName();
			}
			else if (mappers[1] == 5)
			{
				cntrllr2 = typeof(StandardTilt).DisplayName();
			}
			else if (mappers[1] == 6)
			{
				if (SyncSettings.Use_Discrete_Solar_R)
				{
					cntrllr2 = typeof(DiscreteSolar).DisplayName();
				}
				else
				{
					cntrllr2 = typeof(StandardSolar).DisplayName();
				}
			}

			_controllerDeck = new(cntrllr1, cntrllr2);

			LibGBAHawkLink.GBALink_Hard_Reset(GBA_Pntr);
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
			GBP_Mode_Enabled_L = false;
			GBP_Screen_Detection_L = false;
			GBP_Screen_Count_L = 0;

			GBP_Mode_Enabled_R = false;
			GBP_Screen_Detection_R = false;
			GBP_Screen_Count_R = 0;

			LibGBAHawkLink.GBALink_Hard_Reset(GBA_Pntr);
		}

		private IntPtr GBA_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GBA_core = new byte[0xA0000 * 2];

		private readonly GBALink_ControllerDeck _controllerDeck;

		private int _frame = 0;

		public bool GBP_Mode_Enabled_L;
		public bool GBP_Screen_Detection_L;
		public int GBP_Screen_Count_L;

		public bool GBP_Mode_Enabled_R;
		public bool GBP_Screen_Detection_R;
		public int GBP_Screen_Count_R;

		public DisplayType Region => DisplayType.NTSC;

		private readonly ITraceable Tracer;

		private LibGBAHawkLink.TraceCallback tracecb;

		private LibGBAHawkLink.RumbleCallback rumblecb0;
		private LibGBAHawkLink.RumbleCallback rumblecb1;

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

		private void MakeRumble0(bool rumble_on)
		{
			if (Controller != null) { Controller.SetHapticChannelStrength("P1 Rumble", rumble_on ? 255 : 0); }
		}

		private void MakeRumble1(bool rumble_on)
		{
			if (Controller != null) { Controller.SetHapticChannelStrength("P2 Rumble", rumble_on ? 255 : 0); }
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

		private LibGBAHawkLink.MessageCallback GBA_message_L;
		private LibGBAHawkLink.MessageCallback GBA_message_R;

		private void GetMessage_L(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBAHawkLink.GBALink_getmessage(GBA_Pntr, new_m, 0);

			Console.WriteLine(new_m);
		}

		private void GetMessage_R(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBAHawkLink.GBALink_getmessage(GBA_Pntr, new_m, 1);

			Console.WriteLine(new_m);
		}
	}
}
