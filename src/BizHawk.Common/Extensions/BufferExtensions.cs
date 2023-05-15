using System;
using System.IO;
using System.Text;

namespace BizHawk.Common.BufferExtensions
{
	public static class BufferExtensions
	{
		/// <summary>
		/// Converts bytes to an uppercase string of hex numbers in upper case without any spacing or anything
		/// </summary>
		public static string BytesToHexString(this byte[] bytes)
		{
			var sb = new StringBuilder();
			foreach (var b in bytes)
			{
				sb.AppendFormat("{0:X2}", b);
			}

			return sb.ToString();
		}
	}
}
