using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.SNES.Common
{
	/// <summary>
	/// static bindings into SNESHawk.dll
	/// </summary>
	public static class LibSNESHawk
	{
		private const string lib = "SNESHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr SNES_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_destroy(IntPtr core);

		/// <summary>
		/// Load ROM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="romdata">the rom data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <param name="headerdata">header</param>
		/// <param name="bus_conflicts">mapper bus conflicts</param>
		/// <param name="apu_test_regs">activate apu test regs</param>
		/// <param name="cpu_zero">set cpu zero flag at reset</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int SNES_load(IntPtr core, byte[] romdata, uint length, byte[] headerdata, bool bus_conflicts, bool apu_test_regs, bool cpu_zero);

		/// <summary>
		/// Create SRAM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int SNES_create_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Load SRAM.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int SNES_load_SRAM(IntPtr core, byte[] sram_data, uint length);

		/// <summary>
		/// Load palette, always 512 bytes.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="palette">the sram data, can be disposed of once this function returns</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int SNES_load_Palette(IntPtr core, int[] palette);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_Hard_Reset(IntPtr core);

		/// <summary>
		/// Soft Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_Soft_Reset(IntPtr core);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="render">length of romdata in bytes</param>
		/// <param name="sound">Mapper number to load core with</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool SNES_frame_advance(IntPtr core, bool render, bool sound);

		/// <summary>
		/// get cpu cycles since last reset
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong SNES_get_cycles(IntPtr core);

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
		public static extern bool SNES_subframe_advance(IntPtr core, bool render, bool sound, bool do_reset, uint reset_cycle);

		/// <summary>
		/// get subframe cycles
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern ulong SNES_subframe_cycles(IntPtr core);

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
		public static extern void SNES_setcontrollercallback(IntPtr core, ControllerReadCallback callback);

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
		public static extern void SNES_setstrobecallback(IntPtr core, ControllerStrobeCallback callback);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="videobuf">where to send video to</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_get_video(IntPtr core, int[] videobuf);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="aud_buf">where to send left audio to</param>
		/// <param name="n_samp">number of left samples</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern uint SNES_get_audio(IntPtr core, int[] aud_buf, ref uint n_samp);

		/// <summary>
		/// Save State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="saver">save buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_save_state(IntPtr core, byte[] saver);

		/// <summary>
		/// Load State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="loader">load buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_load_state(IntPtr core, byte[] loader);

		/// <summary>
		/// Read the system bus
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">system bus address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getsysbus(IntPtr core, int addr);

		/// <summary>
		/// Read the VRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getvram(IntPtr core, int addr);

		/// <summary>
		/// Read the VROM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getchrrom(IntPtr core, int addr);

		/// <summary>
		/// Read the WRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getram(IntPtr core, int addr);

		/// <summary>
		/// Read the ROM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getrom(IntPtr core, int addr);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getoam(IntPtr core, int addr);

		/// <summary>
		/// Read the Palette Ram
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getpalram(IntPtr core, int addr);

		/// <summary>
		/// Read the SRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_getsram(IntPtr core, int addr);

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
		public static extern void SNES_settracecallback(IntPtr core, TraceCallback callback);

		/// <summary>
		/// get the trace logger header length
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int SNES_getheaderlength(IntPtr core);

		/// <summary>
		/// get the trace logger disassembly length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int SNES_getdisasmlength(IntPtr core);

		/// <summary>
		/// get the trace logger register string length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int SNES_getregstringlength(IntPtr core);

		/// <summary>
		/// get the trace logger header
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_getheader(IntPtr core, StringBuilder h, int l);

		/// <summary>
		/// get the register state from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_getregisterstate(IntPtr core, StringBuilder h, int t, int l);

		/// <summary>
		/// get the disassembly from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_getdisassembly(IntPtr core, StringBuilder h, int t, int l);

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
		public static extern void SNES_setmessagecallback(IntPtr core, MessageCallback callback);

		/// <summary>
		/// get a message from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_getmessage(IntPtr core, StringBuilder h);

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
		public static extern void SNES_setinputpollcallback(IntPtr core, InputPollCallback callback);

		#region PPU_Viewer

		/// <summary>
		/// Get variousa ppu states from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sel">value to get</param>
		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool SNES_get_ppu_vals(IntPtr core, int sel);

		/// <summary>
		/// Get variousa ppu states from the core
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ppu address to peek</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_get_ppu_bus(IntPtr core, uint addr);

		/// <summary>
		/// Get ppu bus as seen by the board
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ppu address to peek</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte SNES_get_board_peek_ppu(IntPtr core, uint addr);

		/// <summary>
		/// Get PPU Pointers
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sel">region to get</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr SNES_get_ppu_pntrs(IntPtr core, int sel);

		/// <summary>
		/// callback for nametable viewer
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		/// <param name="sl">0-153 inclusive</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_setntvcallback(IntPtr core, Action callback, int sl);

		/// <summary>
		/// callback for ppu viewer
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="callback">null to clear</param>
		/// <param name="sl">0-153 inclusive</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void SNES_setppucallback(IntPtr core, Action callback, int sl);

		#endregion
	}
}
