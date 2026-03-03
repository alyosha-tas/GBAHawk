using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.NES.Common;
using BizHawk.Emulation.Cores.Nintendo.NESHawk;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.SubNESHawk
{
	[Core(CoreNames.SubNESHawk2, "", isReleased: false)]
	public partial class SubNESHawk : IEmulator, IStatable, IInputPollable, ICycleTiming, ISettable<NESHawk.NESHawk.NESHawkSettings, NESHawk.NESHawk.NESHawkSyncSettings>
	{
		[CoreConstructor(VSystemID.Raw.NES)]
		public SubNESHawk(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ NESHawk.NESHawk.NESHawkSettings settings, NESHawk.NESHawk.NESHawkSyncSettings syncSettings)
		{
			var subNesSettings = settings ?? new NESHawk.NESHawk.NESHawkSettings();
			var subNesSyncSettings = syncSettings ?? new NESHawk.NESHawk.NESHawkSyncSettings();

			_nesCore = new NESHawk.NESHawk(comm, game, rom, subNesSettings, subNesSyncSettings, true);

			HardReset();

			_nesStatable = _nesCore.ServiceProvider.GetService<IStatable>();

			var ser = new BasicServiceProvider(this);
			ServiceProvider = ser;

			ser.Register(_nesCore.ServiceProvider.GetService<IVideoProvider>());
			ser.Register(_nesCore.ServiceProvider.GetService<ISoundProvider>());
			ser.Register(_nesCore.ServiceProvider.GetService<ITraceable>());
			ser.Register(_nesCore.ServiceProvider.GetService<IDisassemblable>());
			ser.Register(_nesCore.ServiceProvider.GetService<IMemoryDomains>());
			ser.Register(_nesCore.ServiceProvider.GetService<INESPPUViewable>());
			ser.Register(_nesCore.ServiceProvider.GetService<ISaveRam>());

			ser.Register(_nesCore.Tracer);
		}

		private readonly NESHawk.NESHawk _nesCore;

		public void HardReset() => _nesCore.HardReset();

		public void SoftReset() => _nesCore.SoftReset();

		private int _frame;

		public double ClockRate => 5369318.18181;

		public ulong CycleCount => LibNESHawk.NES_subframe_cycles(_nesCore.NES_Pntr);

		public NESHawk.NESHawk.NESHawkSettings GetSettings() => _nesCore.GetSettings();
		public NESHawk.NESHawk.NESHawkSyncSettings GetSyncSettings() => _nesCore.GetSyncSettings();
		public PutSettingsDirtyBits PutSettings(NESHawk.NESHawk.NESHawkSettings o) => _nesCore.PutSettings(o);
		public PutSettingsDirtyBits PutSyncSettings(NESHawk.NESHawk.NESHawkSyncSettings o) => _nesCore.PutSyncSettings(o);

		public ulong TotalExecutedCycles => _nesCore.TotalExecutedCycles;
	}
}
