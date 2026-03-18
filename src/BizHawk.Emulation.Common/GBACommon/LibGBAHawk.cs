using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBA.Common
{
	/// <summary>
	/// static bindings into GBAHawk.dll
	/// </summary>
	public static class LibGBAHawk
	{
		private const string lib = "GBAHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBA_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_destroy(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_load_bios(IntPtr core, byte[] bios);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_load(IntPtr core, byte[] romdata, uint length, int mapper, ulong datetime, bool rtc_functional, 
										  short EEPROMoffset, ushort FlashType64Value, ushort FlashType128Value, short FlashWriteOffset, int FlashSectorOffset, int FlashChipOffset, bool is_GBP);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_create_SRAM(IntPtr core, byte[] sram_data, uint length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_load_SRAM(IntPtr core, byte[] sram_data, uint length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_Hard_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_Set_GBP_Enable(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern bool GBA_frame_advance(IntPtr core, ushort ctrl1, ushort accx, ushort accy, byte solar, bool render, bool sound);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBA_subframe_advance(IntPtr core, ushort ctrl1, ushort accx, ushort accy, byte solar, bool render, bool sound, bool do_reset, uint reset_cycle);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern void GBA_get_video(IntPtr core, int[] videobuf);


		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBA_get_audio(IntPtr core, int[] aud_buf_L, ref uint n_samp_L, int[] aud_buf_R, ref uint n_samp_R);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_save_state(IntPtr core, byte[] saver);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_load_state(IntPtr core, byte[] loader);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getsysbus(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getvram(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getoam(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getpalram(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getregisters(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getwram(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getiwram(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getsram(IntPtr core, int addr);


		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_settracecallback(IntPtr core, TraceCallback callback);


		[UnmanagedFunctionPointer(cc)]
		public delegate void RumbleCallback(bool rumble_on);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_setrumblecallback(IntPtr core, RumbleCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_getheaderlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_getdisasmlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_getregstringlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getheader(IntPtr core, StringBuilder h, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getregisterstate(IntPtr core, StringBuilder h, int t, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getdisassembly(IntPtr core, StringBuilder h, int t, int l);


		[UnmanagedFunctionPointer(cc)]
		public delegate void MessageCallback(int str_length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_setmessagecallback(IntPtr core, MessageCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getmessage(IntPtr core, StringBuilder h);


		#region PPU_Viewer

		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBA_get_ppu_pntrs(IntPtr core, int sel);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_setscanlinecallback(IntPtr core, Action callback, int sl);

		#endregion
	}
}
