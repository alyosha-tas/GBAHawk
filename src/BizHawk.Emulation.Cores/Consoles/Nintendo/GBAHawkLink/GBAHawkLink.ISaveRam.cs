using System;

using BizHawk.Emulation.Common;
using static BizHawk.Emulation.Cores.Nintendo.GBHawkLink.GBHawkLink;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	public partial class GBAHawkLink : ISaveRam
	{
		public byte[] CloneSaveRam()
		{
			if (cart_RAMS[0] != null || cart_RAMS[1] != null)
			{
				int len1 = 0;
				int len2 = 0;
				int index = 0;

				if (cart_RAMS[0] != null)
				{
					len1 = cart_RAMS[0].Length;
				}

				if (cart_RAMS[1] != null)
				{
					len2 = cart_RAMS[1].Length;
				}

				byte[] temp = new byte[len1 + len2];

				if (cart_RAMS[0] != null)
				{
					for (int i = 0; i < cart_RAMS[0].Length; i++)
					{
						temp[index] = cart_RAMS[0][i];
						index++;
					}
				}

				if (cart_RAMS[1] != null)
				{
					for (int i = 0; i < cart_RAMS[1].Length; i++)
					{
						temp[index] = cart_RAMS[1][i];
						index++;
					}
				}

				return temp;
			}
			return null;
		}

		public void StoreSaveRam(byte[] data)
		{
			if (SyncSettings.Use_SRAM)
			{
				if (cart_RAMS[0] != null && cart_RAMS[1] == null)
				{
					Buffer.BlockCopy(data, 0, cart_RAMS[0], 0, cart_RAMS[0].Length);
				}
				else if (cart_RAMS[1] != null && cart_RAMS[0] == null)
				{
					Buffer.BlockCopy(data, 0, cart_RAMS[1], 0, cart_RAMS[1].Length);
				}
				else if (cart_RAMS[1] != null && cart_RAMS[0] != null)
				{
					Buffer.BlockCopy(data, 0, cart_RAMS[0], 0, cart_RAMS[0].Length);
					Buffer.BlockCopy(data, cart_RAMS[0].Length, cart_RAMS[1], 0, cart_RAMS[1].Length);
				}

				Console.WriteLine("loading SRAM here");
			}
		}

		public bool SaveRamModified => (has_bats[0] || has_bats[1]) & SyncSettings.Use_SRAM;
	}
}
