using System;
using System.Runtime.InteropServices;
using System.Text;
using BizHawk.Emulation.Common;

using BizHawk.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Cores.Nintendo.SNES.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SNESHawk
{
	[Core(CoreNames.SNESHawk, isReleased: false)]
	public partial class SNESHawk : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable,
								ISettable<SNESHawk.SNESHawkSettings, SNESHawk.SNESHawkSyncSettings>
	{
		public byte[] IPL;

		public byte[] GamePack;
		public readonly byte[] Header = new byte[0x40];

		public uint ROM_Length;

		public ushort controller_state;

		public byte[] cart_RAM;
		public bool has_bat;

		// following header format
		// 0 = Lo ROM
		// 1 = Hi ROM
		// 5 = Ex Hi ROM
		int mapping_type = 0;

		[CoreConstructor(VSystemID.Raw.SNES)]
		public SNESHawk(CoreComm comm, GameInfo game, byte[] rom, SNESHawk.SNESHawkSettings settings, SNESHawk.SNESHawkSyncSettings syncSettings, bool subframe = false)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (SNESHawkSettings)settings ?? new SNESHawkSettings();
			SyncSettings = (SNESHawkSyncSettings)syncSettings ?? new SNESHawkSyncSettings();

			LeftController = SyncSettings.LeftController;
			RightController = SyncSettings.RightController;

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			// do not accept headered ROMs
			if ((rom.Length % 1024) != 0)
			{
				throw new Exception("Copier Headered ROMs not allowed.");
			}

			// undersized ROM
			if (rom.Length < 0x8000)
			{
				throw new Exception("undersized ROM, invalid.");
			}

			// determine if ROM length is a power of 2, if not mirror the latter contents to get a power of 2
			int cur_length = 0x8000;
			int pow_2_length = 0x8000;

			while (cur_length <= rom.Length)
			{
				pow_2_length = cur_length;

				cur_length *= 2;
			}

			if (pow_2_length != rom.Length)
			{
				GamePack = new byte[pow_2_length * 2];

				for (int i = 0; i < pow_2_length; i++)
				{
					GamePack[i] = rom[i];
				}

				int j = 0;

				int mirror_lim = rom.Length - pow_2_length;

				while (j < pow_2_length)
				{
					for (int k = 0; k < mirror_lim; k++)
					{
						GamePack[pow_2_length + j] = rom[pow_2_length + k];
					}

					j++;
				}
			}
			else
			{
				GamePack = new byte[rom.Length];

				for (int i = 0; i < rom.Length; i++)
				{
					GamePack[i] = rom[i];
				}
			}

			ROM_Length = (uint)GamePack.Length;

			byte[] checksum_check = new byte[GamePack.Length];

			int checksum = 0;
			int proposed_checksum = 0;

			bool header_found = false;

			// now we have a ROM we can guess the header locations and get a valid checksum for it
			// first try lo rom
			for (int i = 0; i < GamePack.Length; i++)
			{
				if ((i < 0x7FDC) || (i >= 0x7FE0))
				{
					checksum_check[i] = GamePack[i];
				}
			}

			checksum_check[0x7FDC] = 0xFF;
			checksum_check[0x7FDD] = 0xFF;

			for (int i = 0; i < GamePack.Length; i++)
			{
				checksum += checksum_check[i];
			}

			checksum &= 0xFFFF;

			proposed_checksum = GamePack[0x7FDF] << 8;
			proposed_checksum |= GamePack[0x7FDE];

			//Console.WriteLine("rom check " + rom.Length + " " + GamePack.Length);

			//Console.WriteLine("here check " + proposed_checksum + " " + checksum);

			if (proposed_checksum == checksum)
			{
				// check the compliment
				checksum ^= 0xFFFF;

				proposed_checksum = GamePack[0x7FDD] << 8;
				proposed_checksum |= GamePack[0x7FDC];

				if (proposed_checksum == checksum)
				{
					// suspect Lo ROM, check if header indeed gives this case
					if ((GamePack[0x7FD5] & 0xF) == 0)
					{
						header_found = true;
						mapping_type = 0;

						Console.WriteLine("Lo ROM detected via header.");

						for (int i = 0; i < 0x40; i++)
						{
							Header[i] = GamePack[0x7FC0 + i];
						}
					}
				}
			}

			// if the ROM is too small and we found no header, don't load
			if ((rom.Length < 0x10000) && !header_found)
			{
				throw new Exception("could not locate header, unable to load");
			}

			if (!header_found)
			{
				// redo for Hi ROM
				for (int i = 0; i < GamePack.Length; i++)
				{
					if ((i < 0xFFDC) || (i >= 0xFFE0))
					{
						checksum_check[i] = rom[i];
					}
				}

				checksum_check[0xFFDC] = 0xFF;
				checksum_check[0xFFDD] = 0xFF;

				checksum_check[0xFFDE] = 0;
				checksum_check[0xFFDF] = 0;

				for (int i = 0; i < GamePack.Length; i++)
				{
					checksum += checksum_check[i];
				}

				checksum &= 0xFFFF;

				proposed_checksum = GamePack[0xFFDF] << 8;
				proposed_checksum |= GamePack[0xFFDE];

				if (proposed_checksum == checksum)
				{
					// check the compliment
					checksum ^= 0xFFFF;

					proposed_checksum = GamePack[0xFFDD] << 8;
					proposed_checksum |= GamePack[0xFFDC];

					if (proposed_checksum == checksum)
					{
						// suspect Hi ROM, check if header indeed gives this case
						if ((GamePack[0xFFD5] & 0xF) == 1)
						{
							header_found = true;
							mapping_type = 1;

							Console.WriteLine("Hi ROM detected via header.");

							for (int i = 0; i < 0x40; i++)
							{
								Header[i] = GamePack[0xFFC0 + i];
							}
						}
					}
				}
			}

			// if the ROM is too small and we found no header, don't load
			if ((rom.Length < 0x410000) && !header_found)
			{
				throw new Exception("could not locate header, unable to load");
			}

			if (!header_found)
			{
				// redo for Ex Hi ROM
				for (int i = 0; i < GamePack.Length; i++)
				{
					if ((i < 0x40FFDC) || (i >= 0x40FFE0))
					{
						checksum_check[i] = rom[i];
					}
				}

				checksum_check[0x40FFDC] = 0xFF;
				checksum_check[0x40FFDD] = 0xFF;

				checksum_check[0x40FFDE] = 0;
				checksum_check[0x40FFDF] = 0;

				for (int i = 0; i < GamePack.Length; i++)
				{
					checksum += checksum_check[i];
				}

				checksum &= 0xFFFF;

				proposed_checksum = GamePack[0x40FFDF] << 8;
				proposed_checksum |= GamePack[0x40FFDE];

				if (proposed_checksum == checksum)
				{
					// check the compliment
					checksum ^= 0xFFFF;

					proposed_checksum = GamePack[0x40FFDD] << 8;
					proposed_checksum |= GamePack[0x40FFDC];

					if (proposed_checksum == checksum)
					{
						// suspect Hi ROM, check if header indeed gives this case
						if ((GamePack[0x40FFD5] & 0xF) == 5)
						{
							header_found = true;
							mapping_type = 5;

							Console.WriteLine("Ex Hi ROM detected via header.");

							for (int i = 0; i < 0x40; i++)
							{
								Header[i] = GamePack[0x40FFC0 + i];
							}
						}
					}
				}
			}

			if (!header_found)
			{
				throw new Exception("could not locate header, unable to load");
			}

			// Use header to choose SRAM size


			// Load up the APU IPL, necessary to function
			IPL = comm.CoreFileProvider.GetFirmwareOrThrow(new("SNES", "IPL"), "IPL Not Found, Cannot Load");

			SNES_Pntr = LibSNESHawk.SNES_create();

			LibSNESHawk.SNES_load_ipl(SNES_Pntr, IPL);

			SNES_message = GetMessage;

			LibSNESHawk.SNES_setmessagecallback(SNES_Pntr, SNES_message);

			LibSNESHawk.SNES_load(SNES_Pntr, GamePack, (uint)GamePack.Length, Header, SyncSettings.APU_Freq, SyncSettings.PPU_H_Pos, SyncSettings.PPU_V_Pos, SyncSettings.DRAM_Refresh_Cycle);

			if (cart_RAM != null) { LibSNESHawk.SNES_create_SRAM(SNES_Pntr, cart_RAM, (uint)cart_RAM.Length); }

			blip_buff.SetRates(1789773, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			SetupMemoryDomains();

			Header_Length = LibSNESHawk.SNES_getheaderlength(SNES_Pntr);
			Disasm_Length = LibSNESHawk.SNES_getdisasmlength(SNES_Pntr);
			Reg_String_Length = LibSNESHawk.SNES_getregstringlength(SNES_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibSNESHawk.SNES_getheader(SNES_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			Mem_Domains.vram = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 0);
			Mem_Domains.oam = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 1);
			Mem_Domains.palram = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 2);
			Mem_Domains.mmio = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 3);

			ResetControllerDefinition(subframe);

			SNES_Controller_Read = Read_Controller;

			LibSNESHawk.SNES_setcontrollercallback(SNES_Pntr, SNES_Controller_Read);

			SNES_Controller_Strobe = Strobe_Controller;

			LibSNESHawk.SNES_setstrobecallback(SNES_Pntr, SNES_Controller_Strobe);

			SNES_InputPoll = Send_Input_Callback;

			LibSNESHawk.SNES_setinputpollcallback(SNES_Pntr, SNES_InputPoll);
		}

		public SNESHawkSyncSettings.ControllerType LeftController;
		public SNESHawkSyncSettings.ControllerType RightController;

		private SNES_ControllerDeck ControllerDeck;

		public ControllerDefinition ControllerDefinition { get; private set; }

		internal void ResetControllerDefinition(bool subframe)
		{
			ControllerDefinition = null;

			ControllerDeck = Controller_Instantiate(subframe);
			ControllerDefinition = ControllerDeck.Definition;
		}

		public SNES_ControllerDeck Controller_Instantiate(bool subframe)
		{
			string left = SNES_ControllerDeck.DefaultControllerName;
			string right = SNES_ControllerDeck.DefaultControllerName;

			if (LeftController == SNESHawkSyncSettings.ControllerType.SNES)
			{
				left = typeof(SNESController).DisplayName();
			}
			else
			{
				left = typeof(UnpluggedSNES).DisplayName();
			}

			if (RightController == SNESHawkSyncSettings.ControllerType.SNES)
			{
				right = typeof(SNESController).DisplayName();
			}
			else
			{
				right = typeof(UnpluggedSNES).DisplayName();
			}


			SNES_ControllerDeck ret = new SNES_ControllerDeck(left, right, subframe);

			return ret;
		}

		public LibSNESHawk.ControllerReadCallback SNES_Controller_Read;

		public byte Read_Controller(bool addr_4016)
		{
			return addr_4016 ? ControllerDeck.ReadPort1(Controller) : ControllerDeck.ReadPort2(Controller);
		}

		public LibSNESHawk.ControllerStrobeCallback SNES_Controller_Strobe;

		public void Strobe_Controller(byte latched_4016, byte new_val)
		{
			var si = new StrobeInfo(latched_4016, new_val);
			ControllerDeck.Strobe(si, Controller);
		}

		public int Build_Header(string romHashMD5, string romHashSHA1)
		{
			int mppr = 0;
			has_bat = false;

			return mppr;
		}

		public ulong TotalExecutedCycles => 0;

		public void HardReset()
		{
			LibSNESHawk.SNES_Hard_Reset(SNES_Pntr);
		}

		public void SoftReset()
		{
			LibSNESHawk.SNES_Soft_Reset(SNES_Pntr);
		}

		public IntPtr SNES_Pntr { get; set; } = IntPtr.Zero;
		public byte[] SNES_core = new byte[0xA0000];

		private int _frame = 0;

		public DisplayType Region => DisplayType.NTSC;

		public readonly ITraceable Tracer;

		public LibSNESHawk.TraceCallback tracecb;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		public void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			LibSNESHawk.SNES_getdisassembly(SNES_Pntr, new_d, t, Disasm_Length);
			LibSNESHawk.SNES_getregisterstate(SNES_Pntr, new_r, t, Reg_String_Length);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(40), registerInfo: new_r.ToString()));
		}

		// NES PPU Viewer
		public Action _NTVCallback;
		public int _NTVCallbackLine = 0;

		public void SetNTVCallback(Action callback, int line)
		{
			_NTVCallback = callback;
			_NTVCallbackLine = line;

			LibSNESHawk.SNES_setntvcallback(SNES_Pntr, _NTVCallback, _NTVCallbackLine);
		}

		public Action _PPUViewCallback;
		public int _PPUViewCallbackLine = 0;

		public void SetPPUViewCallback(Action callback, int line)
		{
			_PPUViewCallback = callback;
			_PPUViewCallbackLine = line;

			LibSNESHawk.SNES_setppucallback(SNES_Pntr, _PPUViewCallback, _PPUViewCallbackLine);
		}

		SNESGPUMemoryAreas Mem_Domains = new SNESGPUMemoryAreas();

		public SNESGPUMemoryAreas GetMemoryAreas()
		{
			Mem_Domains.vram = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 0);
			Mem_Domains.oam = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 1);
			Mem_Domains.palram = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 2);
			Mem_Domains.mmio = LibSNESHawk.SNES_get_ppu_pntrs(SNES_Pntr, 3);

			return Mem_Domains;
		}

		private LibSNESHawk.MessageCallback SNES_message;

		private void GetMessage(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length+1);

			LibSNESHawk.SNES_getmessage(SNES_Pntr, new_m);

			Console.WriteLine(new_m);
		}

		private LibSNESHawk.InputPollCallback SNES_InputPoll;

		public IInputCallbackSystem InputCallbacksSystem => InputCallbacks;

		public void Send_Input_Callback()
		{
			InputCallbacks.Call();
		}
	}
}
