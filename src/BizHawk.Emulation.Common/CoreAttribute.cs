using System;

namespace BizHawk.Emulation.Common
{
	[AttributeUsage(AttributeTargets.Class)]
	public class CoreAttribute : Attribute
	{
		public readonly string Author;

		public readonly string CoreName;

		public readonly bool Released;

		public readonly bool SingleInstance;

		public CoreAttribute(string name, string author, bool singleInstance = false, bool isReleased = true)
		{
			Author = author;
			CoreName = name;
			Released = isReleased;
			SingleInstance = singleInstance;
		}
	}
}
