using System;
using System.Collections.Generic;
using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Consoles.Nintendo.Gameboy;

namespace BizHawk.Emulation.Cores.Nintendo.SubGBHawk
{
	[Core(CoreNames.SubGBHawk, "")]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class SubGBHawk : IEmulator, IStatable, IInputPollable,
		ISettable<GBHawkOld.GBHawkOld.GBSettings, GBHawkOld.GBHawkOld.GBSyncSettings>, ICycleTiming
	{
		[CoreConstructor(VSystemID.Raw.GB, Priority = CorePriority.SuperLow)]
		[CoreConstructor(VSystemID.Raw.GBC, Priority = CorePriority.SuperLow)]
		public SubGBHawk(CoreComm comm, GameInfo game, byte[] rom, /*string gameDbFn,*/ GBHawkOld.GBHawkOld.GBSettings settings, GBHawkOld.GBHawkOld.GBSyncSettings syncSettings)
		{
			
			var subGBSettings = (GBHawkOld.GBHawkOld.GBSettings)settings ?? new GBHawkOld.GBHawkOld.GBSettings();
			var subGBSyncSettings = (GBHawkOld.GBHawkOld.GBSyncSettings)syncSettings ?? new GBHawkOld.GBHawkOld.GBSyncSettings();

			_GBCore = new GBHawkOld.GBHawkOld(comm, game, rom, subGBSettings, subGBSyncSettings, true);

			HardReset();
			current_cycle = 0;
			_cycleCount = 0;

			_GBStatable = _GBCore.ServiceProvider.GetService<IStatable>();

			var ser = new BasicServiceProvider(this);
			ServiceProvider = ser;

			ser.Register(_GBCore.ServiceProvider.GetService<IVideoProvider>());
			ser.Register(_GBCore.ServiceProvider.GetService<ISoundProvider>());
			ser.Register(_GBCore.ServiceProvider.GetService<ITraceable>());
			ser.Register(_GBCore.ServiceProvider.GetService<IMemoryDomains>());
			ser.Register(_GBCore.ServiceProvider.GetService<ISaveRam>());
			ser.Register(_GBCore.ServiceProvider.GetService<IRegionable>());
			ser.Register(_GBCore.ServiceProvider.GetService<IGameboyCommon>());

			_tracer = new TraceBuffer(_GBCore.cpu.TraceHeader);
			ser.Register(_tracer);
		}

		public GBHawkOld.GBHawkOld _GBCore;

		// needed for movies to accurately calculate timing
		private ulong _cycleCount;

		public ulong CycleCount => _cycleCount;

		public double ClockRate => 4194304;

		public void HardReset() => _GBCore.HardReset();

		private int _frame;

		private readonly ITraceable _tracer;

		public GBHawkOld.GBHawkOld.GBSettings GetSettings() => _GBCore.GetSettings();
		public GBHawkOld.GBHawkOld.GBSyncSettings GetSyncSettings() => _GBCore.GetSyncSettings();
		public PutSettingsDirtyBits PutSettings(GBHawkOld.GBHawkOld.GBSettings o) => _GBCore.PutSettings(o);
		public PutSettingsDirtyBits PutSyncSettings(GBHawkOld.GBHawkOld.GBSyncSettings o) => _GBCore.PutSyncSettings(o);

		public ulong TotalExecutedCycles => _GBCore.cpu.TotalExecutedCycles;
	}
}
