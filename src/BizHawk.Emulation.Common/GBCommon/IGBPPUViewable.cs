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
		bool IsCGBMode();

		/// <summary>
		/// Acquire GPU memory for inspection.  The returned object must be disposed as soon as the frontend
		/// tool is done inspecting it, and the pointers become invalid once it is disposed.
		/// </summary>
		/// <returns></returns>
		IGBGPUMemoryAreas LockGPU();

		/// <summary>
		/// set up callback
		/// </summary>
		/// <param name="line">scanline. -1 = end of frame, -2 = RIGHT NOW</param>
		void SetScanlineCallback(ScanlineCallback callback, int line);
	}

	public interface IGBGPUMemoryAreas : IDisposable
	{
		IntPtr Vram { get; }
		IntPtr Oam { get; }
		IntPtr Sppal { get; }
		IntPtr Bgpal { get; }
	}
}
