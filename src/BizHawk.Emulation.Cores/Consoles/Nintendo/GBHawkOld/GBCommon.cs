using System;

using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Consoles.Nintendo.Gameboy
{
	/// <param name="lcdc">current value of register $ff40 (LCDC)</param>
	public delegate void ScanlineCallback(byte lcdc);

	/// <param name="image">The image data</param>
	/// <param name="height">How tall an image is, in pixels. Image is only valid up to that height and must be assumed to be garbage below that.</param>
	/// <param name="top_margin">The top margin of blank pixels. Just form feeds the printer a certain amount at the top.</param>
	/// <param name="bottom_margin">The bottom margin of blank pixels. Just form feeds the printer a certain amount at the bottom.</param>
	/// <param name="exposure">The darkness/intensity of the print job. What the exact values mean is somewhat subjective but 127 is the most exposed/darkest value.</param>
	public delegate void PrinterCallback(IntPtr image, byte height, byte top_margin, byte bottom_margin, byte exposure);

	public interface IGameboyCommon : ISpecializedEmulatorService
	{


		bool IsCGBDMGMode();




	}


}
