using System;
using System.Runtime.InteropServices;
using System.Text;
using BizHawk.Emulation.Common;

using BizHawk.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Cores.Nintendo.N64.Common;

namespace BizHawk.Emulation.Cores.Nintendo.N64Hawk
{
	[Core(CoreNames.N64Hawk, "", isReleased: false)]
	public partial class N64Hawk : IEmulator, IVideoProvider, ISoundProvider, ISaveRam, IInputPollable,
								ISettable<N64Hawk.N64HawkSettings, N64Hawk.N64HawkSyncSettings>
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

		[CoreConstructor(VSystemID.Raw.N64)]
		public N64Hawk(CoreComm comm, GameInfo game, byte[] rom, N64Hawk.N64HawkSettings settings, N64Hawk.N64HawkSyncSettings syncSettings, bool subframe = false)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (N64HawkSettings)settings ?? new N64HawkSettings();
			SyncSettings = (N64HawkSyncSettings)syncSettings ?? new N64HawkSyncSettings();

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

			NES_Pntr = LibN64Hawk.N64_create();

			NES_message = GetMessage;

			LibN64Hawk.N64_setmessagecallback(NES_Pntr, NES_message);

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

			bool bus_conflicts = SyncSettings.Mapper_Bus_Conflicts == true;
			bool apu_test_regs = SyncSettings.Use_APU_Test_Regs == true;
			bool cpu_zero = SyncSettings.CPU_Zero_Reset == true;

			LibN64Hawk.N64_load(NES_Pntr, GamePack, (uint)GamePack.Length, Header, bus_conflicts, apu_test_regs, cpu_zero);

			if (cart_RAM != null) { LibN64Hawk.N64_create_SRAM(NES_Pntr, cart_RAM, (uint)cart_RAM.Length); }

			// set up initial palette
			N64CommonFunctions.SetPalette((byte[,])Palettes.QuickNESPalette.Clone(), Compiled_Palette);
			LibN64Hawk.N64_load_Palette(NES_Pntr, Compiled_Palette);

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

			Header_Length = LibN64Hawk.N64_getheaderlength(NES_Pntr);
			Disasm_Length = LibN64Hawk.N64_getdisasmlength(NES_Pntr);
			Reg_String_Length = LibN64Hawk.N64_getregstringlength(NES_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibN64Hawk.N64_getheader(NES_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			Mem_Domains.vram = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 0);
			Mem_Domains.oam = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 1);
			Mem_Domains.palram = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 2);
			Mem_Domains.mmio = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 3);

			ResetControllerDefinition(subframe);

			NES_Controller_Read = Read_Controller;

			LibN64Hawk.N64_setcontrollercallback(NES_Pntr, NES_Controller_Read);

			NES_Controller_Strobe = Strobe_Controller;

			LibN64Hawk.N64_setstrobecallback(NES_Pntr, NES_Controller_Strobe);

			NES_InputPoll = Send_Input_Callback;

			LibN64Hawk.N64_setinputpollcallback(NES_Pntr, NES_InputPoll);
		}

		public N64HawkSyncSettings.ControllerType LeftController;
		public N64HawkSyncSettings.ControllerType RightController;

		private N64_ControllerDeck ControllerDeck;

		public ControllerDefinition ControllerDefinition { get; private set; }

		internal void ResetControllerDefinition(bool subframe)
		{
			ControllerDefinition = null;

			ControllerDeck = Controller_Instantiate(subframe);
			ControllerDefinition = ControllerDeck.Definition;
		}

		public N64_ControllerDeck Controller_Instantiate(bool subframe)
		{
			string left = N64_ControllerDeck.DefaultControllerName;
			string right = N64_ControllerDeck.DefaultControllerName;

			if (LeftController == N64HawkSyncSettings.ControllerType.N64)
			{
				left = typeof(N64Controller).DisplayName();
			}
			else
			{
				left = typeof(UnpluggedN64).DisplayName();
			}

			if (RightController == N64HawkSyncSettings.ControllerType.N64)
			{
				right = typeof(N64Controller).DisplayName();
			}
			else
			{
				right = typeof(UnpluggedN64).DisplayName();
			}


			N64_ControllerDeck ret = new N64_ControllerDeck(left, right, subframe);

			return ret;
		}

		public LibN64Hawk.ControllerReadCallback NES_Controller_Read;

		public byte Read_Controller(bool addr_4016)
		{
			return addr_4016 ? ControllerDeck.ReadPort1(Controller) : ControllerDeck.ReadPort2(Controller);
		}

		public LibN64Hawk.ControllerStrobeCallback NES_Controller_Strobe;

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
			LibN64Hawk.N64_Hard_Reset(NES_Pntr);
		}

		public void SoftReset()
		{
			LibN64Hawk.N64_Soft_Reset(NES_Pntr);
		}

		public IntPtr NES_Pntr { get; set; } = IntPtr.Zero;
		public byte[] NES_core = new byte[0xA0000];

		private int _frame = 0;

		public DisplayType Region => DisplayType.NTSC;

		public readonly ITraceable Tracer;

		public LibN64Hawk.TraceCallback tracecb;

		// these will be constant values assigned during core construction
		private int Header_Length;
		private readonly int Disasm_Length;
		private readonly int Reg_String_Length;

		public void MakeTrace(int t)
		{
			StringBuilder new_d = new StringBuilder(Disasm_Length);
			StringBuilder new_r = new StringBuilder(Reg_String_Length);

			LibN64Hawk.N64_getdisassembly(NES_Pntr, new_d, t, Disasm_Length);
			LibN64Hawk.N64_getregisterstate(NES_Pntr, new_r, t, Reg_String_Length);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(40), registerInfo: new_r.ToString()));
		}

		// NES PPU Viewer
		public Action _NTVCallback;
		public int _NTVCallbackLine = 0;

		public void SetNTVCallback(Action callback, int line)
		{
			_NTVCallback = callback;
			_NTVCallbackLine = line;

			LibN64Hawk.N64_setntvcallback(NES_Pntr, _NTVCallback, _NTVCallbackLine);
		}

		public Action _PPUViewCallback;
		public int _PPUViewCallbackLine = 0;

		public void SetPPUViewCallback(Action callback, int line)
		{
			_PPUViewCallback = callback;
			_PPUViewCallbackLine = line;

			LibN64Hawk.N64_setppucallback(NES_Pntr, _PPUViewCallback, _PPUViewCallbackLine);
		}

		N64GPUMemoryAreas Mem_Domains = new N64GPUMemoryAreas();

		public N64GPUMemoryAreas GetMemoryAreas()
		{
			Mem_Domains.vram = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 0);
			Mem_Domains.oam = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 1);
			Mem_Domains.palram = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 2);
			Mem_Domains.mmio = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 3);

			return Mem_Domains;
		}

		private LibN64Hawk.MessageCallback NES_message;

		private void GetMessage(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length+1);

			LibN64Hawk.N64_getmessage(NES_Pntr, new_m);

			Console.WriteLine(new_m);
		}

		private LibN64Hawk.InputPollCallback NES_InputPoll;

		public IInputCallbackSystem InputCallbacksSystem => InputCallbacks;

		public void Send_Input_Callback()
		{
			InputCallbacks.Call();
		}

		public byte[] Get_Core_Pal_RAM()
		{
			byte[] pal_ram_ret = new byte[0x20];

			Mem_Domains.palram = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 2);

			Marshal.Copy(Mem_Domains.palram, pal_ram_ret, 0, 0x20);

			return pal_ram_ret;
		}

		public byte[] Get_Core_OAM_RAM()
		{
			byte[] oam_ram_ret = new byte[0x100];

			Mem_Domains.oam = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 1);

			Marshal.Copy(Mem_Domains.oam, oam_ram_ret, 0, 0x100);

			return oam_ram_ret;
		}

		public byte[] Get_Core_EX_RAM()
		{
			byte[] ex_ram_ret = new byte[0x100];

			Mem_Domains.oam = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 3);

			Marshal.Copy(Mem_Domains.oam, ex_ram_ret, 0, 0x400);

			return ex_ram_ret;
		}

		public byte[] Get_Core_EX_CHR()
		{
			byte[] ex_chr_ret;

			if (CHR_ROM_Length != 0)
			{
				ex_chr_ret = new byte[CHR_ROM_Length];

				Mem_Domains.vram = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 0);

				Marshal.Copy(Mem_Domains.vram, ex_chr_ret, 0, (int)CHR_ROM_Length);
			}
			else
			{
				ex_chr_ret = new byte[0x400];

				Mem_Domains.vram = LibN64Hawk.N64_get_ppu_pntrs(NES_Pntr, 4);

				Marshal.Copy(Mem_Domains.vram, ex_chr_ret, 0, 0x400);
			}

			return ex_chr_ret;
		}
	}
}
