using System;
using System.Collections.Generic;
using System.Drawing;

using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA;

namespace BizHawk.Emulation.Cores
{
	[Schema(VSystemID.Raw.GBA)]
	// ReSharper disable once UnusedMember.Global
	public class GBASchema : IVirtualPadSchema
	{
		public IEnumerable<PadSchema> GetPadSchemas(IEmulator core, Action<string> showMessageBox)
		{
			switch (core.ControllerDefinition.Name)
			{
				case "Gameboy Advance Controller + Tilt":
					yield return StandardController();
					yield return ConsoleButtons();
					yield return TiltControls();
					break;
				case "Gameboy Advance Controller + Solar":
					yield return StandardController();
					yield return ConsoleButtons();
					yield return SolarControls();
					break;
				case "Gameboy Advance Controller + Z Gyro":
					yield return StandardController();
					yield return ConsoleButtons();
					yield return ZControls();
					break;
				default:
					yield return StandardController();
					yield return ConsoleButtons();
					break;
			}
		}

		private static PadSchema TiltControls()
		{
			return new PadSchema
			{
				DisplayName = "Tilt",
				Size = new Size(266, 250),
				Buttons = new[]
				{
					new TargetedPairSchema(14, 17, "P1 Tilt X")
					{
						TargetSize = new Size(256, 240)
					}
				}
			};
		}

		private static PadSchema ZControls()
		{
			return new PadSchema
			{
				DisplayName = "Z Gyro",
				Size = new Size(266, 80),
				Buttons = new[]
				{
					new SingleAxisSchema(10, 15, "P1 Z Gyro")
					{
						TargetSize = new Size(226, 69),
						MinValue = -90,
						MaxValue = 90
					}
				}
			};
		}

		private static PadSchema SolarControls()
		{
			return new PadSchema
			{
				DisplayName = "Solar Sensor",
				Size = new Size(266, 80),
				Buttons = new[]
				{
					new SingleAxisSchema(10, 15, "P1 Solar")
					{
						TargetSize = new Size(226, 69),
						MinValue = 0x50,
						MaxValue = 0xF0
					}
				}
			};
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
					new ButtonSchema(2, 12, 1, "L") { DisplayName = "l" },
					new ButtonSchema(166, 12, 1, "R") { DisplayName = "r" }
				}
			};
		}

		protected static PadSchema ConsoleButtons()
		{
			return new ConsoleSchema
			{
				Size = new Size(75, 50),
				Buttons = new[]
				{
					new ButtonSchema(10, 15, "P1 Power") { DisplayName = "Power" }
				}
			};
		}
	}
}
