using System;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GB.Common
{
	/// <param name="lcdc">current value of register $ff40 (LCDC)</param>
	public delegate void ScanlineCallback(byte lcdc);


	/// <summary>
	/// supports the PPU and NT viewers.  do not modify any returned arrays!
	/// </summary>
	public interface IGBPPUViewable : IEmulatorService
	{
		public bool IsCGBMode();

		public bool IsCGBDMGMode();

		/// <summary>
		/// get the 32 byte BG palette ram
		/// </summary>
		IntPtr GetBGPalRam();


		/// <summary>
		/// get the 32 byte Sprite palette ram
		/// </summary>
		IntPtr GetSPRPalRam();

		/// <summary>
		/// returns the object attribute memory
		/// </summary>
		IntPtr GetOAM();

		/// <summary>
		/// returns the VRAM
		/// </summary>
		IntPtr GetVRAM();

		/// <summary>
		/// set up callback
		/// </summary>
		/// <param name="line">scanline. -1 = end of frame, -2 = RIGHT NOW</param>
		void SetScanlineCallback(ScanlineCallback callback, int line);
	}
}
