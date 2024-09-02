using System;
using BizHawk.Emulation.Common;
using BizHawk.Emulation.Cores.Nintendo.GBA.Common;

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

		public bool use_sram;

		[CoreConstructor(VSystemID.Raw.GBAL)]
		public GBAHawkLink_Debug(CoreLoadParameters<GBAHawkLink_Debug.GBAHawkLink_Debug_Settings, GBAHawkLink_Debug.GBAHawkLink_Debug_SyncSettings> lp)
		{
			if (lp.Roms.Count != 2)
				throw new InvalidOperationException("Wrong number of roms");

			var ser = new BasicServiceProvider(this);
			ServiceProvider = ser;

			linkSettings = (GBAHawkLink_Debug_Settings)lp.Settings ?? new GBAHawkLink_Debug_Settings();
			linkSyncSettings = (GBAHawkLink_Debug_SyncSettings)lp.SyncSettings ?? new GBAHawkLink_Debug_SyncSettings();
			
			use_sram = linkSyncSettings.Use_SRAM;
			
			_controllerDeck = new(
				GBA_ControllerDeck.DefaultControllerName,
				GBA_ControllerDeck.DefaultControllerName);

			var temp_set_L = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_Settings();
			var temp_set_R = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_Settings();

			var temp_sync_L = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_SyncSettings();
			var temp_sync_R = new GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_SyncSettings();

			temp_sync_L.RTCInitialState = (GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_SyncSettings.InitRTCState)((int)linkSyncSettings.RTCInitialState_L);
			temp_sync_R.RTCInitialState = (GBAHawk_Debug.GBAHawk_Debug.GBAHawk_Debug_SyncSettings.InitRTCState)((int)linkSyncSettings.RTCInitialState_R);
			temp_sync_L.RTCInitialTime = linkSyncSettings.RTCInitialTime_L;
			temp_sync_R.RTCInitialTime = linkSyncSettings.RTCInitialTime_R;
			temp_sync_L.RTCOffset = linkSyncSettings.RTCOffset_L;
			temp_sync_R.RTCOffset = linkSyncSettings.RTCOffset_R;
			temp_sync_L.EEPROMOffset = linkSyncSettings.EEPROMOffset_L;
			temp_sync_R.EEPROMOffset = linkSyncSettings.EEPROMOffset_R;
			temp_sync_L.FlashWriteOffset = linkSyncSettings.FlashWriteOffset_L;
			temp_sync_R.FlashWriteOffset = linkSyncSettings.FlashWriteOffset_R;
			temp_sync_L.FlashChipEraseOffset = linkSyncSettings.FlashChipEraseOffset_L;
			temp_sync_R.FlashChipEraseOffset = linkSyncSettings.FlashChipEraseOffset_R;
			temp_sync_L.FlashSectorEraseOffset = linkSyncSettings.FlashSectorEraseOffset_L;
			temp_sync_R.FlashSectorEraseOffset = linkSyncSettings.FlashSectorEraseOffset_R;
			temp_sync_L.Use_GBP = linkSyncSettings.Use_GBP_L;
			temp_sync_R.Use_GBP = linkSyncSettings.Use_GBP_R;

			L = new GBAHawk_Debug.GBAHawk_Debug(lp.Comm, lp.Roms[0].Game, lp.Roms[0].RomData, temp_set_L, temp_sync_L);
			R = new GBAHawk_Debug.GBAHawk_Debug(lp.Comm, lp.Roms[1].Game, lp.Roms[1].RomData, temp_set_R, temp_sync_R);

			// system starts connected
			L.ext_num = 1;
			R.ext_num = 2;

			// stop mode not allowed for linked systems for now to avoid complicaitons in frame timing
			L.is_linked_system = true;
			R.is_linked_system = true;

			ser.Register<IVideoProvider>(this);
			ser.Register<ISoundProvider>(this); 

			_tracer = new TraceBuffer(L.TraceHeader);
			ser.Register<ITraceable>(_tracer);

			_lStates = L.ServiceProvider.GetService<IStatable>();
			_rStates = R.ServiceProvider.GetService<IStatable>();

			SetupMemoryDomains();

			HardReset();

			// change ser control state since its plugged in
			L.ser_CTRL = 0;
			R.ser_CTRL = 0;
		}

		public ulong TotalExecutedCycles => L.TotalExecutedCycles;

		public void HardReset()
		{
			L.HardReset();
			R.HardReset();
		}

		public DisplayType Region => DisplayType.NTSC;

		public int _frame = 0;

		private readonly GBALink_ControllerDeck _controllerDeck;

		private readonly ITraceable _tracer;

		public bool LinkConnected
		{
			get => _cableconnected;
			set => _cableconnected = value;
		}
	}
}
