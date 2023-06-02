using System;
using System.Text;
using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Common.ReflectionExtensions;

/*
	GBA Emulator
	NOTES: 
	RAM disabling not implemented, check if used by any games

	Open bus behaviour needs to be done more carefully

	EEPROM accesses only emulated at 0xDxxxxxx, check if any games use lower range
*/

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	[Core(CoreNames.GBAHawkLink, "", isReleased: true)]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBAHawkLink : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable, IRegionable,
								ISettable<GBAHawkLink.GBALinkSettings, GBAHawkLink.GBALinkSyncSettings>
	{
		public byte[] BIOS;

		public readonly byte[][] ROMS = new byte[2][];

		public uint[] ROMS_Length = new uint[2];

		public ushort controller_state_1, controller_state_2;
		public ushort Acc_X_state_1, Acc_X_state_2;
		public ushort Acc_Y_state_1, Acc_Y_state_2;
		public byte Solar_state_1, Solar_state_2;

		public byte[][] cart_RAMS = new byte[2][];
		public bool[] has_bats = new bool[2];

		int[] mappers = new int[2];

		[CoreConstructor(VSystemID.Raw.GBAL)]
		public GBAHawkLink(CoreLoadParameters<GBAHawkLink.GBALinkSettings, GBAHawkLink.GBALinkSyncSettings> lp)
		{
			if (lp.Roms.Count != 2)
				throw new InvalidOperationException("Wrong number of roms");

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

				if (rom.Length > 0x6000000)
				{
					throw new Exception("Over size ROM?");
				}
				else
				{
					ROMS_Length[i] = (uint)rom.Length;
					Buffer.BlockCopy(rom, 0, ROMS[i], 0, rom.Length);

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

				mappers[i] = Setup_Mapper(romHashMD5, romHashSHA1, i);

				if (cart_RAMS[i] != null)
				{
					// initialize SRAM to 0xFF;
					if (mappers[i] == 1)
					{
						for (int j = 0; j < cart_RAMS[i].Length; j++)
						{
							cart_RAMS[i][j] = 0xFF;
						}
					}
					// initialize EEPROM to 0xFF;
					if ((mappers[i] == 2) || (mappers[i] == 3) || (mappers[i] == 4))
					{
						for (int j = 0; j < cart_RAMS[i].Length; j++)
						{
							cart_RAMS[i][j] = 0xFF;
						}
					}
					// initialize Flash to 0;
					if (mappers[i] == 5)
					{
						for (int j = 0; j < cart_RAMS[i].Length; j++)
						{
							cart_RAMS[i][j] = 0;
						}
					}
				}
			}
			

			// Load up a BIOS and initialize the correct PPU
			BIOS = lp.Comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

			GBA_Pntr = LibGBAHawkLink.GBALink_create();

			LibGBAHawkLink.GBALink_load_bios(GBA_Pntr, BIOS);

			Console.WriteLine("Mapper: " + mappers);
			LibGBAHawkLink.GBALink_load(GBA_Pntr, ROMS[0], (uint)ROMS_Length[0], mappers[0], ROMS[1], (uint)ROMS_Length[1], mappers[1]);

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

			string cntrllr1 = GBAHawkLink_ControllerDeck.DefaultControllerName;
			string cntrllr2 = GBAHawkLink_ControllerDeck.DefaultControllerName;


			if (mappers[0] == 3)
			{
				cntrllr1 = typeof(StandardTilt).DisplayName();
			}
			else if (mappers[0] == 4)
			{
				cntrllr1 = typeof(StandardSolar).DisplayName();
			}

			if (mappers[1] == 3)
			{
				cntrllr2 = typeof(StandardTilt).DisplayName();
			}
			else if (mappers[1] == 4)
			{
				cntrllr2 = typeof(StandardSolar).DisplayName();
			}

			_controllerDeck = new(cntrllr1, cntrllr2, false);
		}

		public int Setup_Mapper(string romHashMD5, string romHashSHA1, int i)
		{
			int size_f = 0;
			
			int mppr = 0;
			has_bats[i] = false;

			// check for SRAM
			for (int j = 0; j < ROMS.Length; j += 4)
			{
				if (ROMS[i][j] == 0x53)
				{
					if ((ROMS[i][j + 1] == 0x52) && (ROMS[i][j + 2] == 0x41))
					{
						if ((ROMS[i][j + 3] == 0x4D) && (ROMS[i][j + 4] == 0x5F))
						{
							Console.WriteLine("using SRAM mapper");
							mppr = 1;
							break;
						}
					}
				}
				if (ROMS[i][j] == 0x45)
				{
					if ((ROMS[i][j + 1] == 0x45) && (ROMS[i][j + 2] == 0x50))
					{
						if ((ROMS[i][j + 3] == 0x52) && (ROMS[i][j + 4] == 0x4F) && (ROMS[i][j + 5] == 0x4D))
						{
							Console.WriteLine("using EEPROM mapper");
							mppr = 2;
							break;
						}
					}
				}
				if (ROMS[i][j] == 0x46)
				{
					if ((ROMS[i][j + 1] == 0x4C) && (ROMS[i][j + 2] == 0x41))
					{
						if ((ROMS[i][j + 3] == 0x53) && (ROMS[i][j + 4] == 0x48))
						{
							if ((ROMS[i][j + 5] == 0x5F) && (ROMS[i][j + 6] == 0x56))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = 5;
								size_f = 64;

								break;
							}
							if ((ROMS[i][j + 6] == 0x35) && (ROMS[i][j + 6] == 0x31) && (ROMS[i][j + 7] == 0x32))
							{
								Console.WriteLine("using FLASH mapper");
								mppr = 5;
								size_f = 64;

								break;
							}
							if ((ROMS[i][j + 5] == 0x31) && (ROMS[i][j + 6] == 0x4D))
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
				(romHashSHA1 == "SHA1:9B02C4BFD99CCD913A5D7EE7CF269EBC689E1FDE"))   // Higurashi no Nakukoroni (fixed header)

			{
				Console.WriteLine("using SRAM mapper");
				mppr = 1;
			}

			if (mppr == 1)
			{
				has_bats[i] = true;
				cart_RAMS[i] = new byte[0x8000];
			}
			else if (mppr == 2)
			{
				has_bats[i] = true;

				// assume 8 KB saves, use hash check to pick out 512 bytes versions
				if ((romHashSHA1 == "SHA1:947498CB1DB918D305500257E8223DEEADDF561D") || // Yoshi USA
					(romHashSHA1 == "SHA1:A3F2035CA2BDC2BC59E9E46EFBB6187705EBE3D1") || // Yoshi Japan
					(romHashSHA1 == "SHA1:045BE1369964F141009F3701839EC0A8DCCB25C1") || // Yoshi EU
					(romHashSHA1 == "SHA1:40CB751D119A49BE0CD44CF0491C93EBC8795EF0"))   // koro koro puzzle
				{
					Console.WriteLine("Using Tilt Controls");

					cart_RAMS[i] = new byte[0x200];
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

					cart_RAMS[i] = new byte[0x2000];
					mppr = 4;
				}
				else
				{
					cart_RAMS[i] = new byte[0x2000];
				}
			}
			else if (mppr == 5)
			{
				has_bats[i] = true;

				if (size_f == 64)
				{
					cart_RAMS[i] = new byte[0x10000];
				}
				else
				{
					cart_RAMS[i] = new byte[0x20000];
				}
			}

			return mppr;
		}

		public ulong TotalExecutedCycles => 0;

		public void HardReset()
		{
			LibGBAHawkLink.GBALink_Hard_Reset(GBA_Pntr);
		}

		private IntPtr GBA_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GBA_core = new byte[0x80000 * 2];

		private readonly GBAHawkLink_ControllerDeck _controllerDeck;

		private int _frame = 0;

		public DisplayType Region => DisplayType.NTSC;

		private readonly ITraceable Tracer;

		private LibGBAHawkLink.TraceCallback tracecb;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		private void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			LibGBAHawkLink.GBALink_getdisassembly(GBA_Pntr, new_d, t, Disasm_Length, Settings.Trace_Core);
			LibGBAHawkLink.GBALink_getregisterstate(GBA_Pntr, new_r, t, Reg_String_Length, Settings.Trace_Core);

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
		}
	}
}
