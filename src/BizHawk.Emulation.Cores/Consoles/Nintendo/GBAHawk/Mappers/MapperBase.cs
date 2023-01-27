using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.GBAHawk;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawk
{
	public class MapperBase
	{
		public GBAHawk Core { get; set; }

		public virtual byte ReadMemory8(uint addr)
		{
			return 0;
		}

		public virtual ushort ReadMemory16(uint addr)
		{
			return 0;
		}

		public virtual uint ReadMemory32(uint addr)
		{
			return 0;
		}

		public virtual byte PeekMemory(uint addr)
		{
			return 0;
		}

		public virtual void WriteMemory8(uint addr, byte value)
		{
		}

		public virtual void WriteMemory16(uint addr, ushort value)
		{
		}

		public virtual void WriteMemory32(uint addr, uint value)
		{
		}

		public virtual void PokeMemory(uint addr, byte value)
		{
		}

		public virtual void SyncState(Serializer ser)
		{
		}

		public virtual void Dispose()
		{
		}

		public virtual void Reset()
		{
		}

		public virtual void Mapper_Tick()
		{
		}

		public virtual byte Mapper_EEPROM_Read()
		{
			return 0xFF;
		}

		public virtual void Mapper_EEPROM_Write(byte value)
		{

		}

		public virtual void RTC_Get(int value, int index)
		{
		}
	}
}
