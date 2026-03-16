using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Cores.Nintendo.GBHawk;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBHawk
{
	[Core(CoreNames.SubGBHawk, "", isReleased: false)]
	public partial class SubGBHawk : IEmulator, IStatable, IInputPollable, ICycleTiming, ISettable<GBHawk.GBHawk.GBHawkSettings, GBHawk.GBHawk.GBHawkSyncSettings>
	{
		[CoreConstructor(VSystemID.Raw.GB)]
		[CoreConstructor(VSystemID.Raw.GBC)]
		public SubGBHawk(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ GBHawk.GBHawk.GBHawkSettings settings, GBHawk.GBHawk.GBHawkSyncSettings syncSettings)
		{
			var SubGBSettings = settings ?? new GBHawk.GBHawk.GBHawkSettings();
			var SubGBSyncSettings = syncSettings ?? new GBHawk.GBHawk.GBHawkSyncSettings();

			_gbCore = new GBHawk.GBHawk(comm, game, rom, SubGBSettings, SubGBSyncSettings, true);

			HardReset();

			_gbStatable = _gbCore.ServiceProvider.GetService<IStatable>();

			var ser = new BasicServiceProvider(this);
			ServiceProvider = ser;

			ser.Register(_gbCore.ServiceProvider.GetService<IVideoProvider>());
			ser.Register(_gbCore.ServiceProvider.GetService<ISoundProvider>());
			ser.Register(_gbCore.ServiceProvider.GetService<ITraceable>());
			ser.Register(_gbCore.ServiceProvider.GetService<IDisassemblable>());
			ser.Register(_gbCore.ServiceProvider.GetService<IMemoryDomains>());
			ser.Register(_gbCore.ServiceProvider.GetService<IGBPPUViewable>());
			ser.Register(_gbCore.ServiceProvider.GetService<ISaveRam>());

			ser.Register(_gbCore.Tracer);
		}

		private readonly GBHawk.GBHawk _gbCore;

		public void HardReset() => _gbCore.HardReset();

		private int _frame;

		public double ClockRate => 4194304;

		public ulong CycleCount => LibGBHawk.GB_subframe_cycles(_gbCore.GB_Pntr);

		public GBHawk.GBHawk.GBHawkSettings GetSettings() => _gbCore.GetSettings();
		public GBHawk.GBHawk.GBHawkSyncSettings GetSyncSettings() => _gbCore.GetSyncSettings();
		public PutSettingsDirtyBits PutSettings(GBHawk.GBHawk.GBHawkSettings o) => _gbCore.PutSettings(o);
		public PutSettingsDirtyBits PutSyncSettings(GBHawk.GBHawk.GBHawkSyncSettings o) => _gbCore.PutSyncSettings(o);

		public ulong TotalExecutedCycles => _gbCore.TotalExecutedCycles;
	}
}
