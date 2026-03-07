namespace BizHawk.Emulation.Cores.Nintendo.GB.Common
{
	public static class Palettes
	{
		private static readonly float[] rtmul = { 1.239f, 0.794f, 1.019f, 0.905f, 1.023f, 0.741f, 0.75f };
		private static readonly float[] gtmul = { 0.915f, 1.086f, 0.98f, 1.026f, 0.908f, 0.987f, 0.75f };
		private static readonly float[] btmul = { 0.743f, 0.882f, 0.653f, 1.277f, 0.979f, 0.101f, 0.75f };

		public static void ApplyDeemphasis(ref int r, ref int g, ref int b, int deemph_bits)
		{
			//DEEMPH BITS MAY BE ORDERED WRONG. PLEASE CHECK
			if (deemph_bits == 0) return;
			int d = deemph_bits - 1;
			r = (int)(r * rtmul[d]);
			g = (int)(g * gtmul[d]);
			b = (int)(b * btmul[d]);
			if (r > 0xFF) r = 0xFF;
			if (g > 0xFF) g = 0xFF;
			if (b > 0xFF) b = 0xFF;
		}

		public static readonly byte[,] QuickNESPalette =
		{
			{102, 102, 102},
			{0, 42, 136},
			{20, 18, 168},
			{59, 0, 164},
			{92, 0, 126},
			{110, 0, 64},
			{108, 7, 0},
			{87, 29, 0},
			{52, 53, 0},
			{12, 73, 0},
			{0, 82, 0},
			{0, 79, 8},
			{0, 64, 78},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{174, 174, 174},
			{21, 95, 218},
			{66, 64, 254},
			{118, 39, 255},
			{161, 27, 205},
			{184, 30, 124},
			{181, 50, 32},
			{153, 79, 0},
			{108, 110, 0},
			{56, 135, 0},
			{13, 148, 0},
			{0, 144, 50},
			{0, 124, 142},
			{0, 0, 0},
			{0, 0, 0},
			{0, 0, 0},
			{254, 254, 254},
			{100, 176, 254},
			{147, 144, 254},
			{199, 119, 254},
			{243, 106, 254},
			{254, 110, 205},
			{254, 130, 112},
			{235, 159, 35},
			{189, 191, 0},
			{137, 217, 0},
			{93, 229, 48},
			{69, 225, 130},
			{72, 206, 223},
			{79, 79, 79},
			{0, 0, 0},
			{0, 0, 0},
			{254, 254, 254},
			{193, 224, 254},
			{212, 211, 254},
			{233, 200, 254},
			{251, 195, 254},
			{254, 197, 235},
			{254, 205, 198},
			{247, 217, 166},
			{229, 230, 149},
			{208, 240, 151},
			{190, 245, 171},
			{180, 243, 205},
			{181, 236, 243},
			{184, 184, 184},
			{0, 0, 0},
			{0, 0, 0},
		};
	} //class palettes
} //namespace
