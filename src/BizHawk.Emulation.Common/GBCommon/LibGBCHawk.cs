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

		/// <summary>
		/// Free the memory.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="bios">the BIOS data, can be disposed of once this function returns</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_load_bios(IntPtr core, byte[] bios);

		/// <summary>
		/// Load ROM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="romdata">the rom data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_load(IntPtr core, byte[] romdata, uint length);

		/// <summary>
		/// Set cart RTC.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="rtcdata">the rom data, can be disposed of once this function returns</param>
		/// <param name="index">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_set_rtc(IntPtr core, byte rtcdata, uint index);

		/// <summary>
		/// Create SRAM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_create_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Load SRAM.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_load_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Load palette, always 512 bytes.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="palette">the sram data, can be disposed of once this function returns</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_load_Palette(IntPtr core, int[] palette);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_Hard_Reset(IntPtr core);

		/// <summary>
		/// Soft Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_Soft_Reset(IntPtr core);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="render">length of romdata in bytes</param>
		/// <param name="sound">Mapper number to load core with</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBC_frame_advance(IntPtr core, bool render, bool sound);

		/// <summary>
		/// get cpu cycles since last reset
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong GBC_get_cycles(IntPtr core);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="render">length of romdata in bytes</param>
		/// <param name="sound">Mapper number to load core with</param>
		/// <param name="do_reset">length of romdata in bytes</param>
		/// <param name="reset_cycle">Mapper number to load core with</param>
		/// <returns>true if a frame was rendered or a reset occured, false if a poll occured.</returns>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBC_subframe_advance(IntPtr core, bool render, bool sound, bool do_reset, uint reset_cycle);

		/// <summary>
		/// get subframe cycles
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong GBC_subframe_cycles(IntPtr core);

		/// <summary>
		/// read the controller
		/// </summary>
		[UnmanagedFunctionPointer(cc)]
		public delegate byte ControllerReadCallback(bool read_4016);

		/// <summary>
		/// set a callback for reading the controller
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setcontrollercallback(IntPtr core, ControllerReadCallback callback);

		/// <summary>
		/// strobe the controller
		/// </summary>
		[UnmanagedFunctionPointer(cc)]
		public delegate void ControllerStrobeCallback(byte latched_4016, byte new_val);

		/// <summary>
		/// set a callback for strobing the controller
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setstrobecallback(IntPtr core, ControllerStrobeCallback callback);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="videobuf">where to send video to</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_get_video(IntPtr core, int[] videobuf);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="aud_buf">where to send left audio to</param>
		/// <param name="n_samp">number of left samples</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBC_get_audio(IntPtr core, int[] aud_buf, ref uint n_samp);

		/// <summary>
		/// Save State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="saver">save buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_save_state(IntPtr core, byte[] saver);

		/// <summary>
		/// Load State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="loader">load buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_load_state(IntPtr core, byte[] loader);

		/// <summary>
		/// Read the system bus
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">system bus address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getsysbus(IntPtr core, int addr);

		/// <summary>
		/// Read the VRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getvram(IntPtr core, int addr);

		/// <summary>
		/// Read the VROM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getchrrom(IntPtr core, int addr);

		/// <summary>
		/// Read the WRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getram(IntPtr core, int addr);

		/// <summary>
		/// Read the ROM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getrom(IntPtr core, int addr);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getoam(IntPtr core, int addr);

		/// <summary>
		/// Read the Palette Ram
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getpalram(IntPtr core, int addr);

		/// <summary>
		/// Read the SRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_getsram(IntPtr core, int addr);

		/// <summary>
		/// type of the cpu trace callback
		/// </summary>
		/// <param name="t">type of event</param>
		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);

		/// <summary>
		/// set a callback for trace logging
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_settracecallback(IntPtr core, TraceCallback callback);

		/// <summary>
		/// get the trace logger header length
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_getheaderlength(IntPtr core);

		/// <summary>
		/// get the trace logger disassembly length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_getdisasmlength(IntPtr core);

		/// <summary>
		/// get the trace logger register string length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBC_getregstringlength(IntPtr core);

		/// <summary>
		/// get the trace logger header
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getheader(IntPtr core, StringBuilder h, int l);

		/// <summary>
		/// get the register state from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getregisterstate(IntPtr core, StringBuilder h, int t, int l);

		/// <summary>
		/// get the disassembly from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getdisassembly(IntPtr core, StringBuilder h, int t, int l);

		/// <summary>
		/// get a message from the cpu
		/// </summary>
		[UnmanagedFunctionPointer(cc)]
		public delegate void MessageCallback(int str_length);

		/// <summary>
		/// set a callback for messages
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setmessagecallback(IntPtr core, MessageCallback callback);

		/// <summary>
		/// get a message from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_getmessage(IntPtr core, StringBuilder h);

		/// <summary>
		/// get a message from the cpu
		/// </summary>
		[UnmanagedFunctionPointer(cc)]
		public delegate void InputPollCallback();

		/// <summary>
		/// set a callback for messages
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setinputpollcallback(IntPtr core, InputPollCallback callback);

		#region PPU_Viewer

		/// <summary>
		/// Get variousa ppu states from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sel">value to get</param>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBC_get_ppu_vals(IntPtr core, int sel);

		/// <summary>
		/// Get variousa ppu states from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ppu address to peek</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_get_ppu_bus(IntPtr core, uint addr);

		/// <summary>
		/// Get ppu bus as seen by the board
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ppu address to peek</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBC_get_board_peek_ppu(IntPtr core, uint addr);

		/// <summary>
		/// Get PPU Pointers
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sel">region to get</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBC_get_ppu_pntrs(IntPtr core, int sel);

		/// <summary>
		/// callback for nametable viewer
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		/// <param name="sl">0-153 inclusive</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setntvcallback(IntPtr core, Action callback, int sl);

		/// <summary>
		/// callback for ppu viewer
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		/// <param name="sl">0-153 inclusive</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBC_setppucallback(IntPtr core, Action callback, int sl);

		#endregion
	}
}
