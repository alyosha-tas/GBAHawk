using System;
using System.Collections.Generic;
using System.Drawing;

using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores
{
	[Schema(VSystemID.Raw.GBAL)]
	// ReSharper disable once UnusedMember.Global
	public class GBALinkSchema : IVirtualPadSchema
	{
		public IEnumerable<PadSchema> GetPadSchemas(IEmulator core, Action<string> showMessageBox)
		{
			yield return StandardController(1);
			yield return StandardController(2);
			yield return ConsoleButtons();
		}

		private static PadSchema StandardController(int cntr)
		{
			return new PadSchema
			{
				Size = new Size(194, 90),
				Buttons = new[]
				{
					new ButtonSchema(29, 17, cntr, "Up") { DisplayName = "U" },
					new ButtonSchema(29, 61, cntr, "Down") { DisplayName = "D" },
					new ButtonSchema(17, 39, cntr, "Left") { DisplayName = "L" },
					new ButtonSchema(39, 39, cntr, "Right") { DisplayName = "R" },
					new ButtonSchema(130, 39, cntr, "B"),
					new ButtonSchema(154, 39, cntr, "A"),
					new ButtonSchema(64, 39, cntr, "Select") { DisplayName = "s" },
					new ButtonSchema(86, 39, cntr, "Start") { DisplayName = "S" },
					new ButtonSchema(2, 12, cntr, "L") { DisplayName = "l" },
					new ButtonSchema(166, 12, cntr, "R") { DisplayName = "r" }
				}
			};
		}

		protected static PadSchema ConsoleButtons()
		{
			return new ConsoleSchema
			{
				Size = new Size(175, 50),
				Buttons = new[]
				{
					new ButtonSchema(10, 15, "P1 Power") { DisplayName = "Power" },
					new ButtonSchema(90, 15, "P2 Power") { DisplayName = "Power" }
				}
			};
		}
	}
}
