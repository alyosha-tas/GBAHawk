using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.NES.Common;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.NESHawk
{
	public partial class NESHawk : INESPPUViewable
	{
		public int[] GetPalette() => Compiled_Palette;

		public bool BGBaseHigh => LibNESHawk.NES_get_ppu_vals(NES_Pntr, 0);

		public bool SPBaseHigh => LibNESHawk.NES_get_ppu_vals(NES_Pntr, 1);

		public bool SPTall => LibNESHawk.NES_get_ppu_vals(NES_Pntr, 2);

		public byte[] GetPPUBus()
		{
			byte[] ret = new byte[0x3000];
			for (uint i = 0; i < 0x3000; i++)
			{
				ret[i] = LibNESHawk.NES_get_ppu_bus(NES_Pntr, i);
			}
			return ret;
		}

		public byte[] GetPalRam() => Get_Core_Pal_RAM();

		public byte[] GetOam() => Get_Core_OAM_RAM();

		public byte PeekPPU(int addr) => LibNESHawk.NES_get_ppu_bus(NES_Pntr, (uint)addr);

		public byte[] GetExTiles()
		{
			if (mapper == 5)
			{
				return Get_Core_EX_CHR();
			}
			else
			{
				throw new InvalidOperationException();
			}
		}

		public bool ExActive => LibNESHawk.NES_get_ppu_vals(NES_Pntr, 3);

		public byte[] GetExRam()
		{
			if (mapper == 5)
			{
				return Get_Core_EX_RAM();
			}

			throw new InvalidOperationException();
		}

		public MemoryDomain GetCHRROM() => MemoryDomains["CHR ROM"];


		public void InstallCallback1(Action cb, int sl)
		{
			SetNTVCallback(cb, sl);
		}

		public void InstallCallback2(Action cb, int sl)
		{
			SetPPUViewCallback(cb, sl);
		}

		public void RemoveCallback1()
		{
			SetNTVCallback(null, 0);
		}

		public void RemoveCallback2()
		{
			SetPPUViewCallback(null, 0);
		}
	}
}
