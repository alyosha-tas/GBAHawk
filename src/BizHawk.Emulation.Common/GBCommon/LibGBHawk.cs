using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GB.Common
{
	/// <summary>
	/// static bindings into GBHawk.dll
	/// </summary>
	public static class LibGBHawk
	{
		private const string lib = "GBHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GB_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_destroy(IntPtr core);

		/// <summary>
		/// Free the memory.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="bios">the BIOS data, can be disposed of once this function returns</param>
		/// <param name="gbcflag">whether or not console is gbc</param>
		/// <param name="gbc_gba_flag">whether or not console is gba</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_load_bios(IntPtr core, byte[] bios, bool gbcflag, bool gbc_gba_flag);

		/// <summary>
		/// Load ROM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="romdata">the rom data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <param name="mapper_number">mapper id</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_load(IntPtr core, byte[] romdata, uint length, uint mapper_number);

		/// <summary>
		/// Set cart RTC.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="rtcdata">the rom data, can be disposed of once this function returns</param>
		/// <param name="index">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_set_rtc(IntPtr core, int rtcdata, uint index);

		/// <summary>
		/// Create SRAM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_create_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Load SRAM.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_load_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Set GB palette
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="palette"> true for BW, false for Gr</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_load_Palette(IntPtr core, bool palette);

		/// <summary>
		/// when to sync memory domains
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="on_VBL"> true for vbl, false for frame boundary</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_Sync_Domain_VBL(IntPtr core, bool on_VBL);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_Hard_Reset(IntPtr core);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="render">length of romdata in bytes</param>
		/// <param name="sound">Mapper number to load core with</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GB_frame_advance(IntPtr core, byte ctrl1, ushort accx, ushort accy, bool render, bool sound);

		/// <summary>
		/// get cpu cycles since last reset
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong GB_get_cycles(IntPtr core);

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
		public static extern bool GB_subframe_advance(IntPtr core, bool render, bool sound, bool do_reset, uint reset_cycle);

		/// <summary>
		/// get subframe cycles
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong GB_subframe_cycles(IntPtr core);

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
		public static extern void GB_setcontrollercallback(IntPtr core, ControllerReadCallback callback);

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
		public static extern void GB_setstrobecallback(IntPtr core, ControllerStrobeCallback callback);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="videobuf">where to send video to</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_get_video(IntPtr core, int[] videobuf);

		/// <summary>
		/// Get audio data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="aud_buf_L">where to send left audio to</param>
		/// <param name="n_samp_L">number of left samples</param>
		/// <param name="aud_buf_R">where to send right audio to</param>
		/// <param name="n_samp_R">number of right samples</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GB_get_audio(IntPtr core, int[] aud_buf_L, ref uint n_samp_L, int[] aud_buf_R, ref uint n_samp_R);


		/// <summary>
		/// Save State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="saver">save buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_save_state(IntPtr core, byte[] saver);

		/// <summary>
		/// Load State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="loader">load buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_load_state(IntPtr core, byte[] loader);

		/// <summary>
		/// Read the registers
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">system bus address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getregisters(IntPtr core, int addr);

		/// <summary>
		/// Read the system bus
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">system bus address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getsysbus(IntPtr core, int addr);

		/// <summary>
		/// Read the HRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		/// <param name = "onvbl" > get new values only on vbl</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_gethram(IntPtr core, int addr, bool onvbl);

		/// <summary>
		/// Read the VRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		/// <param name = "onvbl" > get new values only on vbl</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getvram(IntPtr core, int addr, bool onvbl);

		/// <summary>
		/// Read the RAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		/// <param name = "onvbl" > get new values only on vbl</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getram(IntPtr core, int addr, bool onvbl);

		/// <summary>
		/// Read the ROM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		/// <param name = "onvbl" > get new values only on vbl</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getrom(IntPtr core, int addr, bool onvbl);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		/// <param name = "onvbl" > get new values only on vbl</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getoam(IntPtr core, int addr, bool onvbl);

		/// <summary>
		/// Read the Palette Ram
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		/// <param name = "onvbl" > get new values only on vbl</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getpalram(IntPtr core, int addr, bool onvbl);

		/// <summary>
		/// Read the SRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		/// <param name = "onvbl" > get new values only on vbl</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GB_getsram(IntPtr core, int addr, bool onvbl);

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
		public static extern void GB_settracecallback(IntPtr core, TraceCallback callback);

		/// <summary>
		/// get the trace logger header length
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_getheaderlength(IntPtr core);

		/// <summary>
		/// get the trace logger disassembly length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_getdisasmlength(IntPtr core);

		/// <summary>
		/// get the trace logger register string length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GB_getregstringlength(IntPtr core);

		/// <summary>
		/// get the trace logger header
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_getheader(IntPtr core, StringBuilder h, int l);

		/// <summary>
		/// get the register state from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_getregisterstate(IntPtr core, StringBuilder h, int t, int l);

		/// <summary>
		/// get the disassembly from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_getdisassembly(IntPtr core, StringBuilder h, int t, int l);

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
		public static extern void GB_setmessagecallback(IntPtr core, MessageCallback callback);

		/// <summary>
		/// get a message from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_getmessage(IntPtr core, StringBuilder h);

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
		public static extern void GB_setinputpollcallback(IntPtr core, InputPollCallback callback);

		#region PPU_Viewer

		/// <summary>
		/// Get PPU Pointers
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sel">region to get</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GB_get_ppu_pntrs(IntPtr core, int sel);

		/// <summary>
		/// set a callback to occur when ly reaches a particular scanline (so at the beginning of the scanline).
		/// when the LCD is active, typically 145 will be the first callback after the beginning of frame advance,
		/// and 144 will be the last callback right before frame advance returns
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		/// <param name="sl">0-153 inclusive</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GB_setscanlinecallback(IntPtr core, ScanlineCallback callback, int sl);

		#endregion
	}
}
