using System.IO;
using System.Reflection;

using BizHawk.Common.StringExtensions;

namespace BizHawk.Common
{
	public static partial class VersionInfo
	{
		/// <remarks>
		/// Bump this immediately after release.
		/// Only use '0'..'9' and '.' or it will fail to parse and the new version notification won't work.
		/// </remarks>
		public static readonly string MainVersion = "2.3.0";

		public static string GetEmuVersion()
			=> $"Version {MainVersion}";

		/// <summary>"2.5.1" => 0x02050100</summary>
		public static uint VersionStrToInt(string s)
		{
			var a = s.Split('.');
			var v = 0U;
			var i = 0;
			while (i < 4)
			{
				v <<= 8;
				v += i < a.Length && byte.TryParse(a[i], out var b) ? b : 0U;
				i++;
			}
			return v;
		}
	}
}
