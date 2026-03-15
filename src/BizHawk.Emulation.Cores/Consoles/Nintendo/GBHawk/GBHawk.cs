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
		public bool Is_GB_in_GBC;

		public byte[] BIOS;
		public readonly byte[] ROM;
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
				Is_GB_in_GBC = true; // for movie files
			}

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			ROM = new byte[rom.Length];

			// make sure rom passes basic sanity checks
			if (rom.Length < 32*1024)
			{
				ROM = new byte[32 * 1024];
			}

			for (int i = 0; i < rom.Length; i++)
			{
				ROM[i] = rom[i];
			}

			string mppr;
			uint mppr_num;

			GBCommonFunctions.Setup_Mapper(romHashMD5, romHashSHA1, header, out mppr, out mppr_num, out has_bat, out Cart_RAM_Size);

			if (Cart_RAM_Size != 0)
			{
				cart_RAM = new byte[Cart_RAM_Size];
				cart_RAM_vbls = new byte[Cart_RAM_Size];

				Console.Write("SRAM: "); Console.WriteLine(Cart_RAM_Size);
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
				BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBC", "World"), "BIOS Not Found, Cannot Load");
			}
			else
			{
				BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GB", "World"), "BIOS Not Found, Cannot Load");
			}

			// Here we modify the BIOS if GBA mode is set (credit to ExtraTricky)
			if (SyncSettings.GBACGB && Is_GBC)
			{
				for (int i = 0; i < 13; i++)
				{
					BIOS[i + 0xF3] = (byte)((GBA_override[i] + BIOS[i + 0xF3]) & 0xFF);
				}
			}

			GB_Pntr = LibGBHawk.GB_create();

			GB_message = GetMessageGB;

			LibGBHawk.GB_setmessagecallback(GB_Pntr, GB_message);

			LibGBHawk.GB_load_bios(GB_Pntr, BIOS, Is_GBC, SyncSettings.GBACGB);

			LibGBHawk.GB_load(GB_Pntr, rom, (uint)rom.Length, mppr_num);

			if (mppr == "MBC3")
			{
				LibGBHawk.GB_set_rtc(GB_Pntr, SyncSettings.RTCOffset, 5);
				LibGBHawk.GB_set_rtc(GB_Pntr, days_upper, 4);
				LibGBHawk.GB_set_rtc(GB_Pntr, days & 0xFF, 3);
				LibGBHawk.GB_set_rtc(GB_Pntr, hours & 0xFF, 2);
				LibGBHawk.GB_set_rtc(GB_Pntr, minutes & 0xFF, 1);
				LibGBHawk.GB_set_rtc(GB_Pntr, seconds & 0xFF, 0);
			}

			if (mppr == "HuC3")
			{
				LibGBHawk.GB_set_rtc(GB_Pntr, years, 24);
				LibGBHawk.GB_set_rtc(GB_Pntr, days_upper, 20);
				LibGBHawk.GB_set_rtc(GB_Pntr, days & 0xFF, 12);
				LibGBHawk.GB_set_rtc(GB_Pntr, minutes_upper, 8);
				LibGBHawk.GB_set_rtc(GB_Pntr, remaining & 0xFF, 0);
			}

			blip_L.SetRates(4194304 * 2, 44100);
			blip_R.SetRates(4194304 * 2, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			SetupMemoryDomains();

			Header_Length = LibGBHawk.GB_getheaderlength(GB_Pntr);
			Disasm_Length = LibGBHawk.GB_getdisasmlength(GB_Pntr);
			Reg_String_Length = LibGBHawk.GB_getregstringlength(GB_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibGBHawk.GB_getheader(GB_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			LibGBHawk.GB_Sync_Domain_VBL(GB_Pntr, Settings.VBL_sync);
		}

		public IntPtr GB_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GB_core = new byte[0x80000];

		public ulong TotalExecutedCycles => 0;//Settings.cycle_return_setting == GBHawkSettings.Cycle_Return.CPU ? 0 : 0;

		public bool IsCGBMode() => Is_GBC;

		public bool IsCGBDMGMode() => Is_GB_in_GBC;

		private int _frame = 0;

		public IntPtr GetBGPalRam()
		{
			IntPtr temp_ptr = IntPtr.Zero;

			if (GB_Pntr != IntPtr.Zero)
			{
				temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 0);
			}

			return temp_ptr;
		}

		public IntPtr GetSPRPalRam()
		{
			IntPtr temp_ptr = IntPtr.Zero;

			if (GB_Pntr != IntPtr.Zero)
			{
				temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 1);
			}

			return temp_ptr;
		}

		public IntPtr GetOAM()
		{
			IntPtr temp_ptr = IntPtr.Zero;
			
			if (GB_Pntr != IntPtr.Zero)
			{
				temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 2);
			}

			return temp_ptr;
		}

		public IntPtr GetVRAM()
		{
			IntPtr temp_ptr = IntPtr.Zero;

			if (GB_Pntr != IntPtr.Zero)
			{
				temp_ptr = LibGBHawk.GB_get_ppu_pntrs(GB_Pntr, 3);
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
				_scanlineCallback(0);
			}
			else
			{
				LibGBHawk.GB_setscanlinecallback(GB_Pntr, _scanlineCallback, _scanlineCallbackLine);
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

		public void HardReset()
		{
			LibGBHawk.GB_Hard_Reset(GB_Pntr);
		}

		public readonly ITraceable Tracer;

		public LibGBHawk.TraceCallback tracecb;

		// these will be constant values assigned during core construction
		public int Header_Length;
		public readonly int Disasm_Length;
		public readonly int Reg_String_Length;

		public void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			LibGBHawk.GB_getdisassembly(GB_Pntr, new_d, t, Disasm_Length);
			LibGBHawk.GB_getregisterstate(GB_Pntr, new_r, t, Reg_String_Length);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(50), registerInfo: new_r.ToString()));
		}
	}
}
