using System.Collections.Generic;

namespace BizHawk.Client.Common
{
	public static class PlatformFrameRates
	{
		private static readonly Dictionary<string, double> Rates = new Dictionary<string, double>
		{
			["NES"] = 60.098813897440515532, // discussion here: http://forums.nesdev.com/viewtopic.php?t=492 ; a rational expression would be (19687500 / 11) / ((341*262-0.529780.5)/3) -> (118125000 / 1965513) -> 60.098813897440515529533511098629 (so our chosen number is very close)
			["FDS"] = 60.098813897440515532,

			["GB"] = 262144.0 / 4389.0, // 59.7275005696
			["GBC"] = 262144.0 / 4389.0, // 59.7275005696

			["GBA"] = 262144.0 / 4389.0, // 59.7275005696

		};

		public static double GetFrameRate(string systemId, bool pal)
			=> Rates.TryGetValue(systemId + (pal ? "_PAL" : ""), out var d) ? d : 60.0;
	}
}
