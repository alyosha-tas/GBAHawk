using System;

using BizHawk.Emulation.Common;

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

					for (int i = 0; i < cart_RAMS[0].Length; i++)
					{
						cart_RAMS[0][i] = LibGBAHawkLink.GBALink_getsram(GBA_Pntr, i, 0);
					}		
				}

				if (cart_RAMS[1] != null)
				{
					len2 = cart_RAMS[1].Length;

					for (int i = 0; i < cart_RAMS[1].Length; i++)
					{
						cart_RAMS[1][i] = LibGBAHawkLink.GBALink_getsram(GBA_Pntr, i, 1);
					}
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
			if (use_sram)
			{
				if (cart_RAMS[0] != null && cart_RAMS[1] == null)
				{
					Buffer.BlockCopy(data, 0, cart_RAMS[0], 0, cart_RAMS[0].Length);

					LibGBAHawkLink.GBALink_load_SRAM(GBA_Pntr, cart_RAMS[0], (uint)cart_RAMS[0].Length, 0);
				}
				else if (cart_RAMS[1] != null && cart_RAMS[0] == null)
				{
					Buffer.BlockCopy(data, 0, cart_RAMS[1], 0, cart_RAMS[1].Length);

					LibGBAHawkLink.GBALink_load_SRAM(GBA_Pntr, cart_RAMS[1], (uint)cart_RAMS[1].Length, 1);
				}
				else if (cart_RAMS[1] != null && cart_RAMS[0] != null)
				{
					Buffer.BlockCopy(data, 0, cart_RAMS[0], 0, cart_RAMS[0].Length);
					Buffer.BlockCopy(data, cart_RAMS[0].Length, cart_RAMS[1], 0, cart_RAMS[1].Length);

					LibGBAHawkLink.GBALink_load_SRAM(GBA_Pntr, cart_RAMS[0], (uint)cart_RAMS[0].Length, 0);
					LibGBAHawkLink.GBALink_load_SRAM(GBA_Pntr, cart_RAMS[1], (uint)cart_RAMS[1].Length, 1);
				}
			}
		}

		public bool SaveRamModified => (has_bats[0] || has_bats[1]) && use_sram;
	}
}
