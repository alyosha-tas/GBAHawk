using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

using BizHawk.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA.Common
{
	public interface IGBAGPUViewable : IEmulatorService
	{
		GBAGPUMemoryAreas GetMemoryAreas();

		/// <summary>
		/// calls correspond to entering hblank (maybe) and in a regular frame, the sequence of calls will be 160, 161, ..., 227, 0, ..., 159
		/// </summary>
		void SetScanlineCallback(Action callback, int scanline);
	}

	public class GBAGPUMemoryAreas
	{
		public IntPtr vram;
		public IntPtr oam;
		public IntPtr mmio;
		public IntPtr palram;
	}

	public class GBACommonFunctions
	{
		public static bool pokemon_check(string romHashSHA1)
		{
			if ((romHashSHA1 == "SHA1:424740BE1FC67A5DDB954794443646E6AEEE2C1B") || // Pokemon Ruby (Germany) (Rev 1)
				(romHashSHA1 == "SHA1:1C2A53332382E14DAB8815E3A6DD81AD89534050") || // "" (Germany)	
				(romHashSHA1 == "SHA1:F28B6FFC97847E94A6C21A63CACF633EE5C8DF1E") || // "" (USA)
				(romHashSHA1 == "SHA1:5B64EACF892920518DB4EC664E62A086DD5F5BC8") || // "" (USA, Europe) (Rev 2)
				(romHashSHA1 == "SHA1:610B96A9C9A7D03D2BAFB655E7560CCFF1A6D894") || // "" (Europe) (Rev 1)
				(romHashSHA1 == "SHA1:A6EE94202BEC0641C55D242757E84DC89336D4CB") || // "" (France)
				(romHashSHA1 == "SHA1:BA888DFBA231A231CBD60FE228E894B54FB1ED79") || // "" (France) (Rev 1)
				(romHashSHA1 == "SHA1:5C5E546720300B99AE45D2AA35C646C8B8FF5C56") || // "" (Japan)
				(romHashSHA1 == "SHA1:1F49F7289253DCBFECBC4C5BA3E67AA0652EC83C") || // "" (Spain)
				(romHashSHA1 == "SHA1:9AC73481D7F5D150A018309BBA91D185CE99FB7C") || // "" (Spain) (Rev 1)
				(romHashSHA1 == "SHA1:2B3134224392F58DA00F802FAA1BF4B5CF6270BE") || // "" (Italy)
				(romHashSHA1 == "SHA1:015A5D380AFE316A2A6FCC561798EBFF9DFB3009") || // "" (Italy) (Rev 1)

				(romHashSHA1 == "SHA1:1692DB322400C3141C5DE2DB38469913CEB1F4D4") || // Pokemon Emerald (Italy)
				(romHashSHA1 == "SHA1:F3AE088181BF583E55DAF962A92BB46F4F1D07B7") || // "" (USA, Europe)
				(romHashSHA1 == "SHA1:FE1558A3DCB0360AB558969E09B690888B846DD9") || // "" (Spain)
				(romHashSHA1 == "SHA1:D7CF8F156BA9C455D164E1EA780A6BF1945465C2") || // "" (Japan)
				(romHashSHA1 == "SHA1:61C2EB2B380B1A75F0C94B767A2D4C26CD7CE4E3") || // "" (Germany)
				(romHashSHA1 == "SHA1:CA666651374D89CA439007BED54D839EB7BD14D0") || // "" (France)

				(romHashSHA1 == "SHA1:5A087835009D552D4C5C1F96BE3BE3206E378153") || // Pokemon Sapphire (Germany)
				(romHashSHA1 == "SHA1:7E6E034F9CDCA6D2C4A270FDB50A94DEF5883D17") || // "" (Germany) (Rev 1)
				(romHashSHA1 == "SHA1:4722EFB8CD45772CA32555B98FD3B9719F8E60A9") || // "" (Europe) (Rev 1)
				(romHashSHA1 == "SHA1:89B45FB172E6B55D51FC0E61989775187F6FE63C") || // "" (USA, Europe) (Rev 2)
				(romHashSHA1 == "SHA1:3CCBBD45F8553C36463F13B938E833F652B793E4") || // "" (USA)
				(romHashSHA1 == "SHA1:3233342C2F3087E6FFE6C1791CD5867DB07DF842") || // "" (Japan)
				(romHashSHA1 == "SHA1:0FE9AD1E602E2FAFA090AEE25E43D6980625173C") || // "" (Rev 1)
				(romHashSHA1 == "SHA1:3A6489189E581C4B29914071B79207883B8C16D8") || // "" (Spain)
				(romHashSHA1 == "SHA1:C269B5692B2D0E5800BA1DDF117FDA95AC648634") || // "" (France)
				(romHashSHA1 == "SHA1:860E93F5EA44F4278132F6C1EE5650D07B852FD8") || // "" (France) (Rev 1)
				(romHashSHA1 == "SHA1:73EDF67B9B82FF12795622DCA412733755D2C0FE") || // "" (Italy) (Rev 1)
				(romHashSHA1 == "SHA1:F729DD571FB2C09E72C5C1D68FE0A21E72713D34"))   // "" (Italy))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	public class GBA_ControllerDeck
	{
		public GBA_ControllerDeck(string controller1Name, bool is_subrame = false)
		{
			Port1 = ControllerCtors.TryGetValue(controller1Name, out var ctor1)
				? ctor1(1)
				: throw new InvalidOperationException($"Invalid controller type: {controller1Name}");

			Definition = new(Port1.Definition.Name)
			{
				BoolButtons = Port1.Definition.BoolButtons
					.ToList()
			};

			foreach (var kvp in Port1.Definition.Axes) Definition.Axes.Add(kvp);

			if (is_subrame)
			{
				Definition.AddAxis("Reset Cycle", 0.RangeTo(280896), 280896);
			}

			Definition.MakeImmutable();
		}

		public ushort ReadPort1(IController c)
		{
			return Port1.Read(c);
		}

		public (ushort X, ushort Y) ReadAcc1(IController c)
			=> Port1.ReadAcc(c);

		public byte ReadSolar1(IController c)
		{
			return Port1.SolarSense(c);
		}

		public ControllerDefinition Definition { get; }

		public void SyncState(Serializer ser)
		{
			Port1.SyncState(ser);
		}

		private readonly IPort Port1;

		private static IReadOnlyDictionary<string, Func<int, IPort>> _controllerCtors;

		public static IReadOnlyDictionary<string, Func<int, IPort>> ControllerCtors => _controllerCtors
			??= new Dictionary<string, Func<int, IPort>>
			{
				[typeof(StandardControls).DisplayName()] = portNum => new StandardControls(portNum),
				[typeof(StandardTilt).DisplayName()] = portNum => new StandardTilt(portNum),
				[typeof(StandardSolar).DisplayName()] = portNum => new StandardSolar(portNum),
				[typeof(StandardZGyro).DisplayName()] = portNum => new StandardZGyro(portNum)
			};

		public static string DefaultControllerName => typeof(StandardControls).DisplayName();
	}

	public class GBALink_ControllerDeck
	{
		public GBALink_ControllerDeck(string controller1Name, string controller2Name)
		{
			Port1 = GBA_ControllerDeck.ControllerCtors.TryGetValue(controller1Name, out var ctor1)
				? ctor1(1)
				: throw new InvalidOperationException($"Invalid controller type: {controller1Name}");
			Port2 = GBA_ControllerDeck.ControllerCtors.TryGetValue(controller2Name, out var ctor2)
				? ctor2(2)
				: throw new InvalidOperationException($"Invalid controller type: {controller2Name}");

			Definition = new ControllerDefinition(Port1.Definition.Name)
			{
				BoolButtons = Port1.Definition.BoolButtons
					.Concat(Port2.Definition.BoolButtons)
					.Concat(new[] { "Toggle Cable" })
					.ToList()
			};

			foreach (var kvp in Port1.Definition.Axes) Definition.Axes.Add(kvp);
			foreach (var kvp in Port2.Definition.Axes) Definition.Axes.Add(kvp);

			Definition.MakeImmutable();
		}

		public ushort ReadPort1(IController c)
		{
			return Port1.Read(c);
		}

		public ushort ReadPort2(IController c)
		{
			return Port2.Read(c);
		}

		public (ushort X, ushort Y) ReadAcc1(IController c)
			=> Port1.ReadAcc(c);

		public (ushort X, ushort Y) ReadAcc2(IController c)
			=> Port2.ReadAcc(c);

		public byte ReadSolar1(IController c)
		{
			return Port1.SolarSense(c);
		}

		public byte ReadSolar2(IController c)
		{
			return Port2.SolarSense(c);
		}

		public ControllerDefinition Definition { get; }

		public void SyncState(Serializer ser)
		{
			Port1.SyncState(ser);

			Port2.SyncState(ser);
		}

		private readonly IPort Port1;
		private readonly IPort Port2;
	}


	/// <summary>
	/// Represents a GBA add on
	/// </summary>
	public interface IPort
	{
		ushort Read(IController c);

		(ushort X, ushort Y) ReadAcc(IController c);

		byte SolarSense(IController c);

		ControllerDefinition Definition { get; }

		void SyncState(Serializer ser);

		int PortNum { get; }
	}

	[DisplayName("Gameboy Advance Controller")]
	public class StandardControls : IPort
	{
		public StandardControls(int portNum)
		{
			PortNum = portNum;
			Definition = new("Gameboy Advance Controller")
			{
				BoolButtons = BaseDefinition
				.Select(b => "P" + PortNum + " " + b)
				.ToList()
			};
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		public (ushort X, ushort Y) ReadAcc(IController c)
			=> (0, 0);

		public byte SolarSense(IController c)
			=> 0xFF;

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			//nothing
		}
	}

	[DisplayName("Gameboy Advance Controller + Tilt")]
	public class StandardTilt : IPort
	{
		public StandardTilt(int portNum)
		{
			PortNum = portNum;
			Definition = new ControllerDefinition("Gameboy Advance Controller + Tilt")
			{
				BoolButtons = BaseDefinition.Select(b => $"P{PortNum} {b}").ToList()
			}.AddXYPair($"P{PortNum} Tilt {{0}}", AxisPairOrientation.RightAndUp, (-90).RangeTo(90), 0);
		}

		public int PortNum { get; }

		public float theta, phi, theta_prev, phi_prev, phi_prev_2;

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		public (ushort X, ushort Y) ReadAcc(IController c)
		{
			theta_prev = theta;
			phi_prev_2 = phi_prev;
			phi_prev = phi;

			theta = (float)(c.AxisValue(Definition.Axes[1]) * Math.PI / 180.0);
			phi = (float)(c.AxisValue(Definition.Axes[0]) * Math.PI / 180.0);

			// acc x is the result of rotating around body y AFTER rotating around body x
			// therefore this control scheme gives decreasing sensitivity in X as Y rotation increases
			var temp = (float)(Math.Cos(theta) * Math.Sin(phi));
			// additional acceleration components are dominated by axial components due to off axis rotation.
			// They vary widely based on physical hand movements, but this roughly matches what I observe in a real GBP
			var temp2 = (float)((phi - 2 * phi_prev + phi_prev_2) * 59.7275 * 59.7275 * 0.1);
			var accX = (ushort)(0x3A0 - Math.Floor(temp * 256) - temp2);

			// acc y is just the sine of the angle
			var temp3 = (float)Math.Sin(theta);
			// here we add in the acceleration generated by the point of rotation being far away from the accelerometer
			// this term dominates other facators due to the cartridge being far from the players hands in whatever system is being used.
			// It roughly matches what I observe in a real GBP
			var temp4 = (float)(Math.Pow((theta - theta_prev) * 59.7275, 2) * 0.15);
			var accY = (ushort)(0x3A0 - Math.Floor(temp3 * 256) + temp4);

			return (accX, accY);
		}

		public byte SolarSense(IController c)
			=> 0xFF;

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			// since we need rate of change of angle, need to savestate them
			ser.Sync(nameof(theta), ref theta);
			ser.Sync(nameof(phi), ref phi);
			ser.Sync(nameof(phi_prev), ref phi_prev);
		}
	}


	[DisplayName("Gameboy Advance Controller + Solar")]
	public class StandardSolar : IPort
	{
		public StandardSolar(int portNum)
		{
			PortNum = portNum;
			Definition = new ControllerDefinition("Gameboy Advance Controller + Solar")
			{
				BoolButtons = BaseDefinition.Select(b => $"P{PortNum} {b}").ToList()
			}.AddAxis($"P{PortNum} Solar", (0x50).RangeTo(0xF0), 0xA0);
		}

		public int PortNum { get; }

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		public (ushort X, ushort Y) ReadAcc(IController c)
			=> (0, 0);

		public byte SolarSense(IController c)
		{
			return (byte)(0x140 - (int)(c.AxisValue(Definition.Axes[0])));
		}

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			// nothing
		}
	}

	[DisplayName("Gameboy Advance Controller + Z Gyro")]
	public class StandardZGyro : IPort
	{
		public StandardZGyro(int portNum)
		{
			PortNum = portNum;
			Definition = new ControllerDefinition("Gameboy Advance Controller + Z Gyro")
			{
				BoolButtons = BaseDefinition.Select(b => $"P{PortNum} {b}").ToList()
			}.AddAxis($"P{PortNum} Z Gyro", (-90).RangeTo(90), 0);
		}

		public int PortNum { get; }

		public float theta, theta_prev;

		public ControllerDefinition Definition { get; }

		public ushort Read(IController c)
		{
			ushort result = 0x3FF;

			if (c.IsPressed(Definition.BoolButtons[0]))
			{
				result &= 0xFFBF;
			}
			if (c.IsPressed(Definition.BoolButtons[1]))
			{
				result &= 0xFF7F;
			}
			if (c.IsPressed(Definition.BoolButtons[2]))
			{
				result &= 0xFFDF;
			}
			if (c.IsPressed(Definition.BoolButtons[3]))
			{
				result &= 0xFFEF;
			}
			if (c.IsPressed(Definition.BoolButtons[4]))
			{
				result &= 0xFFF7;
			}
			if (c.IsPressed(Definition.BoolButtons[5]))
			{
				result &= 0xFFFB;
			}
			if (c.IsPressed(Definition.BoolButtons[6]))
			{
				result &= 0xFFFD;
			}
			if (c.IsPressed(Definition.BoolButtons[7]))
			{
				result &= 0xFFFE;
			}
			if (c.IsPressed(Definition.BoolButtons[8]))
			{
				result &= 0xFDFF;
			}
			if (c.IsPressed(Definition.BoolButtons[9]))
			{
				result &= 0xFEFF;
			}

			return result;
		}

		// repurposing from X/Y controls, X is Z axis, Y is just zero
		// seems to act like a rate gyro
		public (ushort X, ushort Y) ReadAcc(IController c)
		{
			theta_prev = theta;

			theta = c.AxisValue(Definition.Axes[0]);

			float d_theta = (float)((theta - theta_prev) * 59.7275);

			var accX = (ushort)(0x6C0 + Math.Floor(d_theta));

			return (accX, 0);
		}

		public byte SolarSense(IController c)
			=> 0xFF;

		private static readonly string[] BaseDefinition =
		{
			"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
		};

		public void SyncState(Serializer ser)
		{
			// since we need rate of change of angle, need to savestate them
			ser.Sync(nameof(theta), ref theta);
		}
	}
}
