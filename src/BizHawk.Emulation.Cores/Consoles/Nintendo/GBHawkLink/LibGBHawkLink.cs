using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBLink
{
	/// <summary>
	/// static bindings into GBHawk.dll
	/// </summary>
	public static class LibGBHawkLink
	{
		private const string lib = "GBHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBLink_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_destroy(IntPtr core);

		/// <summary>
		/// Free the memory.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="bios">the BIOS data, can be disposed of once this function returns</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_load_bios(IntPtr core, byte[] bios);

		/// <summary>
		/// Load ROM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="romdata0">the rom data, can be disposed of once this function returns</param>
		/// <param name="length0">length of romdata in bytes</param>
		/// <param name="mapper0">Mapper number to load core with</param>
		/// <param name="romdata1">the rom data, can be disposed of once this function returns</param>
		/// <param name="length1">length of romdata in bytes</param>
		/// <param name="mapper1">Mapper number to load core with</param>
		/// <param name="datetime0">rtc initial time</param>
		/// <param name="rtc_functional0">rtc setting</param>
		/// <param name="datetime1">rtc initial time</param>
		/// <param name="rtc_functional1">rtc setting</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_load(IntPtr core, byte[] romdata0, uint length0, int mapper0,
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

		/// <summary>
		/// Create SRAM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <param name="core_num">which core to load SRAM to</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_create_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);

		/// <summary>
		/// Load SRAM.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_load_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_Hard_Reset(IntPtr core);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_Set_GBP_Enable(IntPtr core, uint core_num);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="ctrl0">controller data for player 1</param>
		/// <param name="accx0">x acceleration</param>
		/// <param name="accy0">y acceleration</param>
		/// <param name="render0">length of romdata in bytes</param>
		/// <param name="sound0">Mapper number to load core with</param>
		/// <param name="ctrl1">controller data for player 1</param>
		/// <param name="accx1">x acceleration</param>
		/// <param name="accy1">y acceleration</param>
		/// <param name="render1">length of romdata in bytes</param>
		/// <param name="sound1">Mapper number to load core with</param>
		/// <param name="l_reset">left console reset</param>
		/// <param name="r_reset">right console reset</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern bool GBLink_frame_advance(IntPtr core, ushort ctrl0, ushort accx0, ushort accy0, bool render0, bool sound0,
																	 ushort ctrl1, ushort accx1, ushort accy1, bool render1, bool sound1,
																	 bool l_reset, bool r_reset);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="videobuf">where to send video to</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_get_video(IntPtr core, int[] videobuf, uint core_num);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="aud_buf_L">where to send left audio to</param>
		/// <param name="n_samp_L">number of left samples</param>
		/// <param name="aud_buf_R">where to send right audio to</param>
		/// <param name="n_samp_R">number of right samples</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBLink_get_audio(IntPtr core, int[] aud_buf_L, ref uint n_samp_L, int[] aud_buf_R, ref uint n_samp_R, uint core_num);

		/// <summary>
		/// Save State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="saver">save buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_save_state(IntPtr core, byte[] saver);

		/// <summary>
		/// Load State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="loader">load buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_load_state(IntPtr core, byte[] loader);

		/// <summary>
		/// Read the system bus
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">system bus address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getsysbus(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the VRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getvram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getoam(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getpalram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the WRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getwram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the IWRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getiwram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the SRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		/// <param name="core_num">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBLink_getsram(IntPtr core, int addr, int core_num);

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
		public static extern void GBLink_settracecallback(IntPtr core, TraceCallback callback, uint core_num);

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
		public static extern void GBA_setrumblecallback(IntPtr core, RumbleCallback callback, int num);

		/// <summary>
		/// get the trace logger header length
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_getheaderlength(IntPtr core);

		/// <summary>
		/// get the trace logger disassembly length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_getdisasmlength(IntPtr core);

		/// <summary>
		/// get the trace logger register string length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBLink_getregstringlength(IntPtr core);

		/// <summary>
		/// get the trace logger header
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_getheader(IntPtr core, StringBuilder h, int l);

		/// <summary>
		/// get the register state from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_getregisterstate(IntPtr core, StringBuilder h, int t, int l, uint core_num);

		/// <summary>
		/// get the disassembly from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBLink_getdisassembly(IntPtr core, StringBuilder h, int t, int l, uint core_num);
	}
}
