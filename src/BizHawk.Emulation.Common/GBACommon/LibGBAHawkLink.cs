using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BizHawk.Emulation.Cores.Nintendo.GBA.Common
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
		public static extern IntPtr GBALink_create(uint num_roms);

		/// <param name="core">opaque state pointer</param>
		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_destroy(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_load_bios(IntPtr core, byte[] bios);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_load(IntPtr core, byte[] romdata, uint length, int mapper,
															ulong datetime, bool rtc_functional, short EEPROMoffset,
															ushort FlashType64Value, ushort FlashType128Value,
															short FlashWriteOffset, int FlashSectorOffset,
															int FlashChipOffset, bool is_GBP);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_load_video(IntPtr core, byte[] romdata, uint core_num);

		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_create_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_load_SRAM(IntPtr core, byte[] sram_data, uint length, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_Hard_Reset(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_Set_GBP_Enable(IntPtr core, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		[return: MarshalAs(UnmanagedType.I1)]
		public static extern bool GBALink_frame_advance(IntPtr core, ushort[] ctrls, ushort[] accxs, ushort[] accys, byte[] solar_states,
																	 bool[] renders, bool[] sounds, bool[] resets);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_get_video(IntPtr core, int[] videobuf, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern uint GBALink_get_audio(IntPtr core, int[] aud_buf_L_0, ref uint n_samp_L_0, int[] aud_buf_R_0, ref uint n_samp_R_0,
																 int[] aud_buf_L_1, ref uint n_samp_L_1, int[] aud_buf_R_1, ref uint n_samp_R_1, bool[] audio_enables);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_save_state(IntPtr core, byte[] saver);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_load_state(IntPtr core, byte[] loader);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getsysbus(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getvram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getoam(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getpalram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getwram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getiwram(IntPtr core, int addr, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern byte GBALink_getsram(IntPtr core, int addr, int core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void TraceCallback(int t);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_settracecallback(IntPtr core, TraceCallback callback, uint core_num);


		[UnmanagedFunctionPointer(cc)]
		public delegate void RumbleCallback(bool rumble_on);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBA_setrumblecallback(IntPtr core, RumbleCallback callbackA,
																	 RumbleCallback callbackB,
																	 RumbleCallback callbackC,
																	 RumbleCallback callbackD);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getheaderlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getdisasmlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern int GBALink_getregstringlength(IntPtr core);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getheader(IntPtr core, StringBuilder h, int l);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getregisterstate(IntPtr core, StringBuilder h, int t, int l, uint core_num);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getdisassembly(IntPtr core, StringBuilder h, int t, int l, uint core_num);

		[UnmanagedFunctionPointer(cc)]
		public delegate void MessageCallback(int str_length);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_setmessagecallback(IntPtr core, MessageCallback callback_L, MessageCallback callback_R);


		[DllImport(lib, CallingConvention = cc)]
		public static extern void GBALink_getmessage(IntPtr core, StringBuilder h, int s);
	}
}
