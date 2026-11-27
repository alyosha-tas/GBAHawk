using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GB.Common
{
	/// <summary>
	/// static bindings into GBCHawk.dll
	/// </summary>
	public static class LibGBCHawk
	{
		private const string lib = "GBCHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBC_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_destroy(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_load_bios(IntPtr core, byte[] bios, bool gbc_gba_flag);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_load(IntPtr core, byte[] romdata, uint length, uint mapper_number);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_set_rtc(IntPtr core, int rtcdata, uint index);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_create_SRAM(IntPtr core, byte[] sram_data, uint length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_load_SRAM(IntPtr core, byte[] sram_data, uint length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_Sync_Domain_VBL(IntPtr core, bool on_VBL);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_Hard_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBC_frame_advance(IntPtr core, byte ctrl1, ushort accx, ushort accy, bool render, bool sound);


		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong GBC_get_cycles(IntPtr core, bool cycle_type);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBC_subframe_advance(IntPtr core, byte ctrl1, ushort accx, ushort accy, bool render, bool sound, bool do_reset, uint input_cycle);


		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong GBC_subframe_cycles(IntPtr core);


		[UnmanagedFunctionPointer(cc)]
		public delegate byte ControllerReadCallback(bool read_4016);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setcontrollercallback(IntPtr core, ControllerReadCallback callback);


		[UnmanagedFunctionPointer(cc)]
		public delegate void ControllerStrobeCallback(byte latched_4016, byte new_val);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setstrobecallback(IntPtr core, ControllerStrobeCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_get_video(IntPtr core, int[] videobuf);


		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBC_get_audio(IntPtr core, int[] aud_buf_L, ref uint n_samp_L, int[] aud_buf_R, ref uint n_samp_R);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_save_state(IntPtr core, byte[] saver);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_load_state(IntPtr core, byte[] loader);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getregisters(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getsysbus(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_gethram(IntPtr core, int addr, bool onvbl);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getvram(IntPtr core, int addr, bool onvbl);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getram(IntPtr core, int addr, bool onvbl);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getrom(IntPtr core, int addr, bool onvbl);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getoam(IntPtr core, int addr, bool onvbl);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getsram(IntPtr core, int addr, bool onvbl);


		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_settracecallback(IntPtr core, TraceCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_getheaderlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_getdisasmlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_getregstringlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getheader(IntPtr core, StringBuilder h, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getregisterstate(IntPtr core, StringBuilder h, int t, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getdisassembly(IntPtr core, StringBuilder h, int t, int l);


		[UnmanagedFunctionPointer(cc)]
		public delegate void MessageCallback(int str_length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setmessagecallback(IntPtr core, MessageCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getmessage(IntPtr core, StringBuilder h);


		[UnmanagedFunctionPointer(cc)]
		public delegate void InputPollCallback();


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setinputpollcallback(IntPtr core, InputPollCallback callback);

		#region PPU_Viewer

		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBC_get_ppu_pntrs(IntPtr core, int sel);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setscanlinecallback(IntPtr core, ScanlineCallback callback, int sl);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_executescanlinecallback(IntPtr core);

		#endregion
	}
}
