#nullable disable

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;

namespace BizHawk.Common
{
	public unsafe class Serializer
	{
		public Serializer() { }

		public bool IsReader => _isReader;

		public bool IsWriter => !IsReader;

		public BinaryReader BinaryReader => _br;

		public BinaryWriter BinaryWriter => _bw;

		public Serializer(BinaryWriter bw)
		{
			StartWrite(bw);
		}

		public Serializer(BinaryReader br)
		{
			StartRead(br);
		}

		public static Serializer CreateBinaryWriter(BinaryWriter bw)
		{
			return new Serializer(bw);
		}

		public static Serializer CreateBinaryReader(BinaryReader br)
		{
			return new Serializer(br);
		}

		public void StartWrite(BinaryWriter bw)
		{
			_bw = bw;
			_isReader = false;
		}

		public void StartRead(BinaryReader br)
		{
			_br = br;
			_isReader = true;
		}

		public void BeginSection(string name)
		{
			_sections.Push(name);

		}

		public void EndSection()
		{
			var name = _sections.Pop();
		}

		/// <exception cref="InvalidOperationException"><typeparamref name="T"/> does not inherit <see cref="Enum"/></exception>
		public void SyncEnum<T>(string name, ref T val) where T : struct
		{
			if (typeof(T).BaseType != typeof(Enum))
			{
				throw new InvalidOperationException();
			}
			
			if (IsReader)
			{
				val = (T)Enum.ToObject(typeof(T), _br.ReadInt32());
			}
			else
			{
				_bw.Write(Convert.ToInt32(val));
			}
		}

		public void Sync(string name, ref byte[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(useNull);
			}
			else
			{
				_bw.WriteByteBuffer(val);
			}
		}

		public void Sync(string name, ref bool[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToBoolBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<bool>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}

		public void Sync(string name, ref short[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToShortBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<short>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}

		public void Sync(string name, ref ushort[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToUShortBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<ushort>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}

		public void Sync(string name, ref int[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToIntBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<int>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}

		public void Sync(string name, ref uint[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToUIntBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<uint>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}

		public void Sync(string name, ref float[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToFloatBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<float>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}

		public void Sync(string name, ref double[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToDoubleBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<double>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}


		public void Sync(string name, ref ulong[] val, bool useNull)
		{
			if (IsReader)
			{
				val = _br.ReadByteBuffer(false).ToULongBuffer();
				if (val == null && !useNull)
				{
					val = Array.Empty<ulong>();
				}
			}
			else
			{
				_bw.WriteByteBuffer(val.ToUByteBuffer());
			}
		}

		public void Sync(string name, ref byte val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref ushort val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref uint val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref sbyte val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref short val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref int val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref long val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref ulong val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref float val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref double val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		public void Sync(string name, ref bool val)
		{
			if (IsReader)
			{
				Read(ref val);
			}
			else
			{
				Write(ref val);
			}
		}

		/// <exception cref="InvalidOperationException"><see cref="IsReader"/> is <see langword="false"/> and <paramref name="name"/> is longer than <paramref name="length"/> chars</exception>
		public void SyncFixedString(string name, ref string val, int length)
		{
			// TODO - this could be made more efficient perhaps just by writing values right out of the string..
			if (IsReader)
			{
				var buf = new char[length];

				_br.Read(buf, 0, length);

				var len = 0;
				for (; len < length; len++)
				{
					if (buf[len] == 0)
					{
						break;
					}
				}

				val = new string(buf, 0, len);
			}
			else
			{
				if (name.Length > length)
				{
					throw new InvalidOperationException($"{nameof(SyncFixedString)} too long");
				}

				var buf = val.ToCharArray();
				var remainder = new char[length - buf.Length];

				_bw.Write(buf);
				_bw.Write(remainder);
			}
		}

		private BinaryReader _br;
		private BinaryWriter _bw;

		private bool _isReader;
		private readonly Stack<string> _sections = new Stack<string>();
		private Section _readerSection, _currSection;
		private readonly Stack<Section> _sectionStack = new Stack<Section>();

		private string Item(string key)
		{
			return _currSection.Items[key];
		}

		private bool Present(string key)
		{
			return _currSection.Items.ContainsKey(key);
		}

		private void SyncBuffer(string name, int elemsize, int len, void* ptr)
		{
			if (IsReader)
			{
				byte[] temp = null;
				Sync(name, ref temp, false);
				int todo = Math.Min(temp.Length, len * elemsize);
				System.Runtime.InteropServices.Marshal.Copy(temp, 0, new IntPtr(ptr), todo);
			}
			else
			{
				int todo = len * elemsize;
				var temp = new byte[todo];
				System.Runtime.InteropServices.Marshal.Copy(new IntPtr(ptr), temp, 0, todo);
				Sync(name, ref temp, false);
			}
		}

		private void Read(ref byte val)
		{
			val = _br.ReadByte();
		}

		private void Write(ref byte val)
		{
			_bw.Write(val);
		}

		private void Read(ref ushort val)
		{
			val = _br.ReadUInt16();
		}

		private void Write(ref ushort val)
		{
			_bw.Write(val);
		}

		private void Read(ref uint val)
		{
			{ val = _br.ReadUInt32(); }
		}

		private void Write(ref uint val)
		{
			_bw.Write(val);
		}

		private void ReadText(string name, ref uint val)
		{
			if (Present(name))
			{
				val = uint.Parse(Item(name).Replace("0x", ""), NumberStyles.HexNumber);
			}
		}

		private void Read(ref sbyte val)
		{
			val = _br.ReadSByte();
		}

		private void Write(ref sbyte val)
		{
			_bw.Write(val);
		}

		private void Read(ref short val)
		{
			val = _br.ReadInt16();
		}

		private void Write(ref short val)
		{
			_bw.Write(val);
		}

		private void Read(ref int val)
		{
			val = _br.ReadInt32();
		}

		private void Write(ref int val)
		{
			_bw.Write(val);
		}

		private void Read(ref long val)
		{
			val = _br.ReadInt64();
		}

		private void Write(ref long val)
		{
			_bw.Write(val);
		}

		private void Read(ref ulong val)
		{
			val = _br.ReadUInt64();
		}

		private void Write(ref ulong val)
		{
			_bw.Write(val);
		}

		private void Read(ref float val)
		{
			val = _br.ReadSingle();
		}

		private void Write(ref float val)
		{
			_bw.Write(val);
		}

		private void Read(ref double val)
		{
			val = _br.ReadDouble();
		}

		private void Write(ref double val)
		{
			_bw.Write(val);
		}

		private void Read(ref bool val)
		{
			val = _br.ReadBoolean();
		}

		private void Write(ref bool val)
		{
			_bw.Write(val);
		}

		private sealed class Section : Dictionary<string, Section>
		{
			public string Name = "";
			public readonly Dictionary<string, string> Items = new Dictionary<string, string>();
		}
	}
}
