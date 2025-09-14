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

		/// <summary>
		/// Free the memory.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="bios">the BIOS data, can be disposed of once this function returns</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_load_bios(IntPtr core, byte[] bios);

		/// <summary>
		/// Load ROM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="romdata">the rom data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <param name="mapper">Mapper number to load core with</param>
		/// <param name="datetime">rtc initial time</param>
		/// <param name="rtc_functional">rtc setting</param>
		/// <param name="EEPROMoffset"></param>
		/// <param name="is_GBP">playing on GBP</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_load(IntPtr core, byte[] romdata, uint length, int mapper, ulong datetime, bool rtc_functional, 
										  short EEPROMoffset, ushort FlashType64Value, ushort FlashType128Value, short FlashWriteOffset, int FlashSectorOffset, int FlashChipOffset, bool is_GBP);

		/// <summary>
		/// Create SRAM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_create_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Load SRAM.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_load_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_Hard_Reset(IntPtr core);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_Set_GBP_Enable(IntPtr core);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="ctrl1">controller data for player 1</param>
		/// <param name="accx">x acceleration</param>
		/// <param name="accy">y acceleration</param>
		/// <param name="solar">solar state</param>
		/// <param name="render">length of romdata in bytes</param>
		/// <param name="sound">Mapper number to load core with</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern bool GBA_frame_advance(IntPtr core, ushort ctrl1, ushort accx, ushort accy, byte solar, bool render, bool sound);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="ctrl1">controller data for player 1</param>
		/// <param name="accx">x acceleration</param>
		/// <param name="accy">y acceleration</param>
		/// <param name="solar">solar state</param>
		/// <param name="render">length of romdata in bytes</param>
		/// <param name="sound">Mapper number to load core with</param>
		/// <param name="do_reset">length of romdata in bytes</param>
		/// <param name="reset_cycle">Mapper number to load core with</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBA_subframe_advance(IntPtr core, ushort ctrl1, ushort accx, ushort accy, byte solar, bool render, bool sound, bool do_reset, uint reset_cycle);


		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="videobuf">where to send video to</param>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern void GBA_get_video(IntPtr core, int[] videobuf);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="aud_buf_L">where to send left audio to</param>
		/// <param name="n_samp_L">number of left samples</param>
		/// <param name="aud_buf_R">where to send right audio to</param>
		/// <param name="n_samp_R">number of right samples</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBA_get_audio(IntPtr core, int[] aud_buf_L, ref uint n_samp_L, int[] aud_buf_R, ref uint n_samp_R);

		/// <summary>
		/// Save State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="saver">save buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_save_state(IntPtr core, byte[] saver);

		/// <summary>
		/// Load State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="loader">load buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_load_state(IntPtr core, byte[] loader);

		/// <summary>
		/// Read the system bus
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">system bus address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getsysbus(IntPtr core, int addr);

		/// <summary>
		/// Read the VRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getvram(IntPtr core, int addr);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getoam(IntPtr core, int addr);

		/// <summary>
		/// Read the Palette Ram
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getpalram(IntPtr core, int addr);

		/// <summary>
		/// Read the registers
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getregisters(IntPtr core, int addr);

		/// <summary>
		/// Read the WRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getwram(IntPtr core, int addr);

		/// <summary>
		/// Read the IWRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getiwram(IntPtr core, int addr);

		/// <summary>
		/// Read the SRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBA_getsram(IntPtr core, int addr);

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
		public static extern void GBA_settracecallback(IntPtr core, TraceCallback callback);

		/// <summary>
		/// rumble callback
		/// </summary>
		[UnmanagedFunctionPointer(cc)]
		public delegate void RumbleCallback(bool rumble_on);

		/// <summary>
		/// set a callback for rumble
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_setrumblecallback(IntPtr core, RumbleCallback callback);

		/// <summary>
		/// get the trace logger header length
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_getheaderlength(IntPtr core);

		/// <summary>
		/// get the trace logger disassembly length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_getdisasmlength(IntPtr core);

		/// <summary>
		/// get the trace logger register string length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBA_getregstringlength(IntPtr core);

		/// <summary>
		/// get the trace logger header
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getheader(IntPtr core, StringBuilder h, int l);

		/// <summary>
		/// get the register state from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getregisterstate(IntPtr core, StringBuilder h, int t, int l);

		/// <summary>
		/// get the disassembly from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getdisassembly(IntPtr core, StringBuilder h, int t, int l);

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
		public static extern void GBA_setmessagecallback(IntPtr core, MessageCallback callback);

		/// <summary>
		/// get a message from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_getmessage(IntPtr core, StringBuilder h);


		#region PPU_Viewer

		/// <summary>
		/// Get PPU Pointers
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sel">region to get</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBA_get_ppu_pntrs(IntPtr core, int sel);

		/// <summary>
		/// set a callback to occur when ly reaches a particular scanline (so at the beginning of the scanline).
		/// when the LCD is active, typically 145 will be the first callback after the beginning of frame advance,
		/// and 144 will be the last callback right before frame advance returns
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		/// <param name="sl">0-153 inclusive</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_setscanlinecallback(IntPtr core, Action callback, int sl);

		#endregion
	}
}
