using System;
using BizHawk.Common;

namespace BizHawk.Emulation.Cores.Nintendo.NES
{
	//generally mapper2

	//Mega Man
	//Castlevania
	//Contra
	//Duck Tales
	//Metal Gear

	//TODO - look for a mirror=H UNROM--maybe there are none? this may be fixed to the board type.

	// why are there no bus conflicts in here???????

	[NesBoardImplPriority]
	internal sealed class UxROM : NesBoardBase
	{
		//configuration
		private int prg_mask;
		private int vram_byte_mask;
		private Func<int, int> adjust_prg;

		//state
		private int prg;

		//the VS actually does have 2 KB of nametable address space
		//let's make the extra space here, instead of in the main NES to avoid confusion
		private byte[] CIRAM_VS = new byte[0x800];

		public override bool Configure(EDetectionOrigin origin)
		{
			adjust_prg = (x) => x;

			//configure
			switch (Cart.BoardType)
			{
				case "MAPPER0002-00":
					//probably a mistake. 
					//but (for chrram): "Use of $00 with no CHR ROM implies that the game is wired to map nametable memory in CHR space. The value $00 MUST NOT be used if a mapper isn't defined to allow this. "
					//well, i'm not going to do that now. we'll save it for when it's needed
					//"it's only mapper 218 and no other mappers"
					//so, don't assume this
					//Cart.vram_size = 8;
					break;

				case "MAPPER002":
					AssertChr(0); Cart.VramSize = 8;
					break;

				case "NES-UNROM": //mega man
				case "HVC-UNROM": 
				case "KONAMI-UNROM":
				case "NES-UNEPROM": // proto
				case "IREM-UNROM":
				case "TAITO-UNROM":
					AssertPrg(128); AssertChr(0); AssertVram(8);
					//AssertWram(0); //JJ - Tobidase Daisakusen Part 2 (J) includes WRAM
					break;
	
				case "HVC-UN1ROM":
					AssertPrg(128); AssertChr(0); AssertWram(0); AssertVram(8);
					adjust_prg = (x) => ((x >> 2) & 7);
					break;

				case "NES-UOROM": //paperboy 2
				case "HVC-UOROM":
				case "JALECO-JF-15":
				case "JALECO-JF-18":
					AssertPrg(256); AssertChr(0); AssertVram(8); AssertWram(0);
					break;
				default:
					return false;
			}
			//these boards always have 8KB of VRAM
			vram_byte_mask = (Cart.VramSize*1024) - 1;
			prg_mask = (Cart.PrgSize / 16) - 1;
			SetMirrorType(Cart.PadH, Cart.PadV);

			return true;
		}

		public override byte ReadPrg(int addr)
		{
			int block = addr >> 14;
			int page = block == 1 ? prg_mask : prg;
			int ofs = addr & 0x3FFF;
			return Rom[(page << 14) | ofs];
		}
		public override void WritePrg(int addr, byte value)
		{
			prg = adjust_prg(value) & prg_mask;
		}

		public override byte ReadPpu(int addr)
		{
			if (addr < 0x2000)
			{
				return Vram[addr & vram_byte_mask];
			}
			else
			{
				return base.ReadPpu(addr);
			}
		}

		public override void WritePpu(int addr, byte value)
		{
			if (addr < 0x2000)
			{
				Vram[addr & vram_byte_mask] = value;
			}
			else
				base.WritePpu(addr,value);
		}

		public override void SyncState(Serializer ser)
		{
			base.SyncState(ser);
			ser.Sync(nameof(prg), ref prg);
		}
	}
}
