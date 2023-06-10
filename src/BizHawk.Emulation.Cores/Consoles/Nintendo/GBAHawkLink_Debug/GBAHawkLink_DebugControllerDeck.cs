using System;
using System.Linq;

using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug
{
	public class GBAHawkLink_Debug_ControllerDeck
	{
		public GBAHawkLink_Debug_ControllerDeck(string controller1Name, string controller2Name)
		{
			Port1 = GBAHawk_Debug_ControllerDeck.ControllerCtors.TryGetValue(controller1Name, out var ctor1)
				? ctor1(1)
				: throw new InvalidOperationException($"Invalid controller type: {controller1Name}");
			Port2 = GBAHawk_Debug_ControllerDeck.ControllerCtors.TryGetValue(controller2Name, out var ctor2)
				? ctor2(2)
				: throw new InvalidOperationException($"Invalid controller type: {controller2Name}");

			Definition = new ControllerDefinition(Port1.Definition.Name)
			{
				BoolButtons = Port1.Definition.BoolButtons
					.Concat(Port2.Definition.BoolButtons)
					.Concat(new[] { "Toggle Cable" } )
					.ToList()
			}.MakeImmutable();
		}

		public ushort ReadPort1(IController c)
		{
			return Port1.Read(c);
		}

		public ushort ReadPort2(IController c)
		{
			return Port2.Read(c);
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
}
