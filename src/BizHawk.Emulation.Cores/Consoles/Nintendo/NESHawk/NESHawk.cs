using System;
using System.Runtime.InteropServices;
using System.Text;
using BizHawk.Emulation.Common;

using BizHawk.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Cores.Nintendo.NES.Common;

namespace BizHawk.Emulation.Cores.Nintendo.NESHawk
{
	[Core(CoreNames.NESHawk2, "", isReleased: false)]
	public partial class NESHawk : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable,
								ISettable<NESHawk.NESHawkSettings, NESHawk.NESHawkSyncSettings>
	{
		public byte[] GamePack;
		public readonly byte[] Header = new byte[0x10];

		public uint ROM_Length;
		public uint CHR_ROM_Length;

		public ushort controller_state;

		public int[] Compiled_Palette = new int[64 * 8];

		public byte[] cart_RAM;
		public bool has_bat;
		int mapper;

		bool vram_32;

		[CoreConstructor(VSystemID.Raw.NES)]
		public NESHawk(CoreComm comm, GameInfo game, byte[] rom, NESHawk.NESHawkSettings settings, NESHawk.NESHawkSyncSettings syncSettings, bool subframe = false)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (NESHawkSettings)settings ?? new NESHawkSettings();
			SyncSettings = (NESHawkSyncSettings)syncSettings ?? new NESHawkSyncSettings();

			LeftController = SyncSettings.LeftController;
			RightController = SyncSettings.RightController;

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			// only 16 byte header size supported
			if ((rom.Length & 0xFF) == 0x0)
			{
				// can only load if the game is in the db
				Build_Header(romHashMD5, romHashSHA1);

				GamePack = new byte[rom.Length];
				Buffer.BlockCopy(rom, 0, GamePack, 0, rom.Length);
			}
			else if ((rom.Length & 0xFF) == 0x10)
			{
				Console.WriteLine("ines header detected");

				if (rom.Length > 16)
				{
					GamePack = new byte[rom.Length - 0x10];
				}
				else
				{
					throw new Exception("ROM too small");
				}

				Buffer.BlockCopy(rom, 0x10, GamePack, 0, rom.Length - 0x10);
				Buffer.BlockCopy(rom, 0, Header, 0, 0x10);
			}
			else
			{
				throw new Exception("Header size not supported");
			}

			has_bat = ((Header[6] & 0x04) == 0x04);

			// now we have a header and rom file to send to the core
			// still need to deal with save ram
			if (Header[8] != 0)
			{
				cart_RAM = new byte[(int)(Header[8])*0x2000];
			}

			if (cart_RAM != null)
			{
				for (int i = 0; i < cart_RAM.Length; i++)
				{
					cart_RAM[i] = 0xFF;
				}
			}

			ROM_Length = (uint)Header[4] * 0x4000;

			CHR_ROM_Length = (uint)Header[5] * 0x2000;

			if (GamePack.Length != (CHR_ROM_Length + ROM_Length))
			{
				throw new Exception("Mismatch between file length and header info.");
			}

			NES_Pntr = LibNESHawk.NES_create();

			NES_message = GetMessage;

			LibNESHawk.NES_setmessagecallback(NES_Pntr, NES_message);

			mapper = (Header[6] >> 4);

			mapper |= (Header[7] & 0xF0);

			Console.WriteLine("Mapper: (ines) " + mapper);

			if (mapper == 9)
			{
				if (ROM_Length != 128 * 1024)
				{
					throw new Exception("Unsupported PRG ROM Size");
				}

				if (CHR_ROM_Length != 128 * 1024)
				{
					throw new Exception("Unsupported CHR ROM Size");
				}
			}

			bool mmc3_irq = SyncSettings.MMC3_IRQ_Type == NESHawkSyncSettings.MMC3IRQType.Old;
			bool bus_conflicts = SyncSettings.Mapper_Bus_Conflicts == true;
			bool apu_test_regs = SyncSettings.Use_APU_Test_Regs == true;
			bool cpu_zero = SyncSettings.CPU_Zero_Reset == true;

			LibNESHawk.NES_load(NES_Pntr, GamePack, (uint)GamePack.Length, Header, mmc3_irq, bus_conflicts, apu_test_regs, cpu_zero);

			if (cart_RAM != null) { LibNESHawk.NES_create_SRAM(NES_Pntr, cart_RAM, (uint)cart_RAM.Length); }

			// set up initial palette
			NESCommonFunctions.SetPalette((byte[,])Palettes.QuickNESPalette.Clone(), Compiled_Palette);
			LibNESHawk.NES_load_Palette(NES_Pntr, Compiled_Palette);

			blip_buff.SetRates(1789773, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			if (mapper == 30)
			{
				vram_32 = true;
			}
			else
			{
				vram_32 = false;
			}

			SetupMemoryDomains();

			Header_Length = LibNESHawk.NES_getheaderlength(NES_Pntr);
			Disasm_Length = LibNESHawk.NES_getdisasmlength(NES_Pntr);
			Reg_String_Length = LibNESHawk.NES_getregstringlength(NES_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibNESHawk.NES_getheader(NES_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			Mem_Domains.vram = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 0);
			Mem_Domains.oam = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 1);
			Mem_Domains.palram = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 2);
			Mem_Domains.mmio = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 3);

			ResetControllerDefinition(subframe);

			NES_Controller_Read = Read_Controller;

			LibNESHawk.NES_setcontrollercallback(NES_Pntr, NES_Controller_Read);

			NES_Controller_Strobe = Strobe_Controller;

			LibNESHawk.NES_setstrobecallback(NES_Pntr, NES_Controller_Strobe);

			NES_InputPoll = Send_Input_Callback;

			LibNESHawk.NES_setinputpollcallback(NES_Pntr, NES_InputPoll);
		}

		public NESHawkSyncSettings.ControllerType LeftController;
		public NESHawkSyncSettings.ControllerType RightController;

		private NES_ControllerDeck ControllerDeck;

		public ControllerDefinition ControllerDefinition { get; private set; }

		internal void ResetControllerDefinition(bool subframe)
		{
			ControllerDefinition = null;

			ControllerDeck = Controller_Instantiate(subframe);
			ControllerDefinition = ControllerDeck.Definition;
		}

		public NES_ControllerDeck Controller_Instantiate(bool subframe)
		{
			string left = NES_ControllerDeck.DefaultControllerName;
			string right = NES_ControllerDeck.DefaultControllerName;

			if (LeftController == NESHawkSyncSettings.ControllerType.Standard)
			{
				left = typeof(NESController).DisplayName();
			}
			else if (LeftController == NESHawkSyncSettings.ControllerType.SNES)
			{
				left = typeof(SNESController).DisplayName();
			}
			else if (LeftController == NESHawkSyncSettings.ControllerType.FourScore)
			{
				left = typeof(FourScore).DisplayName();
			}
			else
			{
				left = typeof(UnpluggedNES).DisplayName();
			}

			if (RightController == NESHawkSyncSettings.ControllerType.Standard)
			{
				right = typeof(NESController).DisplayName();
			}
			else if (RightController == NESHawkSyncSettings.ControllerType.SNES)
			{
				right = typeof(SNESController).DisplayName();
			}
			else if (RightController == NESHawkSyncSettings.ControllerType.FourScore)
			{
				right = typeof(FourScore).DisplayName();
			}
			else
			{
				right = typeof(UnpluggedNES).DisplayName();
			}


			NES_ControllerDeck ret = new NES_ControllerDeck(left, right, subframe);

			return ret;
		}

		public LibNESHawk.ControllerReadCallback NES_Controller_Read;

		public byte Read_Controller(bool addr_4016)
		{
			return addr_4016 ? ControllerDeck.ReadPort1(Controller) : ControllerDeck.ReadPort2(Controller);
		}

		public LibNESHawk.ControllerStrobeCallback NES_Controller_Strobe;

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
			LibNESHawk.NES_Hard_Reset(NES_Pntr);
		}

		public void SoftReset()
		{
			LibNESHawk.NES_Soft_Reset(NES_Pntr);
		}

		public IntPtr NES_Pntr { get; set; } = IntPtr.Zero;
		public byte[] NES_core = new byte[0xA0000];

		private int _frame = 0;

		public DisplayType Region => DisplayType.NTSC;

		public readonly ITraceable Tracer;

		public LibNESHawk.TraceCallback tracecb;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		public void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			LibNESHawk.NES_getdisassembly(NES_Pntr, new_d, t, Disasm_Length);
			LibNESHawk.NES_getregisterstate(NES_Pntr, new_r, t, Reg_String_Length);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(40), registerInfo: new_r.ToString()));
		}

		// NES PPU Viewer
		public Action _NTVCallback;
		public int _NTVCallbackLine = 0;

		public void SetNTVCallback(Action callback, int line)
		{
			_NTVCallback = callback;
			_NTVCallbackLine = line;

			LibNESHawk.NES_setntvcallback(NES_Pntr, _NTVCallback, _NTVCallbackLine);
		}

		public Action _PPUViewCallback;
		public int _PPUViewCallbackLine = 0;

		public void SetPPUViewCallback(Action callback, int line)
		{
			_PPUViewCallback = callback;
			_PPUViewCallbackLine = line;

			LibNESHawk.NES_setppucallback(NES_Pntr, _PPUViewCallback, _PPUViewCallbackLine);
		}

		NESGPUMemoryAreas Mem_Domains = new NESGPUMemoryAreas();

		public NESGPUMemoryAreas GetMemoryAreas()
		{
			Mem_Domains.vram = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 0);
			Mem_Domains.oam = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 1);
			Mem_Domains.palram = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 2);
			Mem_Domains.mmio = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 3);

			return Mem_Domains;
		}

		private LibNESHawk.MessageCallback NES_message;

		private void GetMessage(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length+1);

			LibNESHawk.NES_getmessage(NES_Pntr, new_m);

			Console.WriteLine(new_m);
		}

		private LibNESHawk.InputPollCallback NES_InputPoll;

		public IInputCallbackSystem InputCallbacksSystem => InputCallbacks;

		public void Send_Input_Callback()
		{
			InputCallbacks.Call();
		}

		public byte[] Get_Core_Pal_RAM()
		{
			byte[] pal_ram_ret = new byte[0x20];

			Mem_Domains.palram = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 2);

			Marshal.Copy(Mem_Domains.palram, pal_ram_ret, 0, 0x20);

			return pal_ram_ret;
		}

		public byte[] Get_Core_OAM_RAM()
		{
			byte[] oam_ram_ret = new byte[0x100];

			Mem_Domains.oam = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 1);

			Marshal.Copy(Mem_Domains.oam, oam_ram_ret, 0, 0x100);

			return oam_ram_ret;
		}

		public byte[] Get_Core_EX_RAM()
		{
			byte[] ex_ram_ret = new byte[0x100];

			Mem_Domains.oam = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 3);

			Marshal.Copy(Mem_Domains.oam, ex_ram_ret, 0, 0x400);

			return ex_ram_ret;
		}

		public byte[] Get_Core_EX_CHR()
		{
			byte[] ex_chr_ret;

			if (CHR_ROM_Length != 0)
			{
				ex_chr_ret = new byte[CHR_ROM_Length];

				Mem_Domains.vram = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 0);

				Marshal.Copy(Mem_Domains.vram, ex_chr_ret, 0, (int)CHR_ROM_Length);
			}
			else
			{
				ex_chr_ret = new byte[0x400];

				Mem_Domains.vram = LibNESHawk.NES_get_ppu_pntrs(NES_Pntr, 4);

				Marshal.Copy(Mem_Domains.vram, ex_chr_ret, 0, 0x400);
			}

			return ex_chr_ret;
		}
	}
}
