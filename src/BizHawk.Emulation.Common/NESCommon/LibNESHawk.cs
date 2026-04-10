using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.NES.Common
{
	/// <summary>
	/// static bindings into NESHawk.dll
	/// </summary>
	public static class LibNESHawk
	{
		private const string lib = "NESHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr NES_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_destroy(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int NES_load(IntPtr core, byte[] romdata, uint length, byte[] headerdata, bool Old_MMC3_IRQ, bool bus_conflicts, bool apu_test_regs, bool cpu_zero, byte special_flag);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int NES_create_SRAM(IntPtr core, byte[] sram_data, uint length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int NES_load_SRAM(IntPtr core, byte[] sram_data, uint length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int NES_load_Palette(IntPtr core, int[] palette);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_Hard_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_Soft_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool NES_frame_advance(IntPtr core, bool render, bool sound);


		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong NES_get_cycles(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool NES_subframe_advance(IntPtr core, bool render, bool sound, bool do_reset, uint reset_cycle);


		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong NES_subframe_cycles(IntPtr core);


		[UnmanagedFunctionPointer(cc)]
		public delegate byte ControllerReadCallback(bool read_4016);

		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getzapperstate(IntPtr core, int zapper_x, int zapper_y);

		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_setcontrollercallback(IntPtr core, ControllerReadCallback callback);


		[UnmanagedFunctionPointer(cc)]
		public delegate void ControllerStrobeCallback(byte latched_4016, byte new_val);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_setstrobecallback(IntPtr core, ControllerStrobeCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_get_video(IntPtr core, int[] videobuf);


		[DllImport(lib, CallingConvention = cc)]
		public static extern uint NES_get_audio(IntPtr core, int[] aud_buf, ref uint n_samp);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_save_state(IntPtr core, byte[] saver);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_load_state(IntPtr core, byte[] loader);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getsysbus(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getvram(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getchrrom(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getram(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getrom(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getoam(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getpalram(IntPtr core, int addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_getsram(IntPtr core, int addr);


		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_settracecallback(IntPtr core, TraceCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int NES_getheaderlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int NES_getdisasmlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int NES_getregstringlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_getheader(IntPtr core, StringBuilder h, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_getregisterstate(IntPtr core, StringBuilder h, int t, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_getdisassembly(IntPtr core, StringBuilder h, int t, int l);


		[UnmanagedFunctionPointer(cc)]
		public delegate void MessageCallback(int str_length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_setmessagecallback(IntPtr core, MessageCallback callback);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_getmessage(IntPtr core, StringBuilder h);


		[UnmanagedFunctionPointer(cc)]
		public delegate void InputPollCallback();


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_setinputpollcallback(IntPtr core, InputPollCallback callback);

		#region PPU_Viewer

		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool NES_get_ppu_vals(IntPtr core, int sel);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_get_ppu_bus(IntPtr core, uint addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte NES_get_board_peek_ppu(IntPtr core, uint addr);


		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr NES_get_ppu_pntrs(IntPtr core, int sel);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_setntvcallback(IntPtr core, Action callback, int sl);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void NES_setppucallback(IntPtr core, Action callback, int sl);

		#endregion
	}
}
