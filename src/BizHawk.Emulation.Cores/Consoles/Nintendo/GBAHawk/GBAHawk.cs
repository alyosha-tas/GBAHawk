using System;
using System.Text;
using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using BizHawk.Common.ReflectionExtensions;
using System.Security.Cryptography;

/*
	GBA Emulator
	NOTES: 
	RAM disabling not implemented, check if used by any games

	EEPROM accesses only emulated at 0xDxxxxxx, check if any games use lower range
*/

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	[Core(CoreNames.GBAHawk, isReleased: true)]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBAHawk : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable, IRegionable, IGBAGPUViewable,
								ISettable<GBAHawk.GBASettings, GBAHawk.GBASyncSettings>
	{
		public byte[] BIOS;

		public readonly byte[] ROM = new byte[0x6000000];
		public readonly byte[] header = new byte[0x50];

		public uint ROM_Length;

		public ushort controller_state;
		public ushort Acc_X_state;
		public ushort Acc_Y_state;
		public byte Solar_state = 0x50;

		public ushort Flash_Type_64_Value = 0;
		public ushort Flash_Type_128_Value = 0;
		public byte[] cart_RAM;
		public uint Cart_RAM_Size;
		public bool has_bat;
		public int mapper;

		[CoreConstructor(VSystemID.Raw.GBA)]
		public GBAHawk(CoreComm comm, GameInfo game, byte[] rom, GBAHawk.GBASettings settings, GBAHawk.GBASyncSettings syncSettings)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (GBASettings)settings ?? new GBASettings();
			SyncSettings = (GBASyncSettings)syncSettings ?? new GBASyncSettings();

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

				// some roms expect a different repetition pattern (probably depends on chip size)
				if ((romHashSHA1 == "SHA1:676357A271981699D97CFE1730F0AB3D07CE1F69") || // gba ldm-stm
					(romHashSHA1 == "SHA1:D336E8B65FBC4DE054FCEDE16CFE1B1978EAAEE4"))   // armfuck
				{
					for (int i = 0; i < rom.Length; i++)
					{
						ROM[i + 0x1000000] = rom[i];
						ROM[i + 0x2000000] = rom[i];
						ROM[i + 0x3000000] = rom[i];
						ROM[i + 0x4000000] = rom[i];
						ROM[i + 0x5000000] = rom[i];
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

			Buffer.BlockCopy(ROM, 0x100, header, 0, 0x50);

			GBACommonFunctions.Setup_Mapper(romHashMD5, romHashSHA1, ROM, out mapper, out has_bat, out Cart_RAM_Size);

			if (Cart_RAM_Size != 0)
			{
				cart_RAM = new byte[Cart_RAM_Size];

				Console.WriteLine("SRAM Size: " + Cart_RAM_Size);
			}

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
				// initialize Flash to 0xFF;
				if ((mapper == 7) || (mapper == 8))
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
			}

			if (SyncSettings.Flash_Type_64 == GBA.GBAHawk.GBASyncSettings.FlashChipType64.Atmel) { Flash_Type_64_Value = 0x3D1F; }
			if (SyncSettings.Flash_Type_64 == GBA.GBAHawk.GBASyncSettings.FlashChipType64.Macronix) { Flash_Type_64_Value = 0x1CC2; }
			if (SyncSettings.Flash_Type_64 == GBA.GBAHawk.GBASyncSettings.FlashChipType64.Panasonic) { Flash_Type_64_Value = 0x1B32; }
			if (SyncSettings.Flash_Type_64 == GBA.GBAHawk.GBASyncSettings.FlashChipType64.SST) { Flash_Type_64_Value = 0xD4BF; }

			if (SyncSettings.Flash_Type_128 == GBA.GBAHawk.GBASyncSettings.FlashChipType128.Macronix) { Flash_Type_128_Value = 0x09C2; }
			if (SyncSettings.Flash_Type_128 == GBA.GBAHawk.GBASyncSettings.FlashChipType128.Sanyo) { Flash_Type_128_Value = 0x1362; }

			// Load up a BIOS and initialize the correct PPU
			BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			GBA_Pntr = LibGBAHawk.GBA_create();

			LibGBAHawk.GBA_load_bios(GBA_Pntr, BIOS);

			bool rtc_working = true;

			byte temp_year = 0;
			byte temp_month = 1;
			byte temp_day = 1;
			byte temp_week = 0;
			byte temp_hour = 0;
			byte temp_minute = 0;
			byte temp_second = 0;
			byte temp_ctrl = 0;

			if (SyncSettings.RTCInitialState == GBASyncSettings.InitRTCState.Reset_Bad_Batt)
			{
				rtc_working = false;
			}
			else if (SyncSettings.RTCInitialState == GBASyncSettings.InitRTCState.RTC_Set)
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

			rumblecb = MakeRumble;

			LibGBAHawk.GBA_setrumblecallback(GBA_Pntr, rumblecb);

			GBA_message = GetMessage;

			LibGBAHawk.GBA_setmessagecallback(GBA_Pntr, GBA_message);

			Console.WriteLine("Mapper: " + mapper);

			LibGBAHawk.GBA_load(GBA_Pntr, ROM, (uint)ROM_Length, mapper, date_time, rtc_working, SyncSettings.EEPROMOffset, Flash_Type_64_Value, Flash_Type_128_Value,
								SyncSettings.FlashWriteOffset, SyncSettings.FlashSectorEraseOffset, SyncSettings.FlashChipEraseOffset, SyncSettings.Use_GBP);

			if (cart_RAM != null) { LibGBAHawk.GBA_create_SRAM(GBA_Pntr, cart_RAM, (uint)cart_RAM.Length); }

			blip_L.SetRates(4194304 * 4, 44100);
			blip_R.SetRates(4194304 * 4, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			SetupMemoryDomains();

			Header_Length = LibGBAHawk.GBA_getheaderlength(GBA_Pntr);
			Disasm_Length = LibGBAHawk.GBA_getdisasmlength(GBA_Pntr);
			Reg_String_Length = LibGBAHawk.GBA_getregstringlength(GBA_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibGBAHawk.GBA_getheader(GBA_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			if (mapper == 3)
			{
				_controllerDeck = new(typeof(StandardZGyro).DisplayName());
			}
			else if (mapper == 5)
			{
				_controllerDeck = new(typeof(StandardTilt).DisplayName());
			}
			else if (mapper == 6)
			{
				if (SyncSettings.Use_Discrete_Solar)
				{
					_controllerDeck = new(typeof(DiscreteSolar).DisplayName());
				}
				else
				{
					_controllerDeck = new(typeof(StandardSolar).DisplayName());
				}
			}
			else
			{
				_controllerDeck = new(GBA_ControllerDeck.DefaultControllerName);
			}

			Mem_Domains.vram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 0);
			Mem_Domains.oam = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 1);
			Mem_Domains.palram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 2);
			Mem_Domains.mmio = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 3);
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

			GBP_Mode_Enabled = false;
			GBP_Screen_Detection = false;
			GBP_Screen_Count = 0;

			LibGBAHawk.GBA_Hard_Reset(GBA_Pntr);
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

		private LibGBAHawk.TraceCallback tracecb;

		private LibGBAHawk.RumbleCallback rumblecb;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		private void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			LibGBAHawk.GBA_getdisassembly(GBA_Pntr, new_d, t, Disasm_Length);
			LibGBAHawk.GBA_getregisterstate(GBA_Pntr, new_r, t, Reg_String_Length);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(80), registerInfo: new_r.ToString()));
		}

		private void MakeRumble(bool rumble_on)
		{
			if (Controller != null) { Controller.SetHapticChannelStrength("P1 Rumble", rumble_on ? 255 : 0); }
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
				LibGBAHawk.GBA_setscanlinecallback(GBA_Pntr, _scanlineCallback, _scanlineCallbackLine);
			}
		}

		GBAGPUMemoryAreas Mem_Domains = new GBAGPUMemoryAreas();

		public GBAGPUMemoryAreas GetMemoryAreas()
		{
			Mem_Domains.vram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 0);
			Mem_Domains.oam = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 1);
			Mem_Domains.palram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 2);
			Mem_Domains.mmio = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 3);

			return Mem_Domains;
		}

		private LibGBAHawk.MessageCallback GBA_message;

		private void GetMessage(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length+1);

			LibGBAHawk.GBA_getmessage(GBA_Pntr, new_m);

			Console.WriteLine(new_m);
		}
	}
}
