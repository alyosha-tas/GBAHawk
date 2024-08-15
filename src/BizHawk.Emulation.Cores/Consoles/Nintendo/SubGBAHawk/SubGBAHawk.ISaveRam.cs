using System;

using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBA
{
	public partial class SubGBAHawk : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			if (cart_RAM != null)
			{
				for (int i = 0; i < cart_RAM.Length; i++)
				{
					cart_RAM[i] = LibGBAHawk.GBA_getsram(GBA_Pntr, i);
				}

				return cart_RAM;
			}

			return (byte[])cart_RAM?.Clone();
		}

		public void StoreSaveRam(byte[] data)
		{
			if (use_sram)
			{
				Buffer.BlockCopy(data, 0, cart_RAM, 0, data.Length);
				Console.WriteLine("loading SRAM here");
				LibGBAHawk.GBA_load_SRAM(GBA_Pntr, cart_RAM, (uint)cart_RAM.Length);
			}
		}

		public bool SaveRamModified => has_bat && use_sram;
	}
}
