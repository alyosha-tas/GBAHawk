using BizHawk.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using System;
using System.Text;

// TODO: mode1_disableint_gbc.gbc behaves differently between GBC and GBA, why?
// TODO: Window Position A6 behaves differently
// TODO: Verify open bus behaviour for bad SRAM accesses for other MBCs
// TODO: Apparently sprites at x=A7 do not stop the trigger for FF0F bit flip, but still do not dispatch interrupt or
// mode 3 change, see 10spritesPrLine_10xposA7_m0irq_2_dmg08_cgb04c_out2.gbc
// TODO: there is a tile glitch when setting LCDC.Bit(4) in GBC that is not implemented yet, the version of the glitch for reset is implemented though
// TODO: In some GBC models, apparently unmapped memory after OAM contains 48 bytes that are fully read/write'able
// this is not implemented and which models it effects is not clear, see oam_echo_ram_read.gbc and oam_echo_ram_read_2.gbc

namespace BizHawk.Emulation.Cores.Nintendo.GBHawk
{
	[Core(CoreNames.GBHawk, "")]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBHawk : IEmulator, ISaveRam, IInputPollable, IRegionable, IGBPPUViewable,
	ISettable<GBHawk.GBHawkSettings, GBHawk.GBHawkSyncSettings>
	{
		public bool Is_GBC;
		public bool is_GB_in_GBC;

		public byte[] BIOS;
		public readonly byte[] _rom;
		public readonly byte[] header = new byte[0x50];

		public byte[] cart_RAM;
		public byte[] cart_RAM_vbls;
		public bool has_bat;
		public int Cart_RAM_Size;
		public bool Use_MT;

		public bool is_subframe_core = false;

		private static readonly byte[] GBA_override = { 0xFF, 0x00, 0xCD, 0x03, 0x35, 0xAA, 0x31, 0x90, 0x94, 0x00, 0x00, 0x00, 0x00 };

		[CoreConstructor(VSystemID.Raw.GB)]
		[CoreConstructor(VSystemID.Raw.GBC)]
		public GBHawk(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ GBHawkSettings settings, GBHawkSyncSettings syncSettings, bool subframe = false)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (GBHawkSettings)settings ?? new GBHawkSettings();
			SyncSettings = (GBHawkSyncSettings)syncSettings ?? new GBHawkSyncSettings();

			is_subframe_core = subframe;

			Is_GBC = SyncSettings.ConsoleMode != GBHawkSyncSettings.ConsoleModeType.GB;

			Buffer.BlockCopy(rom, 0x100, header, 0, 0x50);

			if ((header[0x43] != 0x80) && (header[0x43] != 0xC0))
			{
				is_GB_in_GBC = true; // for movie files
			}

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			_rom = rom;
			string mppr;

			GBCommonFunctions.Setup_Mapper(romHashMD5, romHashSHA1, header, out mppr, out has_bat, out Cart_RAM_Size);

			if (Cart_RAM_Size != 0)
			{
				cart_RAM = new byte[Cart_RAM_Size];
				cart_RAM_vbls = new byte[Cart_RAM_Size];

				Console.Write("RAM: "); Console.WriteLine(Cart_RAM_Size);
			}

			_controllerDeck = new(mppr is "MBC7"
				? typeof(StandardTilt).DisplayName()
				: GBHawkControllerDeck.DefaultControllerName, subframe);

			if ((mppr == "MBC3") || (mppr == "HuC3") || (mppr == "TAMA5"))
			{
				Use_MT = true;
			}

			int years = 0;
			int days = 0;
			int days_upper = 0;
			int hours = 0 ;
			int minutes = 0;
			int minutes_upper = 0;
			int seconds = 0;
			int remaining = 0;



			if (mppr == "MBC3")
			{
				days = (int)Math.Floor(SyncSettings.RTCInitialTime / 86400.0);
				days_upper = ((days & 0x100) >> 8) | ((days & 0x200) >> 2);
				remaining = SyncSettings.RTCInitialTime - (days * 86400);
				hours = (int)Math.Floor(remaining / 3600.0);
				remaining = remaining - (hours * 3600);
				minutes = (int)Math.Floor(remaining / 60.0);
				seconds = remaining - (minutes * 60);
			}

			if (mppr == "HuC3")
			{
				years = (int)Math.Floor(SyncSettings.RTCInitialTime / 31536000.0);
				remaining = SyncSettings.RTCInitialTime - (years * 31536000);
				days = (int)Math.Floor(remaining / 86400.0);
				days_upper = (days >> 8) & 0xF;
				remaining = remaining - (days * 86400);
				minutes = (int)Math.Floor(remaining / 60.0);
				minutes_upper = (minutes >> 8) & 0xF;
			}


			if (Is_GBC)
			{
				GB_Pntr = LibGBCHawk.GBC_create();

				GBC_message = GetMessageGBC;

				LibGBCHawk.GBC_setmessagecallback(GB_Pntr, GBC_message);

				BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBC", "World"), "BIOS Not Found, Cannot Load");

				// Here we modify the BIOS if GBA mode is set (credit to ExtraTricky)
				if (SyncSettings.GBACGB)
				{
					for (int i = 0; i < 13; i++)
					{
						BIOS[i + 0xF3] = (byte)((GBA_override[i] + BIOS[i + 0xF3]) & 0xFF);
					}
				}

				LibGBCHawk.GBC_load_bios(GB_Pntr, BIOS);

				LibGBCHawk.GBC_load(GB_Pntr, rom, (uint)rom.Length);

				if (mppr == "MBC3")
				{
					mapper.RTC_Get(SyncSettings.RTCOffset, 5);
					mapper.RTC_Get(days_upper, 4);
					mapper.RTC_Get(days & 0xFF, 3);
					mapper.RTC_Get(hours & 0xFF, 2);
					mapper.RTC_Get(minutes & 0xFF, 1);
					mapper.RTC_Get(seconds & 0xFF, 0);
				}

				if (mppr == "HuC3")
				{
					mapper.RTC_Get(years, 24);
					mapper.RTC_Get(days_upper, 20);
					mapper.RTC_Get(days & 0xFF, 12);
					mapper.RTC_Get(minutes_upper, 8);
					mapper.RTC_Get(remaining & 0xFF, 0);
				}
			}
			else
			{
				GB_Pntr = LibGBHawk.GB_create();

				GB_message = GetMessageGB;

				LibGBHawk.GB_setmessagecallback(GB_Pntr, GB_message);

				BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GB", "World"), "BIOS Not Found, Cannot Load");

				LibGBHawk.GB_load_bios(GB_Pntr, BIOS);

				LibGBHawk.GB_load(GB_Pntr, rom, (uint)rom.Length);
			}

			ser.Register<IVideoProvider>(this);
			ser.Register<ISoundProvider>(audio);
			ServiceProvider = ser;

			_tracer = new TraceBuffer(cpu.TraceHeader);
			ser.Register<ITraceable>(_tracer);
			ser.Register<IStatable>(new StateSerializer(SyncState));
            ser.Register<IDisassemblable>(_disassembler);
			SetupMemoryDomains();
			cpu.SetCallbacks(ReadMemory, PeekMemory, PeekMemory, WriteMemory);
			HardReset();

			_scanlineCallback = null;
		}

		public IntPtr GB_Pntr { get; set; } = IntPtr.Zero;

		public ulong TotalExecutedCycles => Settings.cycle_return_setting == GBHawkSettings.Cycle_Return.CPU ? cpu.TotalExecutedCycles : CycleCount;

		public bool IsCGBMode() => Is_GBC;

		public bool IsCGBDMGMode() => is_GB_in_GBC;

		public IntPtr GetBGPalRam()
		{
			IntPtr temp_ptr = IntPtr.Zero;

			if (GB_Pntr != IntPtr.Zero)
			{
				if (Is_GBC)
				{
					temp_ptr = LibGBCHawk.GBC_get_ppu_pntrs(GB_Pntr, 1);
				}
				else
				{
					temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 1);
				}
			}

			return temp_ptr;
		}

		public IntPtr GetSPRPalRam()
		{
			IntPtr temp_ptr = IntPtr.Zero;

			if (GB_Pntr != IntPtr.Zero)
			{
				if (Is_GBC)
				{
					temp_ptr = LibGBCHawk.GBC_get_ppu_pntrs(GB_Pntr, 1);
				}
				else
				{
					temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 1);
				}
			}

			return temp_ptr;
		}

		public IntPtr GetOAM()
		{
			IntPtr temp_ptr = IntPtr.Zero;
			
			if (GB_Pntr != IntPtr.Zero)
			{
				if (Is_GBC)
				{
					temp_ptr = LibGBCHawk.GBC_get_ppu_pntrs(GB_Pntr, 1);
				}
				else
				{
					temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 1);
				}
			}

			return temp_ptr;
		}

		public IntPtr GetVRAM()
		{
			IntPtr temp_ptr = IntPtr.Zero;

			if (GB_Pntr != IntPtr.Zero)
			{
				if (Is_GBC)
				{
					temp_ptr = LibGBCHawk.GBC_get_ppu_pntrs(GB_Pntr, 2);
				}
				else
				{
					temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 2);
				}
			}

			return temp_ptr;
		}

		/// <param name="image">The image data</param>
		/// <param name="height">How tall an image is, in pixels. Image is only valid up to that height and must be assumed to be garbage below that.</param>
		/// <param name="top_margin">The top margin of blank pixels. Just form feeds the printer a certain amount at the top.</param>
		/// <param name="bottom_margin">The bottom margin of blank pixels. Just form feeds the printer a certain amount at the bottom.</param>
		/// <param name="exposure">The darkness/intensity of the print job. What the exact values mean is somewhat subjective but 127 is the most exposed/darkest value.</param>
		public delegate void PrinterCallback(IntPtr image, byte height, byte top_margin, byte bottom_margin, byte exposure);


		public ScanlineCallback _scanlineCallback;
		public int _scanlineCallbackLine = 0;

		public void SetScanlineCallback(ScanlineCallback callback, int line)
		{
			_scanlineCallback = callback;
			_scanlineCallbackLine = line;

			if (line == -2)
			{
				_scanlineCallback(ppu.LCDC);
			}
		}

		public DisplayType Region => DisplayType.NTSC;

		private readonly GBHawkControllerDeck _controllerDeck;

		private LibGBHawk.MessageCallback GB_message;
		private void GetMessageGB(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBHawk.GB_getmessage(GB_Pntr, new_m);

			Console.WriteLine(new_m);
		}

		private LibGBCHawk.MessageCallback GBC_message;

		private void GetMessageGBC(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBCHawk.GBC_getmessage(GB_Pntr, new_m);

			Console.WriteLine(new_m);
		}

		public void HardReset()
		{
			GB_bios_register = 0; // bios enable
			GBC_compat = true;
			double_speed = false;
			VRAM_Bank = 0;
			RAM_Bank = 1; // RAM bank always starts as 1 (even writing zero still sets 1)
			RAM_Bank_ret = 0; // return value can still be zero even though the bank itself cannot be
			delays_to_process = false;
			controller_delay_cd = 0;
			clear_counter = 0;

			Register_Reset();
			timer.Reset();
			ppu.Reset();
			audio.Reset();
			serialport.Reset();
			mapper.Reset();
			cpu.Reset();
			
			vid_buffer = new uint[VirtualWidth * VirtualHeight];
			frame_buffer = new int[VirtualWidth * VirtualHeight];

			uint startup_color = 0xFFFFFFFF;
			for (int i = 0; i < vid_buffer.Length; i++)
			{
				vid_buffer[i] = startup_color;
				frame_buffer[i] = (int)vid_buffer[i];
			}

			for (int i = 0; i < ZP_RAM.Length; i++)
			{
				ZP_RAM[i] = 0;
			}

			if (_syncSettings.GBACGB)
			{
				// on GBA, initial RAM is mostly random, choosing 0 allows for stable clear and hotswap for games that encounter
				// uninitialized RAM
				for (int i = 0; i < RAM.Length; i++)
				{
					RAM[i] = 0;//GBA_Init_RAM[i];
				}
			}
			else
			{
				for (int i = 0; i < 0x800; i++)
				{
					if ((i & 0xF) < 8)
					{
						RAM[i] = 0xFF;
						RAM[i + 0x1000] = 0xFF;
						RAM[i + 0x2000] = 0xFF;
						RAM[i + 0x3000] = 0xFF;
						RAM[i + 0x4000] = 0xFF;
						RAM[i + 0x5000] = 0xFF;
						RAM[i + 0x6000] = 0xFF;
						RAM[i + 0x7000] = 0xFF;

						RAM[i + 0x800] = 0;
						RAM[i + 0x1800] = 0;
						RAM[i + 0x2800] = 0;
						RAM[i + 0x3800] = 0;
						RAM[i + 0x4800] = 0;
						RAM[i + 0x5800] = 0;
						RAM[i + 0x6800] = 0;
						RAM[i + 0x7800] = 0;
					}
					else
					{
						RAM[i] = 0;
						RAM[i + 0x1000] = 0;
						RAM[i + 0x2000] = 0;
						RAM[i + 0x3000] = 0;
						RAM[i + 0x4000] = 0;
						RAM[i + 0x5000] = 0;
						RAM[i + 0x6000] = 0;
						RAM[i + 0x7000] = 0;

						RAM[i + 0x800] = 0xFF;
						RAM[i + 0x1800] = 0xFF;
						RAM[i + 0x2800] = 0xFF;
						RAM[i + 0x3800] = 0xFF;
						RAM[i + 0x4800] = 0xFF;
						RAM[i + 0x5800] = 0xFF;
						RAM[i + 0x6800] = 0xFF;
						RAM[i + 0x7800] = 0xFF;
					}
				}

				// some bytes are like this is Gambatte, hardware anomoly? Is it consistent across versions?
				/*
				for (int i = 0; i < 16; i++)
				{
					RAM[0xE02 + (16 * i)] = 0;
					RAM[0xE0A + (16 * i)] = 0xFF;

					RAM[0x1E02 + (16 * i)] = 0;
					RAM[0x1E0A + (16 * i)] = 0xFF;

					RAM[0x2E02 + (16 * i)] = 0;
					RAM[0x2E0A + (16 * i)] = 0xFF;

					RAM[0x3E02 + (16 * i)] = 0;
					RAM[0x3E0A + (16 * i)] = 0xFF;

					RAM[0x4E02 + (16 * i)] = 0;
					RAM[0x4E0A + (16 * i)] = 0xFF;

					RAM[0x5E02 + (16 * i)] = 0;
					RAM[0x5E0A + (16 * i)] = 0xFF;

					RAM[0x6E02 + (16 * i)] = 0;
					RAM[0x6E0A + (16 * i)] = 0xFF;

					RAM[0x7E02 + (16 * i)] = 0;
					RAM[0x7E0A + (16 * i)] = 0xFF;
				}
				*/
			}
		}
	}
}
