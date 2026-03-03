using System;
using System.Collections.Generic;
using System.Drawing;

using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores
{
	[Schema(VSystemID.Raw.NES)]
	// ReSharper disable once UnusedMember.Global
	public class NESSchema : IVirtualPadSchema
	{
		public IEnumerable<PadSchema> GetPadSchemas(IEmulator core, Action<string> showMessageBox)
		{
			switch (core.ControllerDefinition.Name)
			{
				case "NES Controller":
					yield return StandardController();
					yield return ConsoleButtons();
					break;
				case "Unplugged":
					yield return ConsoleButtons();
					break;
				default:
					yield return StandardController();
					yield return ConsoleButtons();
					break;
			}
		}

		private static PadSchema StandardController()
		{
			return new PadSchema
			{
				Size = new Size(194, 90),
				Buttons = new[]
				{
					new ButtonSchema(29, 17, 1, "Up") { DisplayName = "U" },
					new ButtonSchema(29, 61, 1, "Down") { DisplayName = "D" },
					new ButtonSchema(17, 39, 1, "Left") { DisplayName = "L" },
					new ButtonSchema(39, 39, 1, "Right") { DisplayName = "R" },
					new ButtonSchema(130, 39, 1, "B"),
					new ButtonSchema(154, 39, 1, "A"),
					new ButtonSchema(64, 39, 1, "Select") { DisplayName = "s" },
					new ButtonSchema(86, 39, 1, "Start") { DisplayName = "S" },
				}
			};
		}

		protected static PadSchema ConsoleButtons()
		{
			return new ConsoleSchema
			{
				Size = new Size(150, 50),
				Buttons = new[]
				{
					new ButtonSchema(10, 15, "Power") { DisplayName = "Power" },
					new ButtonSchema(70, 15, "Reset") { DisplayName = "Reset" }
				}
			};
		}
	}
}
