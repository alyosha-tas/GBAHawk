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

		[CoreConstructor(VSystemID.Raw.N64)]
		public N64Hawk(CoreComm comm, GameInfo game, byte[] rom, N64Hawk.N64HawkSettings settings, N64Hawk.N64HawkSyncSettings syncSettings, bool subframe = false)
		{
			ServiceProvider = new BasicServiceProvider(this);
			Settings = (N64HawkSettings)settings ?? new N64HawkSettings();
			SyncSettings = (N64HawkSyncSettings)syncSettings ?? new N64HawkSyncSettings();

			P1Controller = SyncSettings.P1Controller;
			P2Controller = SyncSettings.P2Controller;
			P3Controller = SyncSettings.P3Controller;
			P4Controller = SyncSettings.P4Controller;

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			N64_Pntr = LibN64Hawk.N64_create();

			N64_message = GetMessage;

			LibN64Hawk.N64_setmessagecallback(N64_Pntr, N64_message);

			LibN64Hawk.N64_load(N64_Pntr, GamePack, (uint)GamePack.Length, SyncSettings.Enable_RAM_Pack);

			if (cart_RAM != null) { LibN64Hawk.N64_create_SRAM(N64_Pntr, cart_RAM, (uint)cart_RAM.Length); }

			// set up initial palette
			N64CommonFunctions.SetPalette((byte[,])Palettes.QuickNESPalette.Clone(), Compiled_Palette);
			LibN64Hawk.N64_load_Palette(N64_Pntr, Compiled_Palette);

			blip_buff.SetRates(1789773, 44100);

			(ServiceProvider as BasicServiceProvider).Register<ISoundProvider>(this);

			SetupMemoryDomains();

			Header_Length = LibN64Hawk.N64_getheaderlength(N64_Pntr);
			Disasm_Length = LibN64Hawk.N64_getdisasmlength(N64_Pntr);
			Reg_String_Length = LibN64Hawk.N64_getregstringlength(N64_Pntr);

			var newHeader = new StringBuilder(Header_Length);
			LibN64Hawk.N64_getheader(N64_Pntr, newHeader, Header_Length);

			Console.WriteLine(Header_Length + " " + Disasm_Length + " " + Reg_String_Length);

			Tracer = new TraceBuffer(newHeader.ToString());

			var serviceProvider = ServiceProvider as BasicServiceProvider;
			serviceProvider.Register<ITraceable>(Tracer);
			serviceProvider.Register<IStatable>(new StateSerializer(SyncState));

			ResetControllerDefinition(subframe);

			N64_Controller_Read = Read_Controller;

			LibN64Hawk.N64_setcontrollercallback(N64_Pntr, N64_Controller_Read);

			N64_Controller_Strobe = Strobe_Controller;

			LibN64Hawk.N64_setstrobecallback(N64_Pntr, N64_Controller_Strobe);

			N64_InputPoll = Send_Input_Callback;

			LibN64Hawk.N64_setinputpollcallback(N64_Pntr, N64_InputPoll);
		}

		public N64HawkSyncSettings.ControllerType P1Controller;
		public N64HawkSyncSettings.ControllerType P2Controller;
		public N64HawkSyncSettings.ControllerType P3Controller;
		public N64HawkSyncSettings.ControllerType P4Controller;

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
			string P1 = N64_ControllerDeck.DefaultControllerName;
			string P2 = N64_ControllerDeck.DefaultControllerName;
			string P3 = N64_ControllerDeck.DefaultControllerName;
			string P4 = N64_ControllerDeck.DefaultControllerName;

			if (P1Controller == N64HawkSyncSettings.ControllerType.N64)
			{
				P1 = typeof(N64Controller).DisplayName();
			}
			else
			{
				P1 = typeof(UnpluggedN64).DisplayName();
			}

			if (P2Controller == N64HawkSyncSettings.ControllerType.N64)
			{
				P2 = typeof(N64Controller).DisplayName();
			}
			else
			{
				P2 = typeof(UnpluggedN64).DisplayName();
			}

			if (P3Controller == N64HawkSyncSettings.ControllerType.N64)
			{
				P3 = typeof(N64Controller).DisplayName();
			}
			else
			{
				P3 = typeof(UnpluggedN64).DisplayName();
			}

			if (P4Controller == N64HawkSyncSettings.ControllerType.N64)
			{
				P4 = typeof(N64Controller).DisplayName();
			}
			else
			{
				P4 = typeof(UnpluggedN64).DisplayName();
			}


			N64_ControllerDeck ret = new N64_ControllerDeck(P1, P2, P3, P4, subframe);

			return ret;
		}

		public LibN64Hawk.ControllerReadCallback N64_Controller_Read;

		public byte Read_Controller(bool addr_4016)
		{
			return addr_4016 ? ControllerDeck.ReadPort1(Controller) : ControllerDeck.ReadPort2(Controller);
		}

		public LibN64Hawk.ControllerStrobeCallback N64_Controller_Strobe;

		public void Strobe_Controller(byte latched_4016, byte new_val)
		{
			var si = new StrobeInfo(latched_4016, new_val);
			ControllerDeck.Strobe(si, Controller);
		}

		public ulong TotalExecutedCycles => 0;

		public void HardReset()
		{
			LibN64Hawk.N64_Hard_Reset(N64_Pntr);
		}

		public void SoftReset()
		{
			LibN64Hawk.N64_Soft_Reset(N64_Pntr);
		}

		public IntPtr N64_Pntr { get; set; } = IntPtr.Zero;
		public byte[] N64_core = new byte[0xA0000];

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

			LibN64Hawk.N64_getdisassembly(N64_Pntr, new_d, t, Disasm_Length);
			LibN64Hawk.N64_getregisterstate(N64_Pntr, new_r, t, Reg_String_Length);

			Tracer.Put(new(disassembly: new_d.ToString().PadRight(40), registerInfo: new_r.ToString()));
		}

		private LibN64Hawk.MessageCallback N64_message;

		private void GetMessage(int str_length)
		{
			StringBuilder new_m = new StringBuilder(str_length+1);

			LibN64Hawk.N64_getmessage(N64_Pntr, new_m);

			Console.WriteLine(new_m);
		}

		private LibN64Hawk.InputPollCallback N64_InputPoll;

		public IInputCallbackSystem InputCallbacksSystem => InputCallbacks;

		public void Send_Input_Callback()
		{
			InputCallbacks.Call();
		}
	}
}
