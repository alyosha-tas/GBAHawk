﻿using System;

using BizHawk.Common;
using BizHawk.Emulation.Common;

using BizHawk.Emulation.Cores.Nintendo.GBA.Common;
using System.Runtime.InteropServices;

using BizHawk.Common.ReflectionExtensions;

/*
	GBA Emulator
	NOTES: 
	RAM disabling not implemented, check if used by any games

	Open bus behaviour needs to be done more carefully

	EEPROM accesses only emulated at 0xDxxxxxx, check if any games use lower range
*/

namespace BizHawk.Emulation.Cores.Nintendo.GBA.Common
{
	public interface IGBAGPUViewable : IEmulatorService
	{
		GBAGPUMemoryAreas GetMemoryAreas();

		/// <summary>
		/// calls correspond to entering hblank (maybe) and in a regular frame, the sequence of calls will be 160, 161, ..., 227, 0, ..., 159
		/// </summary>
		void SetScanlineCallback(Action callback, int scanline);
	}

	public class GBAGPUMemoryAreas
	{
		public IntPtr vram;
		public IntPtr oam;
		public IntPtr mmio;
		public IntPtr palram;
	}
}

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug
{
	[Core(CoreNames.GBAHawk_Debug, "")]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBAHawk_Debug : IEmulator, ISaveRam, IInputPollable, IRegionable, IGBAGPUViewable, 
	ISettable<GBAHawk_Debug.GBAHawk_Debug_Settings, GBAHawk_Debug.GBAHawk_Debug_SyncSettings>
	{
		internal static class RomChecksums
		{

		}

		public uint Memory_CTRL, ROM_Length;

		public uint Last_BIOS_Read;

		public int WRAM_Waits, SRAM_Waits;
		
		public int ROM_Waits_0_N, ROM_Waits_1_N, ROM_Waits_2_N, ROM_Waits_0_S, ROM_Waits_1_S, ROM_Waits_2_S;

		public ushort INT_EN, INT_Flags, INT_Master, Wait_CTRL;

		public ushort controller_state;

		public byte Post_Boot, Halt_CTRL;

		public bool All_RAM_Disable, WRAM_Enable;

		public bool INT_Master_On;

		// memory domains
		public byte[] WRAM = new byte[0x40000];
		public byte[] IWRAM = new byte[0x8000];
		public byte[] PALRAM = new byte[0x400];
		public byte[] VRAM = new byte[0x18000];
		public byte[] OAM = new byte[0x400];

		// vblank memory domains
		public byte[] WRAM_vbls = new byte[0x40000];
		public byte[] IWRAM_vbls = new byte[0x8000];
		public byte[] PALRAM_vbls = new byte[0x400];
		public byte[] VRAM_vbls = new byte[0x18000];
		public byte[] OAM_vbls = new byte[0x400];

		public byte[] BIOS;
		public readonly byte[] ROM = new byte[0x6000000];
		public readonly byte[] header = new byte[0x50];

		public byte[] cart_RAM;
		public byte[] cart_RAM_vbls;
		public bool has_bat;
		public bool Is_EEPROM;
		public bool EEPROM_Wiring; // when true, can access anywhere in 0xDxxxxxx range, otheriwse only 0xDFFFFE0

		private int Frame_Count = 0;

		public bool Use_MT;
		public ushort addr_access;

		public MapperBase mapper;

		private readonly ITraceable _tracer;

		[CoreConstructor(VSystemID.Raw.GBA)]
		public GBAHawk_Debug(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ GBAHawk_Debug_Settings settings, GBAHawk_Debug_SyncSettings syncSettings, bool subframe = false)
		{
			var ser = new BasicServiceProvider(this);
			
			_ = PutSettings(settings ?? new GBAHawk_Debug_Settings());
			_syncSettings = (GBAHawk_Debug_SyncSettings)syncSettings ?? new GBAHawk_Debug_SyncSettings();

			// Load up a BIOS and initialize the correct PPU
			BIOS = comm.CoreFileProvider.GetFirmwareOrThrow(new("GBA", "Bios"), "BIOS Not Found, Cannot Load");

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

			var mppr = Setup_Mapper(romHashMD5, romHashSHA1);
			
			if (cart_RAM != null) 
			{ 
				cart_RAM_vbls = new byte[cart_RAM.Length]; 

				// initialize SRAM to 0xFF;
				if (mppr == "SRAM")
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
						cart_RAM_vbls[i] = 0xFF;
					}
				}

				// initialize EEPROM to 0xFF;
				if (mppr == "EEPROM")
				{
					for (int i = 0; i < cart_RAM.Length; i++)
					{
						cart_RAM[i] = 0xFF;
						cart_RAM_vbls[i] = 0xFF;
					}
				}
			}

			mapper.Core =this;

			_controllerDeck = new(mppr is "MBC7"
				? typeof(StandardTilt).DisplayName()
				: GBAHawk_Debug_ControllerDeck.DefaultControllerName, subframe);

			ser.Register<IVideoProvider>(this);
			ser.Register<ISoundProvider>(this);
			ServiceProvider = ser;

			_ = PutSettings(settings ?? new GBAHawk_Debug_Settings());
			_syncSettings = (GBAHawk_Debug_SyncSettings)syncSettings ?? new GBAHawk_Debug_SyncSettings();

			_tracer = new TraceBuffer(TraceHeader);
			ser.Register<ITraceable>(_tracer);
			ser.Register<IStatable>(new StateSerializer(SyncState));
			SetupMemoryDomains();

			HardReset();

			DeterministicEmulation = true;

			Mem_Domains.vram = Marshal.AllocHGlobal(VRAM.Length + 1);
			Mem_Domains.oam = Marshal.AllocHGlobal(OAM.Length + 1);
			Mem_Domains.mmio = Marshal.AllocHGlobal(0x60 + 1);
			Mem_Domains.palram = Marshal.AllocHGlobal(PALRAM.Length + 1);
		}

		public ulong TotalExecutedCycles => CycleCount;

		public DisplayType Region => DisplayType.NTSC;

		private readonly GBAHawk_Debug_ControllerDeck _controllerDeck;

		public void HardReset()
		{
			delays_to_process = false;

			IRQ_Write_Delay = IRQ_Write_Delay_2 = IRQ_Write_Delay_3 = false;

			controller_state  = 0x3FF;

			Memory_CTRL = 0;

			Last_BIOS_Read = 0;

			WRAM_Waits = SRAM_Waits = 0;

			ROM_Waits_0_N = ROM_Waits_1_N = ROM_Waits_2_N = ROM_Waits_0_S = ROM_Waits_1_S = ROM_Waits_2_S = 0;

			INT_EN = INT_Flags = INT_Master = Wait_CTRL = 0;

			Post_Boot = Halt_CTRL = 0;

			All_RAM_Disable = WRAM_Enable = false;

			INT_Master_On = false;

			tim_Reset();
			ppu_Reset();
			snd_Reset();
			ser_Reset();
			cpu_Reset();
			dma_Reset();
			pre_Reset();
			mapper.Reset();

			vid_buffer = new uint[VirtualWidth * VirtualHeight];
			frame_buffer = new int[VirtualWidth * VirtualHeight];

			uint startup_color = 0xFFFFFFFF;
			for (int i = 0; i < vid_buffer.Length; i++)
			{
				vid_buffer[i] = startup_color;
				frame_buffer[i] = (int)vid_buffer[i];
			}

			for (int i = 0; i < IWRAM.Length; i++)
			{
				IWRAM[i] = 0;
			}

			for (int i = 0; i < WRAM.Length; i++)
			{
				WRAM[i] = 0;
			}

			// default memory config hardware initialized
			Update_Memory_CTRL(0x0D000020);
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
			Marshal.Copy(VRAM, 0, Mem_Domains.vram, VRAM.Length);
			Marshal.Copy(OAM, 0, Mem_Domains.oam, OAM.Length);
			Marshal.Copy(PALRAM, 0, Mem_Domains.palram, PALRAM.Length);

			byte[] temp = new byte[0x60];

			for (uint i = 0; i < 0x60; i++)
			{
				temp[i] = ppu_Read_Reg_8(i);
			}

			Marshal.Copy(temp, 0, Mem_Domains.mmio, temp.Length);

			return Mem_Domains;
		}

		public string Setup_Mapper(string romHashMD5, string romHashSHA1)
		{
			string mppr = "";
			has_bat = false;
			Is_EEPROM = false;
			EEPROM_Wiring = false;

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
							mppr = "SRAM";
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
							mppr = "EEPROM";
							break;
						}
					}
				}
				// TODO: Implmenet other save types
			}

			// hash checks for individual games / homebrew / test roms
			if (romHashSHA1 == "SHA1:C67E0A5E26EA5EBA2BC11C99D003027A96E44060") // Aging cart test
			{
				Console.WriteLine("using SRAM mapper");
				mppr = "SRAM";
			}

			if (mppr == "")
			{
				mppr = "NROM";
				mapper = new MapperDefault();
			}
			else if (mppr == "SRAM")
			{
				has_bat = true;
				cart_RAM = new byte[0x8000];
				mapper = new MapperSRAM();
			}
			else if (mppr == "EEPROM")
			{
				// assume 8 KB saves, use hash check to pick out 512 bytes versions
				has_bat = true;
				Is_EEPROM = true;

				if (ROM_Length < 0x1000000)
				{
					EEPROM_Wiring = true;
				}
				else
				{
					EEPROM_Wiring = false;
				}

				cart_RAM = new byte[0x2000];
				mapper = new MapperEEPROM();
			}

			return mppr;
		}
	}
}