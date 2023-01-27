namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk
{
	// Default mapper with no save RAM
	public class MapperDefault : MapperBase
	{
		public override void Reset()
		{
			// nothing to initialize
		}

		public override byte ReadMemory8(uint addr)
		{
			return 0xFF; // nothing mapped here
		}

		public override ushort ReadMemory16(uint addr)
		{
			return 0xFFFF; // nothing mapped here
		}

		public override uint ReadMemory32(uint addr)
		{
			return 0xFFFFFFFF; // nothing mapped here
		}

		public override byte PeekMemory(uint addr)
		{
			return ReadMemory8(addr);
		}

		public override void WriteMemory8(uint addr, byte value)
		{

		}

		public override void WriteMemory16(uint addr, ushort value)
		{

		}

		public override void WriteMemory32(uint addr, uint value)
		{

		}

		public override void PokeMemory(uint addr, byte value)
		{
			WriteMemory8(addr, value);
		}
	}
}
