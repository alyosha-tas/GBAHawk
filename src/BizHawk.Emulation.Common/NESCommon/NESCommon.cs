using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Common.ReflectionExtensions;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

namespace BizHawk.Emulation.Cores.Nintendo.NES.Common
{
	public class NESGPUMemoryAreas
	{
		public IntPtr vram;
		public IntPtr oam;
		public IntPtr mmio;
		public IntPtr palram;
	}

	public class NESCommonFunctions
	{
		public int[] Palette_Compiled = new int[64 * 8];

		/// <summary>
		/// Sets the provided palette as current.
		/// Applies the current deemph settings if needed to expand a 64-entry palette to 512
		/// </summary>
		public static void SetPalette(byte[,] pal, int[] comp_pal)
		{
			int nColors = pal.GetLength(0);

			if (nColors == 512)
			{
				//just copy the palette directly
				for (int c = 0; c < 64 * 8; c++)
				{
					int r = pal[c, 0];
					int g = pal[c, 1];
					int b = pal[c, 2];
					comp_pal[c] = (int)unchecked((int)0xFF000000 | (r << 16) | (g << 8) | b);
				}
			}
			else
			{
				//expand using deemph
				for (int i = 0; i < 64 * 8; i++)
				{
					int d = i >> 6;
					int c = i & 63;
					int r = pal[c, 0];
					int g = pal[c, 1];
					int b = pal[c, 2];
					Palettes.ApplyDeemphasis(ref r, ref g, ref b, d);
					comp_pal[i] = (int)unchecked((int)0xFF000000 | (r << 16) | (g << 8) | b);
				}
			}
		}
	}

	public static class RomChecksums
	{
		public const string CamericaGolden5 = /*sha1:*/"60FC5FA5B5ACCAF3AEFEBA73FC8BFFD3C4DAE558";

		public const string CamericaGolden5Overdump = /*sha1:*/"BAD382331C30B22A908DA4BFF2759C25113CC26A";

		public const string CamericaPegasus4in1 = /*sha1:*/"40409FEC8249EFDB772E6FFB2DCD41860C6CCA23";

		public const string DancingBlocks = /*sha1:*/"68ABE1E49C9E9CCEA978A48232432C252E5912C0";

		public const string SeicrossRev2 = "SHA1:4C9C05FAD6F6F33A92A27C2EDC1E7DE12D7F216D"; // yes this is meant to include the prefix

		public const string SilvaSaga = /*sha1:*/"00C50062A2DECE99580063777590F26A253AAB6B";

		public const string Fam_Jump_II = /*sha1:*/"1D7417D31E19B590AFCEB6A8A6E7B9CAB9F9B475";
	}

	/// <summary>
	/// stores information about the strobe lines controlled by $4016
	/// </summary>
	public struct StrobeInfo
	{
		/// <summary>
		/// the current value of $4016.0; strobes regular controller ports
		/// </summary>
		public readonly int OUT0;
		/// <summary>
		/// the current value of $4016.1; strobes expansion port
		/// </summary>
		public readonly int OUT1;
		/// <summary>
		/// the current value of $4016.2; strobes expansion port
		/// </summary>
		public readonly int OUT2;
		/// <summary>
		/// the previous value or $4016.0 (for edge sensitive equipment)
		/// </summary>
		public readonly int OUT0old;
		/// <summary>
		/// the previous value or $4016.1 (for edge sensitive equipment)
		/// </summary>
		public readonly int OUT1old;
		/// <summary>
		/// the previous value or $4016.2 (for edge sensitive equipment)
		/// </summary>
		public readonly int OUT2old;

		/// <param name="oldValue">the old latched $4016 byte</param>
		/// <param name="newValue">the new latched $4016 byte</param>
		public StrobeInfo(byte oldValue, byte newValue)
		{
			OUT0old = oldValue & 1;
			OUT1old = oldValue >> 1 & 1;
			OUT2old = oldValue >> 2 & 1;
			OUT0 = newValue & 1;
			OUT1 = newValue >> 1 & 1;
			OUT2 = newValue >> 2 & 1;
		}
	}

	public class NES_ControllerDeck
	{
		public NES_ControllerDeck(string controller1Name, string controller2Name, bool is_subrame = false)
		{
			Port1 = ControllerCtors.TryGetValue(controller1Name, out var ctor1)
				? ctor1(1)
				: throw new InvalidOperationException($"Invalid controller type: {controller1Name}");
			Port2 = ControllerCtors.TryGetValue(controller2Name, out var ctor2)
				? ctor2(2)
				: throw new InvalidOperationException($"Invalid controller type: {controller2Name}");

			if (Port2 is UnpluggedNES)
			{
				Definition = new ControllerDefinition(Port1.Definition.Name)
				{
					BoolButtons = Port1.Definition.BoolButtons
						.ToList()
				};
			}
			else
			{
				Definition = new ControllerDefinition(Port1.Definition.Name)
				{
					BoolButtons = Port1.Definition.BoolButtons
								.Concat(Port2.Definition.BoolButtons)
								.ToList()
				};
			}

			// controls other than the deck
			Definition.BoolButtons.Add("Power");
			Definition.BoolButtons.Add("Reset");

			foreach (var kvp in Port1.Definition.Axes) Definition.Axes.Add(kvp);
			foreach (var kvp in Port2.Definition.Axes) Definition.Axes.Add(kvp);

			if (is_subrame)
			{
				Definition.AddAxis("Reset Cycle", 0.RangeTo(280896), 280896);
			}

			Definition.MakeImmutable();
		}

		public byte ReadPort1(IController c)
		{
			return Port1.Read(c);
		}

		public byte ReadPort2(IController c)
		{
			return Port2.Read(c);
		}

		public void Strobe(StrobeInfo s, IController c)
		{
			Port1.Strobe(s, c);
			Port2.Strobe(s, c);
		}

		public ControllerDefinition Definition { get; }

		public void SyncState(Serializer ser)
		{
			Port1.SyncState(ser);

			Port2.SyncState(ser);
		}

		private readonly IPort Port1;
		private readonly IPort Port2;

		private static IReadOnlyDictionary<string, Func<int, IPort>>? _controllerCtors;

		public static IReadOnlyDictionary<string, Func<int, IPort>> ControllerCtors => _controllerCtors
			??= new Dictionary<string, Func<int, IPort>>
			{
				[typeof(NESController).DisplayName()] = portNum => new NESController(portNum),
				[typeof(SNESController).DisplayName()] = portNum => new SNESController(portNum),
				[typeof(FourScore).DisplayName()] = portNum => new FourScore(portNum),
				[typeof(UnpluggedNES).DisplayName()] = portNum => new UnpluggedNES(portNum)
			};

		public static string DefaultControllerName => typeof(NESController).DisplayName();
	}


	/// <summary>
	/// Represents a NES add on
	/// </summary>
	public interface IPort
	{
		void Latch(IController c);

		void Strobe(StrobeInfo s, IController c);

		byte Read(IController c);

		ControllerDefinition Definition { get; }

		void SyncState(Serializer ser);

		int PortNum { get; }
	}

	[DisplayName("NES Controller")]
	public class NESController : IPort
	{
		private bool _resetting;
		private int _latchedValue;

		public NESController(int portNum)
		{
			PortNum = portNum;
			Definition = new("NES Controller")
			{
				BoolButtons = BaseDefinition
				.Select(b => "P" + PortNum + " " + b)
				.ToList()
			};
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public void Latch(IController c)
		{
			int ret = 0;
			for (int i = 0; i < 32; i++)
			{
				if (i < 8)
				{
					if (c.IsPressed(Definition.BoolButtons[BaseDefinitionOrder[i]]))
						ret |= 1 << i;
				}
				else
				{
					// 1 in all other bits
					ret |= 1 << i;
				}
			}

			_latchedValue = ret;
		}

		public void Strobe(StrobeInfo s, IController c)
		{
			_resetting = s.OUT0 != 0;
			if (s.OUT0 < s.OUT0old)
				Latch(c);
		}

		public byte Read(IController c)
		{
			if (_resetting)
				Latch(c);
			byte ret = (byte)(_latchedValue & 1);
			if (!_resetting)
				_latchedValue >>= 1; // ASR not LSR, so endless stream of 1s after data
			return ret;
		}

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B",  "A"
		};

		private static readonly int[] BaseDefinitionOrder =
{
			7, 6, 5, 4, 0, 1, 2, 3
		};

		public void SyncState(Serializer ser)
		{
			ser.Sync(nameof(_resetting), ref _resetting);
			ser.Sync(nameof(_latchedValue), ref _latchedValue);
		}
	}

	[DisplayName("SNES Controller")]
	public class SNESController : IPort
	{
		private bool _resetting;
		private int _latchedValue;

		public SNESController(int portNum)
		{
			PortNum = portNum;
			Definition = new("SNES Controller")
			{
				BoolButtons = BaseDefinition
				.Select(b => "P" + PortNum + " " + b)
				.ToList()
			};
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public void Latch(IController c)
		{
			int ret = 0;
			for (int i = 0; i < 32; i++)
			{
				if (i < 12)
				{
					if (c.IsPressed(Definition.BoolButtons[i]))
						ret |= 1 << i;
				}
				else if (i < 16)
				{
					ret |= 0 << i;
				}
				else
				{
					// 1 in all other bits
					ret |= 1 << i;
				}
			}

			_latchedValue = ret;
		}

		public void Strobe(StrobeInfo s, IController c)
		{
			_resetting = s.OUT0 != 0;
			if (s.OUT0 < s.OUT0old)
				Latch(c);
		}

		public byte Read(IController c)
		{
			if (_resetting)
				Latch(c);
			byte ret = (byte)(_latchedValue & 1);
			if (!_resetting)
				_latchedValue >>= 1; // ASR not LSR, so endless stream of 1s after data
			return ret;
		}

		private static readonly string[] BaseDefinition =
		{
			"B", "Y", "Select", "Start", "Up", "Down", "Left", "Right",
			"A", "X", "L", "R"
		};

		public void SyncState(Serializer ser)
		{
			ser.Sync(nameof(_resetting), ref _resetting);
			ser.Sync(nameof(_latchedValue), ref _latchedValue);
		}
	}


	[DisplayName("FourScore")]
	public class FourScore : IPort
	{
		private bool _resetting;
		private int _latchedValue;

		public FourScore(int portNum)
		{
			PortNum = portNum;
			Definition = new ControllerDefinition("FourScore")
			{
				BoolButtons = BaseDefinition.Select(b => $"P{PortNum} {b}").ToList()
			};
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public void Latch(IController c)
		{
			int ret = 0;
			for (int i = 0; i < 32; i++)
			{
				if (i < 16)
				{
					if (c.IsPressed(Definition.BoolButtons[i]))
						ret |= 1 << i;
				}
				else
				{
					// 1 in all other bits
					ret |= 1 << i;
				}
			}

			// set signatures
			_latchedValue &= ~0xff0000;

			if (PortNum == 1) // signatures
				_latchedValue |= 0x040000;
			else
				_latchedValue |= 0x080000;

			_latchedValue = ret;
		}

		public void Strobe(StrobeInfo s, IController c)
		{
			_resetting = s.OUT0 != 0;
			if (s.OUT0 < s.OUT0old)
				Latch(c);
		}

		public byte Read(IController c)
		{
			if (_resetting)
				Latch(c);
			byte ret = (byte)(_latchedValue & 1);
			if (!_resetting)
				_latchedValue >>= 1; // ASR not LSR, so endless stream of 1s after data
			return ret;
		}

		private static readonly string[] BaseDefinition =
		{
			"1_A", "1_B", "1_Select", "1_Start", "1_Up", "1_Down", "1_Left", "1_Right",
			"2_A", "2_B", "2_Select", "2_Start", "2_Up", "2_Down", "2_Left", "2_Right",
		};

		public void SyncState(Serializer ser)
		{
			ser.Sync(nameof(_resetting), ref _resetting);
			ser.Sync(nameof(_latchedValue), ref _latchedValue);
		}
	}


	[DisplayName("Unplugged")]
	public class UnpluggedNES : IPort
	{
		private bool _resetting;
		private int _latchedValue;

		public UnpluggedNES(int portNum)
		{
			PortNum = portNum;
			Definition = new("Unplugged")
			{
				BoolButtons = BaseDefinition
				.Select(b => "P" + PortNum + " " + b)
				.ToList()
			};
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public void Latch(IController c)
		{

		}

		public void Strobe(StrobeInfo s, IController c)
		{

		}

		public byte Read(IController c)
		{
			return 0;
		}

		private static readonly string[] BaseDefinition =
		{
			" "
		};

		public void SyncState(Serializer ser)
		{
			ser.Sync(nameof(_resetting), ref _resetting);
			ser.Sync(nameof(_latchedValue), ref _latchedValue);
		}
	}
}
