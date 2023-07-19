﻿using System;
using System.Text;
using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug;

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

		public ushort controller_state;
		public ushort Acc_X_state;
		public ushort Acc_Y_state;
		public byte Solar_state;

		public byte[] cart_RAM;
		public bool has_bat;
		int mapper;

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

			mapper = Setup_Mapper(romHashMD5, romHashSHA1);

			if (cart_RAM != null)
			{
				// initialize SRAM to 0xFF;
				if (mapper == 1)
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
				// initialize EEPROM to 0xFF;
				if ((mapper == 2) || (mapper == 3) || (mapper == 4))
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
					}
				}
				// initialize Flash to 0;
				if (mapper == 5)
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0;
					}
				}
			}

			// Load up a BIOS and initialize the correct PPU
			BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			GBA_Pntr = LibGBAHawk.GBA_create();

			LibGBAHawk.GBA_load_bios(GBA_Pntr, BIOS);

			Console.WriteLine("Mapper: " + mapper);
			LibGBAHawk.GBA_load(GBA_Pntr, ROM, (uint)ROM_Length, mapper);

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
				_controllerDeck = new(typeof(StandardTilt).DisplayName());
			}
			else if (mapper == 4)
			{
				_controllerDeck = new(typeof(StandardSolar).DisplayName());
			}
			else
			{
				_controllerDeck = new(GBAHawk_ControllerDeck.DefaultControllerName);
			}

			Mem_Domains.vram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 0);
			Mem_Domains.oam = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 1);
			Mem_Domains.palram = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 2);
			Mem_Domains.mmio = LibGBAHawk.GBA_get_ppu_pntrs(GBA_Pntr, 3);

			GBA_message = null;

			LibGBAHawk.GBA_setmessagecallback(GBA_Pntr, GBA_message);
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
				if (ROM[i] == 0x46)
				{
					if ((ROM[i + 1] == 0x4C) && (ROM[i + 2] == 0x41))
					{
						if ((ROM[i + 3] == 0x53) && (ROM[i + 4] == 0x48))
						{
							if ((ROM[i + 5] == 0x5F) && (ROM[i + 6] == 0x56))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = 5;
								size_f = 64;

								break;
							}
							if ((ROM[i + 5] == 0x35) && (ROM[i + 6] == 0x31) && (ROM[i + 7] == 0x32))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = 5;
								size_f = 64;

								break;
							}
							if ((ROM[i + 5] == 0x31) && (ROM[i + 6] == 0x4D))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = 5;
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
				(romHashSHA1 == "SHA1:9B02C4BFD99CCD913A5D7EE7CF269EBC689E1FDE"))   // Higurashi no Nakukoroni (fixed header)

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
				has_bat = true;

				// assume 8 KB saves, use hash check to pick out 512 bytes versions
				if ((romHashSHA1 == "SHA1:947498CB1DB918D305500257E8223DEEADDF561D") || // Yoshi USA
					(romHashSHA1 == "SHA1:A3F2035CA2BDC2BC59E9E46EFBB6187705EBE3D1") || // Yoshi Japan
					(romHashSHA1 == "SHA1:045BE1369964F141009F3701839EC0A8DCCB25C1") || // Yoshi EU
					(romHashSHA1 == "SHA1:40CB751D119A49BE0CD44CF0491C93EBC8795EF0"))   // koro koro puzzle
				{
					Console.WriteLine("Using Tilt Controls");

					cart_RAM = new byte[0x200];
					mppr = 3;
				}
				else if ((romHashSHA1 == "SHA1:F91126CD3A1BF7BF5F770D3A70229171D0D5A6EE") || // Boktai Beta
						 (romHashSHA1 == "SHA1:64F7BF0F0560F6E94DA33B549D3206678B29F557") || // Boktai EU
						 (romHashSHA1 == "SHA1:7164326283DF46A3941EC7B6CECA889CBC40E660") || // Boktai USA
						 (romHashSHA1 == "SHA1:CD10D8ED82F4DAF4072774F70D015E39A5D32D0B") || // Boktai 2 USA
						 (romHashSHA1 == "SHA1:EEACDF5A9D3D2173A4A96689B72DC6B7AD92153C") || // Boktai 2 EU
						 (romHashSHA1 == "SHA1:54A4DCDECA2EE9A22559EB104B88586386639097") || // Boktai 2 JPN
						 (romHashSHA1 == "SHA1:1A81843C3070DECEA4CBCA20C4563541400B2437") || // Boktai 2 JPN Rev 1
						 (romHashSHA1 == "SHA1:2651C5E6875AC60ABFF734510D152166D211C87C"))   // Boktai 3
				{
					Console.WriteLine("Using Solar Sensor");

					cart_RAM = new byte[0x2000];
					mppr = 4;
				}
				else if ((romHashSHA1 == "SHA1:D3C3201F4A401B337009E667F5B001D5E12ECE83") || // Shrek 2 (USA)
						 (romHashSHA1 == "SHA1:1F28AB954789F3946E851D5A132CDA4EDB9B74DD") || // Shrek 2 (USA)
						 (romHashSHA1 == "SHA1:C433C493F54FCD18AD18B7D62F2B7D200FD9D859"))   // Shrek 2 (Input Patch)
				{
					cart_RAM = new byte[0x200];
				}
				else
				{
					cart_RAM = new byte[0x2000];
				}
			}
			else if (mppr == 5)
			{
				has_bat = true;

				if (size_f == 64)
				{
					cart_RAM = new byte[0x10000];
				}
				else
				{
					cart_RAM = new byte[0x20000];
				}
			}

			return mppr;
		}

		public ulong TotalExecutedCycles => 0;

		public void HardReset()
		{
			LibGBAHawk.GBA_Hard_Reset(GBA_Pntr);
		}

		private IntPtr GBA_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GBA_core = new byte[0xA0000];

		private readonly GBAHawk_ControllerDeck _controllerDeck;

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

		private void GetMessage()
		{
			StringBuilder new_m = new StringBuilder(200);

			LibGBAHawk.GBA_getmessage(GBA_Pntr, new_m, 200);

			Console.WriteLine(new_m);
		}
	}
}
