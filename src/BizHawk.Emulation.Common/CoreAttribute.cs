using System;

namespace BizHawk.Emulation.Common
{
	[AttributeUsage(AttributeTargets.Class)]
	public class CoreAttribute : Attribute
	{
		public readonly string CoreName;

		public readonly bool Released;

		public CoreAttribute(string name, bool isReleased = true)
		{
			CoreName = name;
			Released = isReleased;
		}
	}
}
