using System.Drawing;
using BizHawk.Emulation.Common;

namespace BizHawk.Client.EmuHawk.CoreExtensions
{
	public static class CoreExtensions
	{
		public static Bitmap Icon(this IEmulator core)
		{
			var attributes = core.Attributes();

			if (attributes is not PortedCoreAttribute)
			{
				return Properties.Resources.CorpHawkSmall;
			}

			return core switch
			{
				_ => null
			};
		}

		public static string GetSystemDisplayName(this IEmulator emulator) => emulator switch
		{
			NullEmulator => string.Empty,
			_ => EmulatorExtensions.SystemIDToDisplayName(emulator.SystemId)
		};
	}
}
