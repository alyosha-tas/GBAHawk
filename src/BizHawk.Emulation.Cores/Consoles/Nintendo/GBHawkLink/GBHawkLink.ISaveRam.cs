using System;

using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	public partial class GBHawkLink : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			if (cart_RAM[0] != null || cart_RAM[1] != null)
			{
				int len1 = 0;
				int len2 = 0;
				int index = 0;

				if (cart_RAM[0] != null)
				{
					len1 = cart_RAM[0].Length;

					for (int i = 0; i < cart_RAM[0].Length; i++)
					{
						cart_RAM[0][i] = LibGBHawkLink.GBLink_getsram(GBLink_Pntr, i, Current_sync_on_vbl[0], 0);
					}		
				}

				if (cart_RAM[1] != null)
				{
					len2 = cart_RAM[1].Length;

					for (int i = 0; i < cart_RAM[1].Length; i++)
					{
						cart_RAM[1][i] = LibGBHawkLink.GBLink_getsram(GBLink_Pntr, i, Current_sync_on_vbl[1], 1);
					}
				}

				byte[] temp = new byte[len1 + len2];

				if (cart_RAM[0] != null)
				{
					for (int i = 0; i < cart_RAM[0].Length; i++)
					{
						temp[index] = cart_RAM[0][i];
						index++;
					}
				}

				if (cart_RAM[1] != null)
				{
					for (int i = 0; i < cart_RAM[1].Length; i++)
					{
						temp[index] = cart_RAM[1][i];
						index++;
					}
				}

				return temp;
			}
			return null;
		}

		public void StoreSaveRam(byte[] data)
		{
			if (cart_RAM[0] != null && cart_RAM[1] == null)
			{
				Buffer.BlockCopy(data, 0, cart_RAM[0], 0, cart_RAM[0].Length);

				LibGBHawkLink.GBLink_load_SRAM(GBLink_Pntr, cart_RAM[0], (uint)cart_RAM[0].Length, 0);
			}
			else if (cart_RAM[1] != null && cart_RAM[0] == null)
			{
				Buffer.BlockCopy(data, 0, cart_RAM[1], 0, cart_RAM[1].Length);

				LibGBHawkLink.GBLink_load_SRAM(GBLink_Pntr, cart_RAM[1], (uint)cart_RAM[1].Length, 1);
			}
			else if (cart_RAM[1] != null && cart_RAM[0] != null)
			{
				Buffer.BlockCopy(data, 0, cart_RAM[0], 0, cart_RAM[0].Length);
				Buffer.BlockCopy(data, cart_RAM[0].Length, cart_RAM[1], 0, cart_RAM[1].Length);

				LibGBHawkLink.GBLink_load_SRAM(GBLink_Pntr, cart_RAM[0], (uint)cart_RAM[0].Length, 0);
				LibGBHawkLink.GBLink_load_SRAM(GBLink_Pntr, cart_RAM[1], (uint)cart_RAM[1].Length, 1);
			}
		}

		public bool SaveRamModified => (has_bat[0] || has_bat[1] || has_bat[2] || has_bat[3]);
	}
}
