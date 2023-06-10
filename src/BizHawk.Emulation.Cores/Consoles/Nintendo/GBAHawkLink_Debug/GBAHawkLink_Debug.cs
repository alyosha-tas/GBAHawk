using System;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBAHawk_Debug;

namespace BizHawk.Emulation.Cores.Nintendo.GBAHawkLink_Debug
{
	[Core(CoreNames.GBAHawkLink_Debug, "")]
	[ServiceNotApplicable(new[] { typeof(IDriveLight) })]
	public partial class GBAHawkLink_Debug : IEmulator, ISaveRam, IStatable, IInputPollable, IRegionable, ILinkable,
		ISettable<GBAHawkLink_Debug.GBAHawkLink_Debug_Settings, GBAHawkLink_Debug.GBAHawkLink_Debug_SyncSettings>
	{
		// two instances
		public GBAHawk_Debug.GBAHawk_Debug L;
		public GBAHawk_Debug.GBAHawk_Debug R;

		// if true, the link cable is currently connected
		private bool _cableconnected = true;

		// if true, the link cable toggle signal is currently asserted
		private bool _cablediscosignal = false;

		private bool do_r_next = false;

		public bool do_frame_fill;

		[CoreConstructor(VSystemID.Raw.GBAL)]
		public GBAHawkLink_Debug(CoreLoadParameters<GBAHawkLink_Debug.GBAHawkLink_Debug_Settings, GBAHawkLink_Debug.GBAHawkLink_Debug_SyncSettings> lp)
		{
			if (lp.Roms.Count != 2)
				throw new InvalidOperationException("Wrong number of roms");

			var ser = new BasicServiceProvider(this);
			ServiceProvider = ser;

			linkSettings = (GBAHawkLink_Debug_Settings)lp.Settings ?? new GBAHawkLink_Debug_Settings();
			linkSyncSettings = (GBAHawkLink_Debug_SyncSettings)lp.SyncSettings ?? new GBAHawkLink_Debug_SyncSettings();
			_controllerDeck = new(
				GBAHawk_Debug_ControllerDeck.DefaultControllerName,
				GBAHawk_Debug_ControllerDeck.DefaultControllerName);

			var temp_set_L = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_Settings();
			var temp_set_R = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_Settings();

			var temp_sync_L = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_SyncSettings();
			var temp_sync_R = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_SyncSettings();

			temp_sync_L.RTCInitialTime = linkSyncSettings.RTCInitialTime_L;
			temp_sync_R.RTCInitialTime = linkSyncSettings.RTCInitialTime_R;
			temp_sync_L.RTCOffset = linkSyncSettings.RTCOffset_L;
			temp_sync_R.RTCOffset = linkSyncSettings.RTCOffset_R;

			L = new GBAHawk_Debug.GBAHawk_Debug(lp.Comm, lp.Roms[0].Game, lp.Roms[0].RomData, temp_set_L, temp_sync_L);
			R = new GBAHawk_Debug.GBAHawk_Debug(lp.Comm, lp.Roms[1].Game, lp.Roms[1].RomData, temp_set_R, temp_sync_R);

			L.ext_name = "L";
			R.ext_name = "R";

			ser.Register<IVideoProvider>(this);
			ser.Register<ISoundProvider>(this); 

			_tracer = new TraceBuffer(L.TraceHeader);
			ser.Register<ITraceable>(_tracer);

			_lStates = L.ServiceProvider.GetService<IStatable>();
			_rStates = R.ServiceProvider.GetService<IStatable>();

			SetupMemoryDomains();

			HardReset();
		}

		public ulong TotalExecutedCycles => L.TotalExecutedCycles;

		public void HardReset()
		{
			L.HardReset();
			R.HardReset();
		}

		public DisplayType Region => DisplayType.NTSC;

		public int _frame = 0;

		private readonly GBAHawkLink_Debug_ControllerDeck _controllerDeck;

		private readonly ITraceable _tracer;

		public bool LinkConnected
		{
			get => _cableconnected;
			set => _cableconnected = value;
		}
	}
}
