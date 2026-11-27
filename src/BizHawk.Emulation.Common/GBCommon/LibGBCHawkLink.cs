using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GB.Common
{
	/// <summary>
	/// static bindings into GBCHawk.dll
	/// </summary>
	public static class LibGBCHawkLink
	{
		private const string lib = "GBCHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		/// <param name="count">number of systems used</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBCLink_create(uint count);

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_destroy(IntPtr core);

		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_load_bios(IntPtr core, byte[] bios, bool gbc_gba_flag, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_load(IntPtr core, byte[] romdata, uint length, uint mapper, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_set_rtc(IntPtr core, int rtcdata, uint index, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_create_SRAM(IntPtr core, byte[] sram_data, uint length, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_load_SRAM(IntPtr core, byte[] sram_data, uint length, uint console_num);

		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_Sync_Domain_VBL(IntPtr core, bool on_VBL, uint console_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_Hard_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_Set_GBP_Enable(IntPtr core, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBCLink_frame_advance(IntPtr core, byte[] ctrls, ushort[] accxs, ushort[] accys,
																	bool[] renders, bool[] sounds, bool[] resets);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_get_video(IntPtr core, int[] videobuf, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBCLink_get_audio(IntPtr core, int[] aud_buf_L_0, ref uint n_samp_L_0, int[] aud_buf_R_0, ref uint n_samp_R_0,
																int[] aud_buf_L_1, ref uint n_samp_L_1, int[] aud_buf_R_1, ref uint n_samp_R_1, bool[] audio_enables);

		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_change_linking(IntPtr core, bool link_status, uint link_type);

		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_save_state(IntPtr core, byte[] saver);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_load_state(IntPtr core, byte[] loader);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBCLink_getsysbus(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBCLink_getvram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBCLink_getoam(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBCLink_getram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBCLink_gethram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBCLink_getsram(IntPtr core, int addr, bool on_vbl, uint core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_settracecallback(IntPtr core, TraceCallback callback, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_getheaderlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_getdisasmlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBCLink_getregstringlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_getheader(IntPtr core, StringBuilder h, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_getregisterstate(IntPtr core, StringBuilder h, int t, int l, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_getdisassembly(IntPtr core, StringBuilder h, int t, int l, uint core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void MessageCallback(int str_length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_setmessagecallback(IntPtr core, MessageCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBCLink_getmessage(IntPtr core, StringBuilder h);
	}
}
