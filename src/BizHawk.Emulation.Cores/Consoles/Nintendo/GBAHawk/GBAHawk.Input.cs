using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

using BizHawk.Common;
using BizHawk.Common.ReflectionExtensions;
using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public class GBAHawk_ControllerDeck
	{
		public GBAHawk_ControllerDeck(string controller1Name)
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