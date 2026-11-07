using System;
using System.IO;
using System.Text;

namespace BizHawk.Common.IOExtensions
{
	public static class IOExtensions
	{
		public static Span<byte> GetBufferAsSpan(this MemoryStream ms)
			=> ms.GetBuffer().AsSpan().Slice(start: 0, length: (int) ms.Length);

		public static byte[] ReadAllBytes(this Stream stream)
		{
			const int BUFF_SIZE = 4096;
			var buffer = new byte[BUFF_SIZE];

			int bytesRead;
			var inStream = new BufferedStream(stream);
			var outStream = new MemoryStream();

			while ((bytesRead = inStream.Read(buffer, 0, BUFF_SIZE)) > 0)
			{
				outStream.Write(buffer, 0, bytesRead);
			}

			return outStream.ToArray();
		}

		public static void Write(this BinaryWriter bw, int[] buffer)
		{
			foreach (int b in buffer)
			{
				bw.Write(b);
			}
		}

		public static void Write(this BinaryWriter bw, short[] buffer)
		{
			foreach (short b in buffer)
			{
				bw.Write(b);
			}
		}
	}
}
