using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GB.Common;
using BizHawk.Emulation.Cores.Nintendo.GBCHawk;
using System;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBCHawk
{
	[Core(CoreNames.SubGBCHawk, isReleased: true)]
	public partial class SubGBCHawk : IEmulator, IStatable, IInputPollable, ICycleTiming, ISettable<GBCHawk.GBCHawk.GBCHawkSettings, GBCHawk.GBCHawk.GBCHawkSyncSettings>
	{
		[CoreConstructor(VSystemID.Raw.GB)]
		[CoreConstructor(VSystemID.Raw.GBC)]
		public SubGBCHawk(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ GBCHawk.GBCHawk.GBCHawkSettings settings, GBCHawk.GBCHawk.GBCHawkSyncSettings syncSettings)
		{
			var SubGBSettings = settings ?? new GBCHawk.GBCHawk.GBCHawkSettings();
			var SubGBSyncSettings = syncSettings ?? new GBCHawk.GBCHawk.GBCHawkSyncSettings();

			_gbcCore = new GBCHawk.GBCHawk(comm, game, rom, SubGBSettings, SubGBSyncSettings, true);

			HardReset();

			_gbStatable = _gbcCore.ServiceProvider.GetService<IStatable>();

			var ser = new BasicServiceProvider(this);
			ServiceProvider = ser;

			ser.Register(_gbcCore.ServiceProvider.GetService<IVideoProvider>());
			ser.Register(_gbcCore.ServiceProvider.GetService<ISoundProvider>());
			ser.Register(_gbcCore.ServiceProvider.GetService<ITraceable>());
			ser.Register(_gbcCore.ServiceProvider.GetService<IMemoryDomains>());
			ser.Register(_gbcCore.ServiceProvider.GetService<IGBPPUViewable>());
			ser.Register(_gbcCore.ServiceProvider.GetService<ISaveRam>());

			ser.Register(_gbcCore.Tracer);
		}

		private readonly GBCHawk.GBCHawk _gbcCore;

		public void HardReset() => _gbcCore.HardReset();

		private int _frame;

		public double ClockRate => 4194304;

		public ulong CycleCount => LibGBCHawk.GBC_subframe_cycles(_gbcCore.GBC_Pntr);

		public GBCHawk.GBCHawk.GBCHawkSettings GetSettings() => _gbcCore.GetSettings();
		public GBCHawk.GBCHawk.GBCHawkSyncSettings GetSyncSettings() => _gbcCore.GetSyncSettings();
		public PutSettingsDirtyBits PutSettings(GBCHawk.GBCHawk.GBCHawkSettings o) => _gbcCore.PutSettings(o);
		public PutSettingsDirtyBits PutSyncSettings(GBCHawk.GBCHawk.GBCHawkSyncSettings o) => _gbcCore.PutSyncSettings(o);

		public ulong TotalExecutedCycles => _gbcCore.TotalExecutedCycles;
	}
}
