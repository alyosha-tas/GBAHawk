using BizHawk.Emulation.Common;

namespace BizHawk.Emulation.Cores.Nintendo.GBA
{
	public partial class GBAHawk
	{
		public static readonly ControllerDefinition GBAController = new ControllerDefinition("GBA Controller Joystick")
		{
			BoolButtons =
				{
					"Up", "Down", "Left", "Right", "Start", "Select", "B", "A", "L", "R", "Power"
				}
		}.MakeImmutable();
	}
}