using System;
using System.Collections.Generic;

using BizHawk.Common;
using BizHawk.Emulation.Common;

namespace BizHawk.Client.Common
{
	internal class BkmControllerAdapter : IController
	{
		public IInputDisplayGenerator InputDisplayGenerator { get; set; } = null;

		public BkmControllerAdapter(ControllerDefinition definition, string systemId)
		{
			// We do need to map the definition name to the legacy
			// controller names that were used back in the bkm days
			var name = systemId switch
			{
				"SNES" => "SNES Controller",
				"GBA" => "GBA Controller",
				"DGB" => "Dual Gameboy Controller",
				"N64" => "Nintendo 64 Controller",
				"NES" => "NES Controller",
				"GB" => "Gameboy Controller",
				"SMS Controller" => "SMS",
				_ => "Null Controller",
			};
			Definition = new(copyFrom: definition, withName: name);
		}

		public ControllerDefinition Definition { get; set; }

		public bool IsPressed(string button)
		{
			return _myBoolButtons[button];
		}

		public int AxisValue(string name)
		{
			return _myAxisControls[name];
		}

		public IReadOnlyCollection<(string Name, int Strength)> GetHapticsSnapshot() => throw new NotImplementedException(); // no idea --yoshi

		public void SetHapticChannelStrength(string name, int strength) => throw new NotImplementedException(); // no idea --yoshi

		/// <summary>
		/// latches all buttons from the supplied mnemonic string
		/// </summary>
		public void SetControllersAsMnemonic(string mnemonic)
		{
			switch (ControlType)
			{
				case "Null Controller":
					return;
				case "GBA Controller":
					SetGBAControllersAsMnemonic(mnemonic);
					return;
				case "Dual Gameboy Controller":
					SetDualGameBoyControllerAsMnemonic(mnemonic);
					return;
			}

			var c = new MnemonicChecker(mnemonic);

			_myBoolButtons.Clear();

			int start = 3;
			if (ControlType == "NES Controller")
			{
				if (mnemonic.Length < 2)
				{
					return;
				}

				switch (mnemonic[1])
				{
					case 'P':
						Force("Power", true);
						break;
					case 'E':
						Force("FDS Eject", true);
						break;
					case '0':
						Force("FDS Insert 0", true);
						break;
					case '1':
						Force("FDS Insert 1", true);
						break;
					case '2':
						Force("FDS Insert 2", true);
						break;
					case '3':
						Force("FDS Insert 3", true);
						break;
					case 'c':
						Force("VS Coin 1", true);
						break;
					case 'C':
						Force("VS Coin 2", true);
						break;
					default:
					{
						if (mnemonic[1] != '.')
						{
							Force("Reset", true);
						}

						break;
					}
				}
			}

			if (ControlType == "Gameboy Controller")
			{
				if (mnemonic.Length < 2)
				{
					return;
				}

				Force("Power", mnemonic[1] != '.');
			}

			if (ControlType == "SMS Controller" || ControlType == "TI83 Controller" || ControlType == "ColecoVision Basic Controller")
			{
				start = 1;
			}

			if (ControlType == "Atari 2600 Basic Controller")
			{
				if (mnemonic.Length < 2)
				{
					return;
				}

				Force("Reset", mnemonic[1] != '.' && mnemonic[1] != '0');
				Force("Select", mnemonic[2] != '.' && mnemonic[2] != '0');
				start = 4;
			}

			for (int player = 1; player <= BkmMnemonicConstants.Players[ControlType]; player++)
			{
				int srcIndex = (player - 1) * (BkmMnemonicConstants.Buttons[ControlType].Count + 1);
				int ctr = start;
				if (mnemonic.Length < srcIndex + ctr + BkmMnemonicConstants.Buttons[ControlType].Count - 1)
				{
					return;
				}

				string prefix = "";
				if (ControlType != "Gameboy Controller" && ControlType != "TI83 Controller")
				{
					prefix = $"P{player} ";
				}

				foreach (string button in BkmMnemonicConstants.Buttons[ControlType].Keys)
				{
					Force(prefix + button, c[srcIndex + ctr++]);
				}
			}
		}

		private readonly WorkingDictionary<string, bool> _myBoolButtons = new WorkingDictionary<string, bool>();
		private readonly WorkingDictionary<string, int> _myAxisControls = new WorkingDictionary<string, int>();

		private void Force(string button, bool state)
		{
			_myBoolButtons[button] = state;
		}

		private void Force(string name, int state)
		{
			_myAxisControls[name] = state;
		}

		private string ControlType => Definition.Name;

		private void SetGBAControllersAsMnemonic(string mnemonic)
		{
			var c = new MnemonicChecker(mnemonic);
			_myBoolButtons.Clear();
			if (mnemonic.Length < 2)
			{
				return;
			}

			if (mnemonic[1] == 'P')
			{
				Force("Power", true);
			}

			int start = 3;
			foreach (string button in BkmMnemonicConstants.Buttons[ControlType].Keys)
			{
				Force(button, c[start++]);
			}
		}

		private void SetDualGameBoyControllerAsMnemonic(string mnemonic)
		{
			var checker = new MnemonicChecker(mnemonic);
			_myBoolButtons.Clear();
			for (int i = 0; i < BkmMnemonicConstants.DgbMnemonic.Length; i++)
			{
				var t = BkmMnemonicConstants.DgbMnemonic[i];
				if (t.Item1 != null)
				{
					Force(t.Item1, checker[i]);
				}
			}
		}

		private sealed class MnemonicChecker
		{
			private readonly string _mnemonic;

			public MnemonicChecker(string mnemonic)
			{
				_mnemonic = mnemonic;
			}

			public bool this[int c] => !string.IsNullOrEmpty(_mnemonic) && _mnemonic[c] != '.';
		}
	}
}
