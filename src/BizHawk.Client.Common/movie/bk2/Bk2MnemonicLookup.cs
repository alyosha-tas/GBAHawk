using System.Collections.Generic;

using BizHawk.Emulation.Common;

// ReSharper disable StyleCop.SA1509
namespace BizHawk.Client.Common
{
	internal static class Bk2MnemonicLookup
	{
		public static char Lookup(string button, string systemId)
		{
			var key = button.Replace("Key ", "");
			if (key.StartsWith("P"))
			{
				if (key.Length > 2 && key[1] == '1' && key[2] >= '0' && key[2] <= '9') // Hack to support 10-20 controllers, TODO: regex this thing instead
				{
					key = key.Substring(4);
				}
				else if (key.Length > 1 && key[1] >= '0' && key[1] <= '9')
				{
					key = key.Substring(3);
				}
			}

			if (SystemOverrides.TryGetValue(systemId, out var overridesForSys) && overridesForSys.TryGetValue(key, out var c))
			{
				return c;
			}

			if (BaseMnemonicLookupTable.TryGetValue(key, out var c1))
			{
				return c1;
			}

			if (key.Length == 1)
			{
				return key[0];
			}

			return '!';
		}

		public static string LookupAxis(string button, string systemId)
		{
			var key = button
				.Replace("P1 ", "")
				.Replace("P2 ", "")
				.Replace("P3 ", "")
				.Replace("P4 ", "")
				.Replace("Key ", "");

			if (BaseAxisLookupTable.TryGetValue(key, out var s1))
			{
				return s1;
			}

			return button;
		}

		private static readonly Dictionary<string, char> BaseMnemonicLookupTable = new Dictionary<string, char>
		{
			["Power"] = 'P',
			["Reset"] = 'r',
			["Pause"] = 'p',
			["Rotate"] = 'R',

			["Up"] = 'U',
			["Down"] = 'D',
			["Left"] = 'L',
			["Right"] = 'R',

			["Select"] = 's',
			["SELECT"] = 's',
			["Start"] = 'S',
			["START"] = 'S',
			["Run"] = 'R',
			["RUN"] = 'R',

			["Left Shoulder"] = 'l',
			["Right Shoulder"] = 'r',
			["L"] = 'l',
			["R"] = 'r',

			["L1"] = 'l',
			["R1"] = 'r',

			["L2"] = 'L',
			["R2"] = 'R',

			["L3"] = '<',
			["R3"] = '>',

			["Button"] = 'B',
			["Button 1"] = '1',
			["Button 2"] = '2',
			["Button 3"] = '3',
			["Button 4"] = '4',
			["Button 5"] = '5',
			["Button 6"] = '6',
			["Button 7"] = '7',
			["Button 8"] = '8',
			["Button 9"] = '9',
			["B1"] = '1',
			["B2"] = '2',

			["Trigger"] = '1',
			["Trigger 1"] = '1',
			["Trigger 2"] = '2',

			["Mouse Left"] = 'l',
			["Mouse Right"] = 'r',
			["Mouse Center"] = 'c',
			["Mouse Start"] = 's',

			["Left Button"] = 'l',
			["Middle Button"] = 'm',
			["Right Button"] = 'r',

			["Mode"] = 'M',
			["MODE"] = 'M',
			["Mode 1"] = 'M',
			["Mode 2"] = 'm',

			["Fire"] = 'F',
			["Lightgun Trigger"] = 'T',
			["Lightgun Start"] = 'S',
			["Lightgun B"] = 'B',
			["Lightgun C"] = 'C',
			["Microphone"] = 'M',

			["Star"] = '*',
			["Pound"] = '#',

			["X1"] = '1',
			["X2"] = '2',
			["X3"] = '3',
			["X4"] = '4',

			["Y1"] = '1',
			["Y2"] = '2',
			["Y3"] = '3',
			["Y4"] = '4',

			["Triangle"] = 'T',
			["Circle"] = 'O',
			["Cross"] = 'X',
			["Square"] = 'Q',

			["Toggle Left Difficulty"] = 'l',
			["Toggle Right Difficulty"] = 'r',
			["BW"] = 'b',

			["Open"] = 'O',
			["Close"] = 'C',
			["Pedal"] = 'P',

			["Next Disk"] = '>',
			["Previous Disk"] = '<',

			["F1"] = '1',
			["F2"] = '2',
			["F3"] = '3',
			["F4"] = '4',
			["F5"] = '5',
			["F6"] = '6',
			["F7"] = '7',
			["F8"] = '8',
			["F9"] = '9',
			["F10"] = '0'
		};

		private static readonly Dictionary<string, Dictionary<string, char>> SystemOverrides = new Dictionary<string, Dictionary<string, char>>
		{
			[VSystemID.Raw.GBL] = new()
			{
				// gbhawk
				["Toggle Cable"] = 'L',
				["Toggle Cable LC"] = 'L',
				["Toggle Cable CR"] = 'C',
				["Toggle Cable RL"] = 'R',
				["Toggle Cable UD"] = 'U',
				["Toggle Cable LR"] = 'L',
				["Toggle Cable X"] = 'X',
				["Toggle Cable 4x"] = '4',
				// gambatte
				["Toggle Link Connection"] = 'L',
				["Toggle Link Shift"] = 'F',
				["Toggle Link Spacing"] = 'C',
			},
		};

		private static readonly Dictionary<string, string> BaseAxisLookupTable = new Dictionary<string, string>
		{
			["Zapper X"] = "zapX",
			["Zapper Y"] = "zapY",
			["Paddle"] = "Pad",
			["Pen"] = "Pen",
			["Mouse X"] = "mX",
			["Mouse Y"] = "mY",
			["Lightgun X"] = "lX",
			["Lightgun Y"] = "lY",
			["X Axis"] = "aX",
			["Y Axis"] = "aY",
			["LStick X"] = "lsX",
			["LStick Y"] = "lsY",
			["RStick X"] = "rsX",
			["RStick Y"] = "rsY",
			["Disc Select"] = "Disc"
		};
	}
}
