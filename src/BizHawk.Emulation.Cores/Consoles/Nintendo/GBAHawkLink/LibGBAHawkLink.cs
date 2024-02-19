using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBALink
{
	/// <summary>
	/// static bindings into GBAHawk.dll
	/// </summary>
	public static class LibGBAHawkLink
	{
		private const string lib = "GBAHawk";
		private const CallingConvention cc = CallingConvention.Cdecl;

		/// <returns>opaque state pointer</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern IntPtr GBALink_create();

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_destroy(IntPtr core);

		/// <summary>
		/// Load BIOS and BASIC image. each must be 16K in size
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="bios">the BIOS data, can be disposed of once this function returns</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_load_bios(IntPtr core, byte[] bios);

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
		public static extern int GBALink_load(IntPtr core, byte[] romdata0, uint length0, int mapper0,
															byte[] romdata1, uint length1, int mapper1,
															ulong datetime0, bool rtc_functional0,
															ulong datetime1, bool rtc_functional1,
															short EEPROMoffset0, short EEPROMoffset1);

		/// <summary>
		/// Create SRAM image.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <param name="core_num">which core to load SRAM to</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_create_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);

		/// <summary>
		/// Load SRAM.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="sram_data">the sram data, can be disposed of once this function returns</param>
		/// <param name="length">length of romdata in bytes</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_load_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);

		/// <summary>
		/// Hard Reset.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_Hard_Reset(IntPtr core);

		/// <summary>
		/// Advance a frame and send controller data.
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="ctrl0">controller data for player 1</param>
		/// <param name="accx0">x acceleration</param>
		/// <param name="accy0">y acceleration</param>
		/// <param name="solar0">solar state</param>
		/// <param name="render0">length of romdata in bytes</param>
		/// <param name="sound0">Mapper number to load core with</param>
		/// <param name="ctrl1">controller data for player 1</param>
		/// <param name="accx1">x acceleration</param>
		/// <param name="accy1">y acceleration</param>
		/// <param name="solar1">solar state</param>
		/// <param name="render1">length of romdata in bytes</param>
		/// <param name="sound1">Mapper number to load core with</param>
		/// <param name="l_reset">left console reset</param>
		/// <param name="r_reset">right console reset</param>
		/// <returns>0 on success, negative value on failure.</returns>
		[DllImport(lib, CallingConvention = cc)]
		public static extern bool GBALink_frame_advance(IntPtr core, ushort ctrl0, ushort accx0, ushort accy0, byte solar0, bool render0, bool sound0,
																	 ushort ctrl1, ushort accx1, ushort accy1, byte solar1, bool render1, bool sound1,
																	 bool l_reset, bool r_reset);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="videobuf">where to send video to</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_get_video(IntPtr core, int[] videobuf, uint core_num);

		/// <summary>
		/// Get Video data
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="aud_buf_L">where to send left audio to</param>
		/// <param name="n_samp_L">number of left samples</param>
		/// <param name="aud_buf_R">where to send right audio to</param>
		/// <param name="n_samp_R">number of right samples</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBALink_get_audio(IntPtr core, int[] aud_buf_L, ref uint n_samp_L, int[] aud_buf_R, ref uint n_samp_R, uint core_num);

		/// <summary>
		/// Save State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="saver">save buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_save_state(IntPtr core, byte[] saver);

		/// <summary>
		/// Load State
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="loader">load buffer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_load_state(IntPtr core, byte[] loader);

		/// <summary>
		/// Read the system bus
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">system bus address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getsysbus(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the VRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getvram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getoam(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the OAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">vram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getpalram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the WRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getwram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the IWRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getiwram(IntPtr core, int addr, uint core_num);

		/// <summary>
		/// Read the SRAM
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="addr">ram address</param>
		/// <param name="core_num">ram address</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getsram(IntPtr core, int addr, int core_num);

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
		public static extern void GBALink_settracecallback(IntPtr core, TraceCallback callback, uint core_num);

		/// <summary>
		/// get the trace logger header length
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getheaderlength(IntPtr core);

		/// <summary>
		/// get the trace logger disassembly length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getdisasmlength(IntPtr core);

		/// <summary>
		/// get the trace logger register string length, a constant
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getregstringlength(IntPtr core);

		/// <summary>
		/// get the trace logger header
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getheader(IntPtr core, StringBuilder h, int l);

		/// <summary>
		/// get the register state from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getregisterstate(IntPtr core, StringBuilder h, int t, int l, uint core_num);

		/// <summary>
		/// get the disassembly from the cpu
		/// </summary>
		/// <param name="core">opaque state pointer</param>
		/// <param name="h">pointer to const char *</param>
		/// <param name="t">call type</param>
		/// <param name="l">copy length, must be obtained from appropriate get legnth function</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getdisassembly(IntPtr core, StringBuilder h, int t, int l, uint core_num);
	}
}
