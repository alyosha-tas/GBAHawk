using System;
using System.Text;
using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using System.Runtime.InteropServices;

/*
	GBA Emulator
	NOTES: 
	RAM disabling not implemented, check if used by any games

	Open bus behaviour needs to be done more carefully

	EEPROM accesses only emulated at 0xDxxxxxx, check if any games use lower range
*/

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	[Core(CoreNames.GBAHawk, "", isReleased: true)]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBAHawk : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable, IRegionable, IGBAGPUViewable,
								ISettable<GBAHawk.GBASettings, GBAHawk.GBASyncSettings>
	{
		public byte[] BIOS;

		public readonly byte[] ROM = new byte[0x6000000];
		public readonly byte[] header = new byte[0x50];

		public uint ROM_Length;

		public byte[] cart_RAM;
		public bool has_bat;

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

				if (romHashSHA1 != "SHA1:5F989B9A4017F16A431F76FD78A95E9799AA8FCA") // GBA Suite Memory test
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

			int mppr = Setup_Mapper(romHashMD5, romHashSHA1);

			if (cart_RAM != null)
			{
				// initialize SRAM to 0xFF;
				if (mppr == 1)
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
				// initialize EEPROM to 0xFF;
				if (mppr == 2)
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
			}

			// Load up a BIOS and initialize the correct PPU
			BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			GBA_Pntr = LibGBAHawk.GBA_create();

			LibGBAHawk.GBA_load_bios(GBA_Pntr, BIOS);

			Console.WriteLine("Mapper: " + mppr);
			LibGBAHawk.GBA_load(GBA_Pntr, ROM, (uint)ROM_Length, mppr);
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

			current_controller = GBAController;

			Mem_Domains.vram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 0);
			Mem_Domains.oam = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 1);
			Mem_Domains.palram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 2);
			Mem_Domains.mmio = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 3);

			GBA_message = null;

			LibGBAHawk.GBA_setmessagecallback(GBA_Pntr, GBA_message);
		}

		public int Setup_Mapper(string romHashMD5, string romHashSHA1)
		{
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
							mppr = 1;
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
							mppr = 2;
							break;
						}
					}
				}
				// TODO: Implmenet other save types
			}

			// hash checks for individual games / homebrew / test roms
			if ((romHashSHA1 == "SHA1:C67E0A5E26EA5EBA2BC11C99D003027A96E44060") || // Aging cart test
				(romHashSHA1 == "SHA1:AC6D8FD4A1FB5234A889EE092CBE7774DAC21F0E")) // VRAM access test
			{
				Console.WriteLine("using SRAM mapper");
				mppr = 1;
			}

			if (mppr == 1)
			{
				has_bat = true;
				cart_RAM = new byte[0x8000];
			}
			else if (mppr == 2)
			{
				// assume 8 KB saves, use hash check to pick out 512 bytes versions
				has_bat = true;
				cart_RAM = new byte[0x2000];
			}

			return mppr;
		}

		public ulong TotalExecutedCycles => 0;

		public void HardReset()
		{
			LibGBAHawk.GBA_Hard_Reset(GBA_Pntr);
		}

		private IntPtr GBA_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GBA_core = new byte[0x70000];

		// Machine resources
		private IController _controller = NullController.Instance;

		private readonly ControllerDefinition current_controller = null;

		private int _frame = 0;

		public DisplayType Region => DisplayType.NTSC;

		private readonly ITraceable Tracer;

		private LibGBAHawk.TraceCallback tracecb;

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

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(74), registerInfo: new_r.ToString()));
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
			Mem_Domains.vram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 0);
			Mem_Domains.oam = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 1);
			Mem_Domains.palram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 2);
			Mem_Domains.mmio = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 3);

			return Mem_Domains;
		}

		private LibGBAHawk.MessageCallback GBA_message;

		private void GetMessage()
		{
			StringBuilder new_m = new StringBuilder(200);

			LibGBAHawk.GBA_getmessage(GBA_Pntr, new_m, 200);

			Console.WriteLine(new_m);
		}
	}
}
