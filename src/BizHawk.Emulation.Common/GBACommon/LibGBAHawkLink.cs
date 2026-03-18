using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBA.Common
{
	/// <summary>
	/// static bindings into GBAHawk.dll
	/// </summary>
	public static class LibGBAHawkLink
	{
		private const string lib = "GBAHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBALink_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_destroy(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_load_bios(IntPtr core, byte[] bios);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_load(IntPtr core, byte[] romdata0, uint length0, int mapper0,
															byte[] romdata1, uint length1, int mapper1,
															ulong datetime0, bool rtc_functional0,
															ulong datetime1, bool rtc_functional1,
															short EEPROMoffset0, short EEPROMoffset1,
															ushort FlashType64Value0, ushort FlashType64Value1,
															ushort FlashType128Value0, ushort FlashType128Value1,
															short FlashWriteOffset_L, short FlashWriteOffset_R,
															int FlashSectorOffset_L, int FlashSectorOffset_R,
															int FlashChipOffset_L, int FlashChipOffset_R,
															bool is_GBP0, bool is_GBP1);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_create_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_load_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_Hard_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_Set_GBP_Enable(IntPtr core, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern bool GBALink_frame_advance(IntPtr core, ushort ctrl0, ushort accx0, ushort accy0, byte solar0, bool render0, bool sound0,
																	 ushort ctrl1, ushort accx1, ushort accy1, byte solar1, bool render1, bool sound1,
																	 bool l_reset, bool r_reset);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_get_video(IntPtr core, int[] videobuf, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBALink_get_audio(IntPtr core, int[] aud_buf_L, ref uint n_samp_L, int[] aud_buf_R, ref uint n_samp_R, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_save_state(IntPtr core, byte[] saver);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_load_state(IntPtr core, byte[] loader);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getsysbus(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getvram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getoam(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getpalram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getwram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getiwram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getsram(IntPtr core, int addr, int core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_settracecallback(IntPtr core, TraceCallback callback, uint core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void RumbleCallback(bool rumble_on);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_setrumblecallback(IntPtr core, RumbleCallback callback, int num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getheaderlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getdisasmlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getregstringlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getheader(IntPtr core, StringBuilder h, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getregisterstate(IntPtr core, StringBuilder h, int t, int l, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getdisassembly(IntPtr core, StringBuilder h, int t, int l, uint core_num);
	}
}
