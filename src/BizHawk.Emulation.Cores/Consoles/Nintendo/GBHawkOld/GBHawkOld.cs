using System;

using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Emulation.Cores.Consoles.Nintendo.Gameboy;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using BizHawk.Common.ReflectionExtensions;

// TODO: mode1_disableint_gbc.gbc behaves differently between GBC and GBA, why?
// TODO: Window Position A6 behaves differently
// TODO: Verify open bus behaviour for bad SRAM accesses for other MBCs
// TODO: Apparently sprites at x=A7 do not stop the trigger for FF0F bit flip, but still do not dispatch interrupt or
// mode 3 change, see 10spritesPrLine_10xposA7_m0irq_2_dmg08_cgb04c_out2.gbc
// TODO: there is a tile glitch when setting LCDC.Bit(4) in GBC that is not implemented yet, the version of the glitch for reset is implemented though
// TODO: In some GBC models, apparently unmapped memory after OAM contains 48 bytes that are fully read/write'able
// this is not implemented and which models it effects is not clear, see oam_echo_ram_read.gbc and oam_echo_ram_read_2.gbc

namespace BizHawk.Emulation.Cores.Nintendo.GBHawkOld
{
	[Core(CoreNames.GBHawk, "")]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBHawkOld : IEmulator, ISaveRam, IInputPollable, IRegionable, IGameboyCommon,
	ISettable<GBHawkOld.GBSettings, GBHawkOld.GBSyncSettings>
	{

		public int RAM_Bank;
		public int RAM_Bank_ret;
		public byte VRAM_Bank;
		internal bool is_GB_in_GBC;
		public bool double_speed;
		public bool speed_switch;
		public bool HDMA_transfer; // stalls CPU when in progress
		public byte bus_value; // we need the last value on the bus for proper emulation of blocked SRAM
		public ulong bus_access_time; // also need to keep track of the time of the access since it doesn't last very long
		public byte IR_reg, IR_mask, IR_signal, IR_receive, IR_self;
		public int IR_write;


		public ushort addr_access;

		[CoreConstructor(VSystemID.Raw.GB)]
		[CoreConstructor(VSystemID.Raw.GBC)]
		public GBHawkOld(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ GBSettings settings, GBSyncSettings syncSettings, bool subframe = false)
		{
			var ser = new BasicServiceProvider(this);

			is_subframe_core = subframe;

			cpu = new LR35902
			{
				ReadMemory = ReadMemory,
				WriteMemory = WriteMemory,
				PeekMemory = PeekMemory,
				DummyReadMemory = ReadMemory,
				SpeedFunc = SpeedFunc,
				GetButtons = GetButtons,
				GetIntRegs = GetIntRegs,
				SetIntRegs = SetIntRegs
			};
			
			timer = new Timer();
			audio = new Audio();
			serialport = new SerialPort();

			_ = PutSettings(settings ?? new GBSettings());
			_syncSettings = (GBSyncSettings)syncSettings ?? new GBSyncSettings();

			// Load up a BIOS and initialize the correct PPU
			_bios = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBC", "World"), "BIOS Not Found, Cannot Load");
			ppu = new GBC_PPU();

			// set up IR register to off state
			IR_mask = 0; IR_reg = 0x3E; IR_receive = 2; IR_self = 2; IR_signal = 2;

			// Here we modify the BIOS if GBA mode is set (credit to ExtraTricky)
			if ( _syncSettings.GBACGB)
			{
				for (int i = 0; i < 13; i++)
				{
					_bios[i + 0xF3] = (byte)((GBA_override[i] + _bios[i + 0xF3]) & 0xFF);
				}

				IR_mask = 2;
			}

			Buffer.BlockCopy(rom, 0x100, header, 0, 0x50);

			if ((header[0x43] != 0x80) && (header[0x43] != 0xC0))
			{
				ppu = new GBC_GB_PPU();
				is_GB_in_GBC = true; // for movie files
			}

			var romHashMD5 = MD5Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashMD5);
			var romHashSHA1 = SHA1Checksum.ComputePrefixedHex(rom);
			Console.WriteLine(romHashSHA1);

			_rom = rom;
			var mppr = Setup_Mapper(romHashMD5, romHashSHA1);
			if (cart_RAM != null) { cart_RAM_vbls = new byte[cart_RAM.Length]; }

			_controllerDeck = new(mppr is "MBC7"
				? typeof(StandardTilt).DisplayName()
				: GBHawkControllerDeck.DefaultControllerName, subframe);

			timer.Core = this;
			audio.Core = this;
			ppu.Core = this;
			serialport.Core = this;

			ser.Register<IVideoProvider>(this);
			ser.Register<ISoundProvider>(audio);
			ServiceProvider = ser;

			_ = PutSettings(settings ?? new GBSettings());
			_syncSettings = (GBSyncSettings)syncSettings ?? new GBSyncSettings();

			_tracer = new TraceBuffer(cpu.TraceHeader);
			ser.Register<ITraceable>(_tracer);
			ser.Register<IStatable>(new StateSerializer(SyncState));
            ser.Register<IDisassemblable>(_disassembler);
			SetupMemoryDomains();
			cpu.SetCallbacks(ReadMemory, PeekMemory, PeekMemory, WriteMemory);
			HardReset();

			_scanlineCallback = null;
		}

		public ulong TotalExecutedCycles => _settings.cycle_return_setting == GBSettings.Cycle_Return.CPU ? cpu.TotalExecutedCycles : CycleCount;

		public bool IsCGBMode() => true;

		public bool IsCGBDMGMode() => is_GB_in_GBC;

		/// <summary>
		/// Produces a palette in the form that certain frontend inspection tools.
		/// May or may not return a reference to the core's own palette, so please don't mutate.
		/// </summary>
		private uint[] SynthesizeFrontendBGPal()
		{
			return ppu.BG_palette;
		}

		/// <summary>
		/// Produces a palette in the form that certain frontend inspection tools.
		/// May or may not return a reference to the core's own palette, so please don't mutate.
		/// </summary>
		private uint[] SynthesizeFrontendSPPal()
		{
			return ppu.OBJ_palette;
		}

		public IGPUMemoryAreas LockGPU()
		{
			return new GPUMemoryAreas(
				VRAM,
				OAM,
				SynthesizeFrontendSPPal(),
				SynthesizeFrontendBGPal()
			);
		}

		private class GPUMemoryAreas : IGPUMemoryAreas
		{
			public IntPtr Vram { get; }

			public IntPtr Oam { get; }

			public IntPtr Sppal { get; }

			public IntPtr Bgpal { get; }

			private readonly List<GCHandle> _handles = new();

			public GPUMemoryAreas(byte[] vram, byte[] oam, uint[] sppal, uint[] bgpal)
			{
				Vram = AddHandle(vram);
				Oam = AddHandle(oam);
				Sppal = AddHandle(sppal);
				Bgpal = AddHandle(bgpal);
			}

			private IntPtr AddHandle(object target)
			{
				var handle = GCHandle.Alloc(target, GCHandleType.Pinned);
				_handles.Add(handle);
				return handle.AddrOfPinnedObject();
			}

			public void Dispose()
			{
				foreach (var h in _handles)
					h.Free();
				_handles.Clear();
			}
		}

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
