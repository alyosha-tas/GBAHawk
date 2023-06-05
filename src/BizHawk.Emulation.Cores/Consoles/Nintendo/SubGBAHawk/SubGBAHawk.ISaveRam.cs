using System;

using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBA
{
	public partial class SubGBAHawk : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			return (byte[])cart_RAM?.Clone();
		}

		public void StoreSaveRam(byte[] data)
		{
			if (SyncSettings.Use_SRAM)
			{
				Buffer.BlockCopy(data, 0, cart_RAM, 0, data.Length);
				Console.WriteLine("loading SRAM here");
				LibSubGBAHawk.GBA_load_SRAM(GBA_Pntr, cart_RAM, (uint)cart_RAM.Length);
			}
		}

		public bool SaveRamModified => has_bat & SyncSettings.Use_SRAM;
	}
}
