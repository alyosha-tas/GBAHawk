using BizHawk.Common;
using BizHawk.Emulation.Cores.Nintendo.GBHawk;

namespace BizHawk.Emulation.Cores.Nintendo.GBHawk
{
	public class MapperBase
	{
		public GBHawk Core { get; set; }

		public virtual byte ReadMemoryLow(ushort addr)
		{
			return 0;
		}

		public virtual byte ReadMemoryHigh(ushort addr)
		{
			return 0;
		}

		public virtual byte PeekMemoryLow(ushort addr)
		{
			return 0;
		}

		public virtual byte PeekMemoryHigh(ushort addr)
		{
			return 0;
		}

		public virtual void WriteMemory(ushort addr, byte value)
		{
		}

		public virtual void PokeMemory(ushort addr, byte value)
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

		public virtual void RTC_Get(int value, int index)
		{
		}
	}
}
