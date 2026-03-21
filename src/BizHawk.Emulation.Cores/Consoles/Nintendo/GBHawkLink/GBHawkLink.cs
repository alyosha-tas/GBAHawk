using System;
using System.Text;
using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Cores.Nintendo.GBHawk;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	[Core(CoreNames.GBHawkLink, "", isReleased: true)]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBHawkLink : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable, IRegionable,
								ISettable<GBHawkLink.GBLinkSettings, GBHawkLink.GBLinkSyncSettings>
	{
		public byte[][] BIOS = new byte[4][];

		public readonly byte[][] ROM = new byte[4][];

		public uint[] ROM_Length = new uint[4];

		public ushort A_controller_state, B_controller_state, C_controller_state, D_controller_state;
		public ushort A_Acc_X_state, B_Acc_X_state, C_Acc_X_state, D_Acc_X_state;
		public ushort A_Acc_Y_state, B_Acc_Y_state, C_Acc_Y_state, D_Acc_Y_state;

		public byte[][] cart_RAM = new byte[4][];
		public byte[][] cart_RAM_vbls = new byte[4][];
		public uint[] Cart_RAM_Size = new uint[4];
		public bool[] Use_MT = new bool[4];
		public bool[] has_bat = new bool[4];

		int[] mapper = new int[4];

		public int Num_ROMS = 0;
		public bool Is_OneScreenMode = false;
		public bool _cablediscosignal, _cableconnected;
		public bool _cableconnected_AC, _cableconnected_BC, _cableconnected_AB;
		public bool _cableconnected_UD, _cableconnected_LR, _cableconnected_X, _cableconnected_4x;

		public bool[] Is_GBC = new bool[4];
		public bool[] Is_GB_in_GBC = new bool[4];

		public readonly byte[] header = new byte[0x50];

		public int Actual_Width, Actual_Height;

		public bool[] Current_sync_on_vbl = new bool[4];

		private static readonly byte[] GBA_override = { 0xFF, 0x00, 0xCD, 0x03, 0x35, 0xAA, 0x31, 0x90, 0x94, 0x00, 0x00, 0x00, 0x00 };

		[CoreConstructor(VSystemID.Raw.GBL)]
		public GBHawkLink(CoreLoadParameters<GBHawkLink.GBLinkSettings, GBHawkLink.GBLinkSyncSettings> lp)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (GBLinkSettings)lp.Settings ?? new GBLinkSettings();
			SyncSettings = (GBLinkSyncSettings)lp.SyncSettings ?? new GBLinkSyncSettings();

			Is_OneScreenMode = SyncSettings.OneScreenMode;

			_cablediscosignal = false;
			_cableconnected = false;

			_cableconnected_AC = _cableconnected_BC = _cableconnected_AB = false;
			_cableconnected_UD = _cableconnected_LR = _cableconnected_X = _cableconnected_4x = false;

			Num_ROMS = lp.Roms.Count;

			if ((Num_ROMS < 2) || (Num_ROMS > 4))
			{
				throw new Exception("Invalid number of ROMs");
			}

			for (int i = 0; i < Num_ROMS; i++)
			{
				video_buffers[i] = new int[160 * 144];
			}

			if (SyncSettings.OneScreenMode)
			{
				Actual_Width = 160;
				Actual_Height = 144;
			}
			else if (Num_ROMS == 2)
			{
				Actual_Width = 160 * 2 + 2;
				Actual_Height = 144;
			}
			else
			{
				Actual_Width = 160 * 2 + 2;
				Actual_Height = 144 * 2 + 2;
			}

			_vidbuffer = new int[Actual_Width * Actual_Height];

			GBLink_Pntr = LibGBHawkLink.GBLink_create((uint)Num_ROMS);

			GBLink_message = GetMessageGBLink;

			LibGBHawkLink.GBLink_setmessagecallback(GBLink_Pntr, GBLink_message);

			string[] controller_strings = new string[4];

			Current_sync_on_vbl[0] = Settings.A_VBL_sync == true;
			Current_sync_on_vbl[1] = Settings.B_VBL_sync == true;
			Current_sync_on_vbl[2] = Settings.C_VBL_sync == true;
			Current_sync_on_vbl[3] = Settings.D_VBL_sync == true;

			for (int i = 0; i < Num_ROMS; i++)
			{
				Buffer.BlockCopy(lp.Roms[i].RomData, 0x100, header, 0, 0x50);

				if ((header[0x43] != 0x80) && (header[0x43] != 0xC0))
				{
					Is_GB_in_GBC[i] = true; // for movie files
				}

				bool patch_bios = false;
				bool Is_GBA = false;

				switch (i)
				{
					case 0:
						Is_GBC[i] = SyncSettings.A_ConsoleMode != GBLinkSyncSettings.ConsoleModeType.GB;
						Is_GBA = SyncSettings.A_GBACGB;
						if (Is_GBC[i] && Is_GBA) { patch_bios = true; }
						break;
					case 1:
						Is_GBC[i] = SyncSettings.B_ConsoleMode != GBLinkSyncSettings.ConsoleModeType.GB;
						Is_GBA = SyncSettings.B_GBACGB;
						if (Is_GBC[i] && Is_GBA) { patch_bios = true; }
						break;
					case 2: 
						Is_GBC[i] = SyncSettings.C_ConsoleMode != GBLinkSyncSettings.ConsoleModeType.GB;
						Is_GBA = SyncSettings.C_GBACGB;
						if (Is_GBC[i] && Is_GBA) { patch_bios = true; }
						break;
					case 3:
						Is_GBC[i] = SyncSettings.D_ConsoleMode != GBLinkSyncSettings.ConsoleModeType.GB;
						Is_GBA = SyncSettings.D_GBACGB;
						if (Is_GBC[i] && Is_GBA) { patch_bios = true; }
						break;
				}
				
				if (Is_GBC[i])
				{
					BIOS[i] = lp.Comm.CoreFileProvider.GetFirmwareOrThrow(new("GBC", "World"), "BIOS Not Found, Cannot Load");
				}
				else
				{
					BIOS[i] = lp.Comm.CoreFileProvider.GetFirmwareOrThrow(new("GB", "World"), "BIOS Not Found, Cannot Load");
				}

				// Here we modify the BIOS if GBA mode is set (credit to ExtraTricky)
				if (patch_bios)
				{
					for (int j = 0; j < 13; j++)
					{
						BIOS[i][j + 0xF3] = (byte)((GBA_override[j] + BIOS[i][j + 0xF3]) & 0xFF);
					}
				}

				LibGBHawkLink.GBLink_load_bios(GBLink_Pntr, BIOS[i], Is_GBC[i], Is_GBA, (uint)i);

				var romHashMD5 = MD5Checksum.ComputePrefixedHex(lp.Roms[i].RomData);
				Console.WriteLine(romHashMD5);
				var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(lp.Roms[i].RomData);
				Console.WriteLine(romHashSHA1);

				ROM[i] = new byte[lp.Roms[i].RomData.Length];

				// make sure rom passes basic sanity checks
				if (ROM[i].Length < 32 * 1024)
				{
					ROM[i] = new byte[32 * 1024];
				}

				for (int j = 0; j < ROM[i].Length; j++)
				{
					ROM[i][j] = lp.Roms[i].RomData[j];
				}

				string mppr;
				uint mppr_num;

				GBCommonFunctions.Setup_Mapper(romHashMD5, romHashSHA1, header, out mppr, out mppr_num, out has_bat[i], out Cart_RAM_Size[i]);

				if (Cart_RAM_Size[i] != 0)
				{
					cart_RAM[i] = new byte[Cart_RAM_Size[i]];
					cart_RAM_vbls[i] = new byte[Cart_RAM_Size[i]];

					Console.WriteLine("SRAM " + i + ": " + Cart_RAM_Size[i]);
				}

				controller_strings[i] = mppr is "MBC7" ? typeof(StandardTilt).DisplayName() : GBHawkControllerDeck.DefaultControllerName;

				if ((mppr == "MBC3") || (mppr == "HuC3") || (mppr == "TAMA5"))
				{
					Use_MT[i] = true;
				}

				int years = 0;
				int days = 0;
				int days_upper = 0;
				int hours = 0;
				int minutes = 0;
				int minutes_upper = 0;
				int seconds = 0;
				int remaining = 0;

				if (mppr == "MBC3")
				{
					switch (i)
					{
						case 0:
							days = (int)Math.Floor(SyncSettings.A_RTCInitialTime / 86400.0);
							remaining = SyncSettings.A_RTCInitialTime - (days * 86400);
							break;
						case 1:
							days = (int)Math.Floor(SyncSettings.B_RTCInitialTime / 86400.0);
							remaining = SyncSettings.B_RTCInitialTime - (days * 86400);
							break;
						case 2:
							days = (int)Math.Floor(SyncSettings.C_RTCInitialTime / 86400.0);
							remaining = SyncSettings.C_RTCInitialTime - (days * 86400);
							break;
						case 3:
							days = (int)Math.Floor(SyncSettings.D_RTCInitialTime / 86400.0);
							remaining = SyncSettings.D_RTCInitialTime - (days * 86400);
							break;
					}
				
					days_upper = ((days & 0x100) >> 8) | ((days & 0x200) >> 2);
					hours = (int)Math.Floor(remaining / 3600.0);
					remaining = remaining - (hours * 3600);
					minutes = (int)Math.Floor(remaining / 60.0);
					seconds = remaining - (minutes * 60);
				}

				if (mppr == "HuC3")
				{
					switch (i)
					{
						case 0:
							years = (int)Math.Floor(SyncSettings.A_RTCInitialTime / 31536000.0);
							remaining = SyncSettings.A_RTCInitialTime - (years * 31536000);
							break;
						case 1:
							years = (int)Math.Floor(SyncSettings.B_RTCInitialTime / 31536000.0);
							remaining = SyncSettings.B_RTCInitialTime - (years * 31536000);
							break;
						case 2:
							years = (int)Math.Floor(SyncSettings.C_RTCInitialTime / 31536000.0);
							remaining = SyncSettings.C_RTCInitialTime - (years * 31536000);
							break;
						case 3:
							years = (int)Math.Floor(SyncSettings.D_RTCInitialTime / 31536000.0);
							remaining = SyncSettings.D_RTCInitialTime - (years * 31536000);
							break;
					}

					days = (int)Math.Floor(remaining / 86400.0);
					days_upper = (days >> 8) & 0xF;
					remaining = remaining - (days * 86400);
					minutes = (int)Math.Floor(remaining / 60.0);
					minutes_upper = (minutes >> 8) & 0xF;
				}

				Console.WriteLine(ROM[i].Length + " " + mppr_num + " " + i);

				LibGBHawkLink.GBLink_load(GBLink_Pntr, ROM[i], (uint)ROM[i].Length, mppr_num, (uint)i);

				LibGBHawkLink.GBLink_create_SRAM(GBLink_Pntr, cart_RAM[i], Cart_RAM_Size[i], (uint)i);

				Console.WriteLine("Core: " + i + " of: " + Num_ROMS + " created.");

				if (mppr == "MBC3")
				{
					switch (i)
					{
						case 0: LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, SyncSettings.A_RTCOffset, 5, (uint)i); break;
						case 1: LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, SyncSettings.B_RTCOffset, 5, (uint)i); break;
						case 2: LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, SyncSettings.C_RTCOffset, 5, (uint)i); break;
						case 3: LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, SyncSettings.D_RTCOffset, 5, (uint)i); break;
					}
					
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, days_upper, 4, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, days & 0xFF, 3, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, hours & 0xFF, 2, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, minutes & 0xFF, 1, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, seconds & 0xFF, 0, (uint)i);
				}

				if (mppr == "HuC3")
				{
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, years, 24, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, days_upper, 20, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, days & 0xFF, 12, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, minutes_upper, 8, (uint)i);
					LibGBHawkLink.GBLink_set_rtc(GBLink_Pntr, remaining & 0xFF, 0, (uint)i);
				}

				LibGBHawkLink.GBLink_Sync_Domain_VBL(GBLink_Pntr, Current_sync_on_vbl[i], (uint)i);
			}

			HardReset();

			blip_L_0.SetRates(4194304 * 2, 44100);
			blip_R_0.SetRates(4194304 * 2, 44100);
			blip_L_1.SetRates(4194304 * 2, 44100);
			blip_R_1.SetRates(4194304 * 2, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			SetupMemoryDomains();

			Header_Length = LibGBHawkLink.GBLink_getheaderlength(GBLink_Pntr);
			Disasm_Length = LibGBHawkLink.GBLink_getdisasmlength(GBLink_Pntr);
			Reg_String_Length = LibGBHawkLink.GBLink_getregstringlength(GBLink_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibGBHawkLink.GBLink_getheader(GBLink_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			_controllerDeck = new(controller_strings, Num_ROMS);

			// default to 4x linking in 4 player mode
			if (Num_ROMS == 4) { _cableconnected_4x = true; LibGBHawkLink.GBLink_change_linking(GBLink_Pntr, true, 8); }
		}

		public ulong TotalExecutedCycles => 0;

		private LibGBHawkLink.MessageCallback GBLink_message;
		private void GetMessageGBLink(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length + 1);

			LibGBHawkLink.GBLink_getmessage(GBLink_Pntr, new_m);

			Console.WriteLine(new_m);
		}

		public void HardReset()
		{
			LibGBHawkLink.GBLink_Hard_Reset(GBLink_Pntr);
		}

		private IntPtr GBLink_Pntr { get; set; } = IntPtr.Zero;
		private byte[] GB_core = new byte[0xA0000 * 4];

		private readonly GBLink_ControllerDeck _controllerDeck;

		private int _frame = 0;

		public DisplayType Region => DisplayType.NTSC;

		private readonly ITraceable Tracer;

		private LibGBHawkLink.TraceCallback tracecb;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		private void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			uint tracer_core = (uint)Settings.TraceSet;

			LibGBHawkLink.GBLink_getdisassembly(GBLink_Pntr, new_d, t, Disasm_Length, tracer_core);
			LibGBHawkLink.GBLink_getregisterstate(GBLink_Pntr, new_r, t, Reg_String_Length, tracer_core);

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