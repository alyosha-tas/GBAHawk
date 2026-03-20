using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GB.Common
{
	/// <summary>
	/// static bindings into GBHawk.dll
	/// </summary>
	public static class LibGBHawkLink
	{
		private const string lib = "GBHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		/// <param name="count">number of systems used</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBLink_create(uint count);

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_destroy(IntPtr core);

		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_load_bios(IntPtr core, byte[] bios, bool gbcflag, bool gbc_gba_flag, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_load(IntPtr core, byte[] romdata, uint length, uint mapper, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_set_rtc(IntPtr core, int rtcdata, uint index, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_create_SRAM(IntPtr core, byte[] sram_data, uint length, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_load_SRAM(IntPtr core, byte[] sram_data, uint length, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_load_Palette(IntPtr core, bool palette, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_Sync_Domain_VBL(IntPtr core, bool on_VBL, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_Hard_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_Set_GBP_Enable(IntPtr core, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBLink_frame_advance(IntPtr core, byte[] ctrls, ushort[] accxs, ushort[] accys,
																	bool[] renders, bool[] sounds, bool[] resets);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_get_video(IntPtr core, int[] videobuf, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBLink_get_audio(IntPtr core, int[] aud_buf_L_0, ref uint n_samp_L_0, int[] aud_buf_R_0, ref uint n_samp_R_0,
																int[] aud_buf_L_1, ref uint n_samp_L_1, int[] aud_buf_R_1, ref uint n_samp_R_1, bool[] audio_enables);

		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_change_linking(IntPtr core, bool link_status, uint link_type);

		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_save_state(IntPtr core, byte[] saver);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_load_state(IntPtr core, byte[] loader);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getsysbus(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getvram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getoam(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_gethram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getsram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_settracecallback(IntPtr core, TraceCallback callback, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_getheaderlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_getdisasmlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_getregstringlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_getheader(IntPtr core, StringBuilder h, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_getregisterstate(IntPtr core, StringBuilder h, int t, int l, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_getdisassembly(IntPtr core, StringBuilder h, int t, int l, uint core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void MessageCallback(int str_length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_setmessagecallback(IntPtr core, MessageCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_getmessage(IntPtr core, StringBuilder h);
	}
}
