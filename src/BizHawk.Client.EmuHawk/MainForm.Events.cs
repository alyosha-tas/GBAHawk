using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Windows.Forms;

using BizHawk.Bizware.DirectX;
using BizHawk.Bizware.OpenTK3;
using BizHawk.Client.Common;
using BizHawk.Client.GBAHawk.CustomControls;
using BizHawk.Client.GBAHawk.ToolExtensions;
using BizHawk.Common;
using BizHawk.Emulation.Common;
using BizHawk.WinForms.Controls;

namespace BizHawk.Client.GBAHawk
{
	public partial class MainForm
	{
		private void FileSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			SaveStateSubMenu.Enabled =
				LoadStateSubMenu.Enabled =
				SaveSlotSubMenu.Enabled =
				Emulator.HasSavestates();

			OpenRomMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Open ROM"].Bindings;
			CloseRomMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Close ROM"].Bindings;

			MovieSubMenu.Enabled =
				CloseRomMenuItem.Enabled =
				!Emulator.IsNull();

			var hasSaveRam = Emulator.HasSaveRam();
			bool needBold = hasSaveRam && Emulator.AsSaveRam().SaveRamModified && Config.CurrentUseExistingSRAM;

			SaveRAMSubMenu.Enabled = hasSaveRam;
			SaveRAMSubMenu.SetStyle(needBold ? FontStyle.Bold : FontStyle.Regular);

			AVSubMenu.Enabled =
			ScreenshotSubMenu.Enabled =
				Emulator.HasVideoProvider();
		}

		private void RecentRomMenuItem_DropDownOpened(object sender, EventArgs e)
		{
			RecentRomSubMenu.DropDownItems.Clear();
			RecentRomSubMenu.DropDownItems.AddRange(Config.RecentRoms.RecentMenu(this, LoadRomFromRecent, "ROM", romLoading: true));
		}

		private bool HasSlot(int slot) => _stateSlots.HasSlot(Emulator, MovieSession.Movie, slot, SaveStatePrefix());

		private void SaveStateSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			void SetSlotFont(ToolStripMenuItemEx menu, int slot) => menu.SetStyle(
				HasSlot(slot) ? (FontStyle.Italic | FontStyle.Bold) : FontStyle.Regular);

			SetSlotFont(SaveState1MenuItem, 1);
			SetSlotFont(SaveState2MenuItem, 2);
			SetSlotFont(SaveState3MenuItem, 3);
			SetSlotFont(SaveState4MenuItem, 4);
			SetSlotFont(SaveState5MenuItem, 5);
			SetSlotFont(SaveState6MenuItem, 6);
			SetSlotFont(SaveState7MenuItem, 7);
			SetSlotFont(SaveState8MenuItem, 8);
			SetSlotFont(SaveState9MenuItem, 9);
			SetSlotFont(SaveState0MenuItem, 0);

			SaveState1MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 1"].Bindings;
			SaveState2MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 2"].Bindings;
			SaveState3MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 3"].Bindings;
			SaveState4MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 4"].Bindings;
			SaveState5MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 5"].Bindings;
			SaveState6MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 6"].Bindings;
			SaveState7MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 7"].Bindings;
			SaveState8MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 8"].Bindings;
			SaveState9MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 9"].Bindings;
			SaveState0MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save State 0"].Bindings;
			SaveNamedStateMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save Named State"].Bindings;
		}

		private void LoadStateSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			LoadState1MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 1"].Bindings;
			LoadState2MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 2"].Bindings;
			LoadState3MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 3"].Bindings;
			LoadState4MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 4"].Bindings;
			LoadState5MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 5"].Bindings;
			LoadState6MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 6"].Bindings;
			LoadState7MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 7"].Bindings;
			LoadState8MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 8"].Bindings;
			LoadState9MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 9"].Bindings;
			LoadState0MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load State 0"].Bindings;
			LoadNamedStateMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Load Named State"].Bindings;

			AutoloadLastSlotMenuItem.Checked = Config.AutoLoadLastSaveSlot;

			LoadState1MenuItem.Enabled = HasSlot(1);
			LoadState2MenuItem.Enabled = HasSlot(2);
			LoadState3MenuItem.Enabled = HasSlot(3);
			LoadState4MenuItem.Enabled = HasSlot(4);
			LoadState5MenuItem.Enabled = HasSlot(5);
			LoadState6MenuItem.Enabled = HasSlot(6);
			LoadState7MenuItem.Enabled = HasSlot(7);
			LoadState8MenuItem.Enabled = HasSlot(8);
			LoadState9MenuItem.Enabled = HasSlot(9);
			LoadState0MenuItem.Enabled = HasSlot(0);
		}

		private void SaveSlotSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			SelectSlot1MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 1"].Bindings;
			SelectSlot2MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 2"].Bindings;
			SelectSlot3MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 3"].Bindings;
			SelectSlot4MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 4"].Bindings;
			SelectSlot5MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 5"].Bindings;
			SelectSlot6MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 6"].Bindings;
			SelectSlot7MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 7"].Bindings;
			SelectSlot8MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 8"].Bindings;
			SelectSlot9MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 9"].Bindings;
			SelectSlot0MenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Select State 0"].Bindings;
			PreviousSlotMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Previous Slot"].Bindings;
			NextSlotMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Next Slot"].Bindings;
			SaveToCurrentSlotMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Quick Save"].Bindings;
			LoadCurrentSlotMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Quick Load"].Bindings;

			SelectSlot1MenuItem.Checked = Config.SaveSlot == 1;
			SelectSlot2MenuItem.Checked = Config.SaveSlot == 2;
			SelectSlot3MenuItem.Checked = Config.SaveSlot == 3;
			SelectSlot4MenuItem.Checked = Config.SaveSlot == 4;
			SelectSlot5MenuItem.Checked = Config.SaveSlot == 5;
			SelectSlot6MenuItem.Checked = Config.SaveSlot == 6;
			SelectSlot7MenuItem.Checked = Config.SaveSlot == 7;
			SelectSlot8MenuItem.Checked = Config.SaveSlot == 8;
			SelectSlot9MenuItem.Checked = Config.SaveSlot == 9;
			SelectSlot0MenuItem.Checked = Config.SaveSlot == 0;
		}

		private void SaveRamSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			FlushSaveRAMMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Flush SaveRAM"].Bindings;
		}

		private void MovieSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			StopMovieWithoutSavingMenuItem.Enabled = MovieSession.Movie.IsActive() && MovieSession.Movie.Changes;
			StopMovieMenuItem.Enabled
				= SaveMovieMenuItem.Enabled
				= SaveMovieAsMenuItem.Enabled
				= MovieSession.Movie.IsActive();

			ReadonlyMenuItem.Checked = MovieSession.ReadOnly;
			AutomaticallyBackupMoviesMenuItem.Checked = Config.Movies.EnableBackupMovies;
			FullMovieLoadstatesMenuItem.Checked = Config.Movies.VBAStyleMovieLoadState;

			ReadonlyMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Toggle read-only"].Bindings;
			RecordMovieMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Record Movie"].Bindings;
			PlayMovieMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Play Movie"].Bindings;
			StopMovieMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Stop Movie"].Bindings;
			PlayFromBeginningMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Play from beginning"].Bindings;
			SaveMovieMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Save Movie"].Bindings;

			PlayMovieMenuItem.Enabled
				= ImportMoviesMenuItem.Enabled
				= RecentMovieSubMenu.Enabled
					= !Tools.IsLoaded<TAStudio>();

			// Record movie dialog should not be opened while in need of a reboot,
			// Otherwise the wrong sync settings could be set for the recording movie and cause crashes
			RecordMovieMenuItem.Enabled = !Tools.IsLoaded<TAStudio>()
				&& RebootStatusBarIcon.Visible == false;

			PlayFromBeginningMenuItem.Enabled = MovieSession.Movie.IsActive() && !Tools.IsLoaded<TAStudio>();
		}

		private void RecentMovieSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			RecentMovieSubMenu.DropDownItems.Clear();
			RecentMovieSubMenu.DropDownItems.AddRange(Config.RecentMovies.RecentMenu(this, LoadMoviesFromRecent, "Movie"));
		}

		private void MovieEndSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			MovieEndFinishMenuItem.Checked = Config.Movies.MovieEndAction == MovieEndAction.Finish;
			MovieEndRecordMenuItem.Checked = Config.Movies.MovieEndAction == MovieEndAction.Record;
			MovieEndStopMenuItem.Checked = Config.Movies.MovieEndAction == MovieEndAction.Stop;
			MovieEndPauseMenuItem.Checked = Config.Movies.MovieEndAction == MovieEndAction.Pause;

			// Arguably an IControlMainForm property should be set here, but in reality only Tastudio is ever going to interfere with this logic
			MovieEndFinishMenuItem.Enabled =
			MovieEndRecordMenuItem.Enabled =
			MovieEndStopMenuItem.Enabled =
			MovieEndPauseMenuItem.Enabled =
				!Tools.Has<TAStudio>();
		}

		private void AVSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			ConfigAndRecordAVMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Record A/V"].Bindings;
			StopAVIMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Stop A/V"].Bindings;
			CaptureOSDMenuItem.Checked = Config.AviCaptureOsd;
			CaptureLuaMenuItem.Checked = Config.AviCaptureLua || Config.AviCaptureOsd; // or with osd is for better compatibility with old config files

			RecordAVMenuItem.Enabled = !string.IsNullOrEmpty(Config.VideoWriter) && _currAviWriter == null;

			if (_currAviWriter == null)
			{
				ConfigAndRecordAVMenuItem.Enabled = true;
				StopAVIMenuItem.Enabled = false;
			}
			else
			{
				ConfigAndRecordAVMenuItem.Enabled = false;
				StopAVIMenuItem.Enabled = true;
			}
		}

		private void ScreenshotSubMenu_DropDownOpening(object sender, EventArgs e)
		{
			ScreenshotCaptureOSDMenuItem1.Checked = Config.ScreenshotCaptureOsd;
			ScreenshotMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Screenshot"].Bindings;
			ScreenshotClipboardMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["ScreenshotToClipboard"].Bindings;
			ScreenshotClientClipboardMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Screen Client to Clipboard"].Bindings;
		}

		private void OpenRomMenuItem_Click(object sender, EventArgs e)
		{
			OpenRom();
		}

		private void CloseRomMenuItem_Click(object sender, EventArgs e)
		{
			Console.WriteLine($"Closing rom clicked Frame: {Emulator.Frame} Emulator: {Emulator.GetType().Name}");
			CloseRom();
			Console.WriteLine($"Closing rom clicked DONE Frame: {Emulator.Frame} Emulator: {Emulator.GetType().Name}");
		}

		private void Savestate1MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave1");
		private void Savestate2MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave2");
		private void Savestate3MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave3");
		private void Savestate4MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave4");
		private void Savestate5MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave5");
		private void Savestate6MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave6");
		private void Savestate7MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave7");
		private void Savestate8MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave8");
		private void Savestate9MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave9");
		private void Savestate0MenuItem_Click(object sender, EventArgs e) => SaveQuickSave("QuickSave0");

		private void SaveNamedStateMenuItem_Click(object sender, EventArgs e) => SaveStateAs();

		private void Loadstate1MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave1");
		private void Loadstate2MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave2");
		private void Loadstate3MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave3");
		private void Loadstate4MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave4");
		private void Loadstate5MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave5");
		private void Loadstate6MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave6");
		private void Loadstate7MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave7");
		private void Loadstate8MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave8");
		private void Loadstate9MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave9");
		private void Loadstate0MenuItem_Click(object sender, EventArgs e) => LoadQuickSave("QuickSave0");

		private void LoadNamedStateMenuItem_Click(object sender, EventArgs e) => LoadStateAs();

		private void AutoloadLastSlotMenuItem_Click(object sender, EventArgs e)
		{
			Config.AutoLoadLastSaveSlot ^= true;
		}

		private void SelectSlotMenuItems_Click(object sender, EventArgs e)
		{
			if (sender == SelectSlot1MenuItem) Config.SaveSlot = 1;
			else if (sender == SelectSlot2MenuItem) Config.SaveSlot = 2;
			else if (sender == SelectSlot3MenuItem) Config.SaveSlot = 3;
			else if (sender == SelectSlot4MenuItem) Config.SaveSlot = 4;
			else if (sender == SelectSlot5MenuItem) Config.SaveSlot = 5;
			else if (sender == SelectSlot6MenuItem) Config.SaveSlot = 6;
			else if (sender == SelectSlot7MenuItem) Config.SaveSlot = 7;
			else if (sender == SelectSlot8MenuItem) Config.SaveSlot = 8;
			else if (sender == SelectSlot9MenuItem) Config.SaveSlot = 9;
			else if (sender == SelectSlot0MenuItem) Config.SaveSlot = 0;

			UpdateStatusSlots();
			SaveSlotSelectedMessage();
		}

		private void PreviousSlotMenuItem_Click(object sender, EventArgs e)
		{
			PreviousSlot();
		}

		private void NextSlotMenuItem_Click(object sender, EventArgs e)
		{
			NextSlot();
		}

		private void SaveToCurrentSlotMenuItem_Click(object sender, EventArgs e)
		{
			SaveQuickSave($"QuickSave{Config.SaveSlot}");
		}

		private void LoadCurrentSlotMenuItem_Click(object sender, EventArgs e)
		{
			LoadQuickSave($"QuickSave{Config.SaveSlot}");
		}

		private void FlushSaveRAMMenuItem_Click(object sender, EventArgs e)
		{
			FlushSaveRAM();
		}

		private void ReadonlyMenuItem_Click(object sender, EventArgs e)
		{
			ToggleReadOnly();
		}

		private void RecordMovieMenuItem_Click(object sender, EventArgs e)
		{
			using var form = new RecordMovie(this, Config, Game, Emulator, MovieSession, FirmwareManager);
			form.ShowDialog();
		}

		private string CanProvideFirmware(FirmwareID id, string hash)
			=> FirmwareManager.Resolve(
				Config.PathEntries,
				Config.FirmwareUserSpecifications,
				FirmwareDatabase.FirmwareRecords.First(fr => fr.ID == id),
//				exactFile: hash, //TODO re-scan FW dir for this file, then try autopatching
				forbidScan: true)?.Hash;

		private void PlayMovieMenuItem_Click(object sender, EventArgs e)
		{
			using var form = new PlayMovie(this, Config, Game, Emulator, MovieSession, CanProvideFirmware);
			form.ShowDialog();
		}

		private void StopMovieMenuItem_Click(object sender, EventArgs e)
		{
			StopMovie();
		}

		private void PlayFromBeginningMenuItem_Click(object sender, EventArgs e)
		{
			RestartMovie();
		}

		private void ImportMovieMenuItem_Click(object sender, EventArgs e)
		{
			using var ofd = new OpenFileDialog
			{
				InitialDirectory = Config.PathEntries.RomAbsolutePath(Emulator.SystemId),
				Multiselect = true,
				Filter = MovieImport.AvailableImporters.ToString("Movie Files"),
				RestoreDirectory = false
			};

			if (this.ShowDialogWithTempMute(ofd).IsOk())
			{
				foreach (var fn in ofd.FileNames)
				{
					ProcessMovieImport(fn, false);
				}
			}
		}

		private void SaveMovieMenuItem_Click(object sender, EventArgs e)
		{
			SaveMovie();
		}

		private void SaveMovieAsMenuItem_Click(object sender, EventArgs e)
		{
			var filename = MovieSession.Movie.Filename;
			if (string.IsNullOrWhiteSpace(filename))
			{
				filename = Game.FilesystemSafeName();
			}

			var file = ToolFormBase.SaveFileDialog(
				filename,
				Config.PathEntries.MovieAbsolutePath(),
				"Movie Files",
				MovieSession.Movie.PreferredExtension,
				this);

			if (file != null)
			{
				MovieSession.Movie.Filename = file.FullName;
				Config.RecentMovies.Add(MovieSession.Movie.Filename);
				SaveMovie();
			}
		}

		private void StopMovieWithoutSavingMenuItem_Click(object sender, EventArgs e)
		{
			if (Config.Movies.EnableBackupMovies)
			{
				MovieSession.Movie.SaveBackup();
			}

			StopMovie(saveChanges: false);
		}

		private void AutomaticMovieBackupMenuItem_Click(object sender, EventArgs e)
		{
			Config.Movies.EnableBackupMovies ^= true;
		}

		private void FullMovieLoadstatesMenuItem_Click(object sender, EventArgs e)
		{
			Config.Movies.VBAStyleMovieLoadState ^= true;
		}

		private void MovieEndFinishMenuItem_Click(object sender, EventArgs e)
		{
			Config.Movies.MovieEndAction = MovieEndAction.Finish;
		}

		private void MovieEndRecordMenuItem_Click(object sender, EventArgs e)
		{
			Config.Movies.MovieEndAction = MovieEndAction.Record;
		}

		private void MovieEndStopMenuItem_Click(object sender, EventArgs e)
		{
			Config.Movies.MovieEndAction = MovieEndAction.Stop;
		}

		private void MovieEndPauseMenuItem_Click(object sender, EventArgs e)
		{
			Config.Movies.MovieEndAction = MovieEndAction.Pause;
		}

		private void ConfigAndRecordAVMenuItem_Click(object sender, EventArgs e)
		{
			RecordAv();
		}

		private void RecordAVMenuItem_Click(object sender, EventArgs e)
		{
			RecordAv(null, null); // force unattended, but allow traditional setup
		}

		private void StopAVMenuItem_Click(object sender, EventArgs e)
		{
			StopAv();
		}

		private void CaptureOSDMenuItem_Click(object sender, EventArgs e)
		{
			bool c = ((ToolStripMenuItem)sender).Checked;
			Config.AviCaptureOsd = c;
			if (c) // Logic to capture OSD w/o Lua does not currently exist, so disallow that.
				Config.AviCaptureLua = true;
		}

		private void CaptureLuaMenuItem_Click(object sender, EventArgs e)
		{
			bool c = ((ToolStripMenuItem)sender).Checked;
			Config.AviCaptureLua = c;
			if (!c) // Logic to capture OSD w/o Lua does not currently exist, so disallow that.
				Config.AviCaptureOsd = false;
		}

		private void ScreenshotMenuItem_Click(object sender, EventArgs e)
		{
			TakeScreenshot();
		}

		private void ScreenshotAsMenuItem_Click(object sender, EventArgs e)
		{
			var path = $"{ScreenshotPrefix()}.{DateTime.Now:yyyy-MM-dd HH.mm.ss}.png";

			using var sfd = new SaveFileDialog
			{
				InitialDirectory = Path.GetDirectoryName(path),
				FileName = Path.GetFileName(path),
				Filter = FilesystemFilter.PNGs.ToString()
			};

			if (this.ShowDialogWithTempMute(sfd).IsOk())
			{
				TakeScreenshot(sfd.FileName);
			}
		}

		private void ScreenshotClipboardMenuItem_Click(object sender, EventArgs e)
		{
			TakeScreenshotToClipboard();
		}

		private void ScreenshotClientClipboardMenuItem_Click(object sender, EventArgs e)
		{
			TakeScreenshotClientToClipboard();
		}

		private void ScreenshotCaptureOSDMenuItem_Click(object sender, EventArgs e)
		{
			Config.ScreenshotCaptureOsd ^= true;
		}

		private void ExitMenuItem_Click(object sender, EventArgs e)
		{
			if (Tools.AskSave())
			{
				Close();
			}
		}

		public void CloseEmulator(int? exitCode = null)
		{
			_exitRequestPending = true;
			if (exitCode != null) _exitCode = exitCode.Value;
		}

		private void EmulationMenuItem_DropDownOpened(object sender, EventArgs e)
		{
			PauseMenuItem.Checked = _didMenuPause ? _wasPaused : EmulatorPaused;

			SoftResetMenuItem.Enabled = Emulator.ControllerDefinition.BoolButtons.Contains("Reset")
				&& !MovieSession.Movie.IsPlaying();

			HardResetMenuItem.Enabled = Emulator.ControllerDefinition.BoolButtons.Contains("Power")
				&& !MovieSession.Movie.IsPlaying();

			PauseMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Pause"].Bindings;
			RebootCoreMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Reboot Core"].Bindings;
			SoftResetMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Soft Reset"].Bindings;
			HardResetMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Hard Reset"].Bindings;
		}

		private void PauseMenuItem_Click(object sender, EventArgs e)
		{
			if (Config.PauseWhenMenuActivated && sender == PauseMenuItem)
			{
				const string ERR_MSG = nameof(PauseMenuItem_Click) + " ran before " + nameof(MaybeUnpauseFromMenuClosed) + "?";
				Debug.Assert(EmulatorPaused == _wasPaused, ERR_MSG);
				// fall through
			}
			TogglePause();
		}

		private void PowerMenuItem_Click(object sender, EventArgs e)
		{
			RebootCore();
		}

		private void SoftResetMenuItem_Click(object sender, EventArgs e)
		{
			SoftReset();
		}

		private void HardResetMenuItem_Click(object sender, EventArgs e)
		{
			HardReset();
		}

		private void ViewSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			DisplayFPSMenuItem.Checked = Config.DisplayFps;
			DisplayFrameCounterMenuItem.Checked = Config.DisplayFrameCounter;
			DisplayLagCounterMenuItem.Checked = Config.DisplayLagCounter;
			DisplayInputMenuItem.Checked = Config.DisplayInput;
			DisplayRerecordCountMenuItem.Checked = Config.DisplayRerecordCount;
			DisplaySubtitlesMenuItem.Checked = Config.DisplaySubtitles;

			DisplayFPSMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Display FPS"].Bindings;
			DisplayFrameCounterMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Frame Counter"].Bindings;
			DisplayLagCounterMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Lag Counter"].Bindings;
			DisplayInputMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Input Display"].Bindings;

			DisplayStatusBarMenuItem.Checked = Config.DispChromeStatusBarWindowed;
			DisplayLogWindowMenuItem.Checked = Tools.IsLoaded<LogWindow>();

			DisplayLagCounterMenuItem.Enabled = Emulator.CanPollInput();

			DisplayMessagesMenuItem.Checked = Config.DisplayMessages;
		}

		private void WindowSizeSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			x1MenuItem.Checked =
				x2MenuItem.Checked =
				x3MenuItem.Checked =
				x4MenuItem.Checked =
				x5MenuItem.Checked = false;

			switch (Config.TargetZoomFactors[Emulator.SystemId])
			{
				case 1:
					x1MenuItem.Checked = true;
					break;
				case 2:
					x2MenuItem.Checked = true;
					break;
				case 3:
					x3MenuItem.Checked = true;
					break;
				case 4:
					x4MenuItem.Checked = true;
					break;
				case 5:
					x5MenuItem.Checked = true;
					break;
			}
		}

		private void WindowSize_Click(object sender, EventArgs e)
		{
			if (sender == x1MenuItem) Config.TargetZoomFactors[Emulator.SystemId] = 1;
			if (sender == x2MenuItem) Config.TargetZoomFactors[Emulator.SystemId] = 2;
			if (sender == x3MenuItem) Config.TargetZoomFactors[Emulator.SystemId] = 3;
			if (sender == x4MenuItem) Config.TargetZoomFactors[Emulator.SystemId] = 4;
			if (sender == x5MenuItem) Config.TargetZoomFactors[Emulator.SystemId] = 5;

			FrameBufferResized();
		}

		private void DisplayFpsMenuItem_Click(object sender, EventArgs e)
		{
			ToggleFps();
		}

		private void DisplayFrameCounterMenuItem_Click(object sender, EventArgs e)
		{
			ToggleFrameCounter();
		}

		private void DisplayLagCounterMenuItem_Click(object sender, EventArgs e)
		{
			ToggleLagCounter();
		}

		private void DisplayInputMenuItem_Click(object sender, EventArgs e)
		{
			ToggleInputDisplay();
		}

		private void DisplayRerecordsMenuItem_Click(object sender, EventArgs e)
		{
			Config.DisplayRerecordCount ^= true;
		}

		private void DisplaySubtitlesMenuItem_Click(object sender, EventArgs e)
		{
			Config.DisplaySubtitles ^= true;
		}

		private void DisplayStatusBarMenuItem_Click(object sender, EventArgs e)
		{
			Config.DispChromeStatusBarWindowed ^= true;
			SetStatusBar();
		}

		private void DisplayMessagesMenuItem_Click(object sender, EventArgs e)
		{
			Config.DisplayMessages ^= true;
		}

		private void DisplayLogWindowMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<LogWindow>();
		}

		private void ConfigSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			ControllersMenuItem.Enabled = Emulator.ControllerDefinition.Any();
			RewindOptionsMenuItem.Enabled = Emulator.HasSavestates();
		}

		private void FrameSkipMenuItem_DropDownOpened(object sender, EventArgs e)
		{
			MinimizeSkippingMenuItem.Checked = Config.AutoMinimizeSkipping;
			ClockThrottleMenuItem.Checked = Config.ClockThrottle;
			VsyncThrottleMenuItem.Checked = Config.VSyncThrottle;
			NeverSkipMenuItem.Checked = Config.FrameSkip == 0;
			Frameskip1MenuItem.Checked = Config.FrameSkip == 1;
			Frameskip2MenuItem.Checked = Config.FrameSkip == 2;
			Frameskip3MenuItem.Checked = Config.FrameSkip == 3;
			Frameskip4MenuItem.Checked = Config.FrameSkip == 4;
			Frameskip5MenuItem.Checked = Config.FrameSkip == 5;
			Frameskip6MenuItem.Checked = Config.FrameSkip == 6;
			Frameskip7MenuItem.Checked = Config.FrameSkip == 7;
			Frameskip8MenuItem.Checked = Config.FrameSkip == 8;
			Frameskip9MenuItem.Checked = Config.FrameSkip == 9;
			MinimizeSkippingMenuItem.Enabled = !NeverSkipMenuItem.Checked;
			if (!MinimizeSkippingMenuItem.Enabled)
			{
				MinimizeSkippingMenuItem.Checked = true;
			}

			AudioThrottleMenuItem.Enabled = Config.SoundEnabled;
			AudioThrottleMenuItem.Checked = Config.SoundThrottle;
			VsyncEnabledMenuItem.Checked = Config.VSync;

			Speed100MenuItem.Checked = Config.SpeedPercent == 100;
			Speed100MenuItem.Image = (Config.SpeedPercentAlternate == 100) ? Properties.Resources.FastForward : null;
			Speed150MenuItem.Checked = Config.SpeedPercent == 150;
			Speed150MenuItem.Image = (Config.SpeedPercentAlternate == 150) ? Properties.Resources.FastForward : null;
			Speed400MenuItem.Checked = Config.SpeedPercent == 400;
			Speed400MenuItem.Image = (Config.SpeedPercentAlternate == 400) ? Properties.Resources.FastForward : null;
			Speed200MenuItem.Checked = Config.SpeedPercent == 200;
			Speed200MenuItem.Image = (Config.SpeedPercentAlternate == 200) ? Properties.Resources.FastForward : null;
			Speed75MenuItem.Checked = Config.SpeedPercent == 75;
			Speed75MenuItem.Image = (Config.SpeedPercentAlternate == 75) ? Properties.Resources.FastForward : null;
			Speed50MenuItem.Checked = Config.SpeedPercent == 50;
			Speed50MenuItem.Image = (Config.SpeedPercentAlternate == 50) ? Properties.Resources.FastForward : null;

			Speed50MenuItem.Enabled =
				Speed75MenuItem.Enabled =
				Speed100MenuItem.Enabled =
				Speed150MenuItem.Enabled =
				Speed200MenuItem.Enabled =
				Speed400MenuItem.Enabled =
				Config.ClockThrottle;

			miUnthrottled.Checked = Config.Unthrottled;
		}

		private void KeyPriorityMenuItem_DropDownOpened(object sender, EventArgs e)
		{
			BothHkAndControllerMenuItem.Checked = false;
			InputOverHkMenuItem.Checked = false;
			HkOverInputMenuItem.Checked = false;

			switch (Config.InputHotkeyOverrideOptions)
			{
				default:
				case 0:
					BothHkAndControllerMenuItem.Checked = true;
					break;
				case 1:
					InputOverHkMenuItem.Checked = true;
					break;
				case 2:
					HkOverInputMenuItem.Checked = true;
					break;
			}
		}

		private void ControllersMenuItem_Click(object sender, EventArgs e)
		{
			using var controller = new ControllerConfig(this, Emulator, Config);
			if (!controller.ShowDialog().IsOk()) return;
			AddOnScreenMessage("Controller settings saved");

			InitControls();
			InputManager.SyncControls(Emulator, MovieSession, Config);
		}

		private void HotkeysMenuItem_Click(object sender, EventArgs e)
		{
			using var hotkeyConfig = new HotkeyConfig(Config);
			if (!hotkeyConfig.ShowDialog().IsOk()) return;
			AddOnScreenMessage("Hotkey settings saved");

			InitControls();
			InputManager.SyncControls(Emulator, MovieSession, Config);
		}

		private void FirmwaresMenuItem_Click(object sender, EventArgs e)
		{
			if (e is RomLoader.RomErrorArgs args)
			{
				using var configForm = new FirmwaresConfig(
					this,
					FirmwareManager,
					Config.FirmwareUserSpecifications,
					Config.PathEntries,
					retryLoadRom: true,
					reloadRomPath: args.RomPath);
				var result = configForm.ShowDialog();
				args.Retry = result == DialogResult.Retry;
			}
			else
			{
				using var configForm = new FirmwaresConfig(this, FirmwareManager, Config.FirmwareUserSpecifications, Config.PathEntries);
				configForm.ShowDialog();
			}
		}

		private void MessagesMenuItem_Click(object sender, EventArgs e)
		{
			using var form = new MessageConfig(Config);
			if (form.ShowDialog().IsOk()) AddOnScreenMessage("Message settings saved");
		}

		private void PathsMenuItem_Click(object sender, EventArgs e)
		{
			using var form = new PathConfig(Config.PathEntries, Game.System, newPath => MovieSession.BackupDirectory = newPath);
			if (form.ShowDialog().IsOk()) AddOnScreenMessage("Path settings saved");
		}

		private void SoundMenuItem_Click(object sender, EventArgs e)
		{
			static IEnumerable<string> GetDeviceNamesCallback(ESoundOutputMethod outputMethod) => outputMethod switch
			{
				ESoundOutputMethod.DirectSound => IndirectX.GetDSSinkNames(),
				ESoundOutputMethod.XAudio2 => IndirectX.GetXAudio2SinkNames(),
				ESoundOutputMethod.OpenAL => OpenALSoundOutput.GetDeviceNames(),
				_ => Enumerable.Empty<string>()
			};
			using var form = new SoundConfig(this, Config, GetDeviceNamesCallback);
			if (!form.ShowDialog().IsOk()) return;
			AddOnScreenMessage("Sound settings saved");

			if (form.ApplyNewSoundDevice)
			{
				Sound.Dispose();
				Sound = new Sound(Handle, Config, () => Emulator.VsyncRate());
				Sound.StartSound();
			}
			else
			{
				Sound.StopSound();
				Sound.StartSound();
			}
			RewireSound();
		}

		private void AutofireMenuItem_Click(object sender, EventArgs e)
		{
			using var form = new AutofireConfig(Config, InputManager.AutoFireController, InputManager.AutofireStickyXorAdapter);
			if (form.ShowDialog().IsOk()) AddOnScreenMessage("Autofire settings saved");
		}

		private void RewindOptionsMenuItem_Click(object sender, EventArgs e)
		{
			if (Emulator.HasSavestates())
			{
				using var form = new RewindConfig(Config, CreateRewinder, () => this.Rewinder, Emulator.AsStatable());
				if (form.ShowDialog().IsOk()) AddOnScreenMessage("Rewind and State settings saved");
			}
		}

		private void FileExtensionsMenuItem_Click(object sender, EventArgs e)
		{
			using var form = new FileExtensionPreferences(Config.PreferredPlatformsForExtensions);
			if (form.ShowDialog().IsOk()) AddOnScreenMessage("Rom Extension Preferences changed");
		}

		private void BumpAutoFlushSaveRamTimer()
		{
			if (AutoFlushSaveRamIn > Config.FlushSaveRamFrames)
			{
				AutoFlushSaveRamIn = Config.FlushSaveRamFrames;
			}
		}

		private void CustomizeMenuItem_Click(object sender, EventArgs e)
		{
			var prevLuaEngine = Config.LuaEngine;
			using var form = new EmuHawkOptions(Config, BumpAutoFlushSaveRamTimer);
			if (!form.ShowDialog().IsOk()) return;
			AddOnScreenMessage("Custom configurations saved.");
			if (Config.LuaEngine != prevLuaEngine) AddOnScreenMessage("Restart EmuHawk for Lua change to take effect");
		}

		private void ClockThrottleMenuItem_Click(object sender, EventArgs e)
		{
			Config.ClockThrottle ^= true;
			if (Config.ClockThrottle)
			{
				var old = Config.SoundThrottle;
				Config.SoundThrottle = false;
				if (old)
				{
					RewireSound();
				}

				old = Config.VSyncThrottle;
				Config.VSyncThrottle = false;
				if (old)
				{
					_presentationPanel.Resized = true;
				}
			}

			ThrottleMessage();
		}

		private void AudioThrottleMenuItem_Click(object sender, EventArgs e)
		{
			Config.SoundThrottle ^= true;
			RewireSound();
			if (Config.SoundThrottle)
			{
				Config.ClockThrottle = false;
				var old = Config.VSyncThrottle;
				Config.VSyncThrottle = false;
				if (old)
				{
					_presentationPanel.Resized = true;
				}
			}

			ThrottleMessage();
		}

		private void VsyncThrottleMenuItem_Click(object sender, EventArgs e)
		{
			Config.VSyncThrottle ^= true;
			_presentationPanel.Resized = true;
			if (Config.VSyncThrottle)
			{
				Config.ClockThrottle = false;
				var old = Config.SoundThrottle;
				Config.SoundThrottle = false;
				if (old)
				{
					RewireSound();
				}
			}

			if (!Config.VSync)
			{
				Config.VSync = true;
				VsyncMessage();
			}

			ThrottleMessage();
		}

		private void VsyncEnabledMenuItem_Click(object sender, EventArgs e)
		{
			Config.VSync ^= true;
			if (!Config.VSyncThrottle) // when vsync throttle is on, vsync is forced to on, so no change to make here
			{
				_presentationPanel.Resized = true;
			}

			VsyncMessage();
		}

		private void UnthrottledMenuItem_Click(object sender, EventArgs e)
		{
			Config.Unthrottled ^= true;
			ThrottleMessage();
		}

		private void MinimizeSkippingMenuItem_Click(object sender, EventArgs e)
		{
			Config.AutoMinimizeSkipping ^= true;
		}

		private void NeverSkipMenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 0; FrameSkipMessage(); }
		private void Frameskip1MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 1; FrameSkipMessage(); }
		private void Frameskip2MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 2; FrameSkipMessage(); }
		private void Frameskip3MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 3; FrameSkipMessage(); }
		private void Frameskip4MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 4; FrameSkipMessage(); }
		private void Frameskip5MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 5; FrameSkipMessage(); }
		private void Frameskip6MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 6; FrameSkipMessage(); }
		private void Frameskip7MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 7; FrameSkipMessage(); }
		private void Frameskip8MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 8; FrameSkipMessage(); }
		private void Frameskip9MenuItem_Click(object sender, EventArgs e) { Config.FrameSkip = 9; FrameSkipMessage(); }

		private void Speed50MenuItem_Click(object sender, EventArgs e) => ClickSpeedItem(50);
		private void Speed75MenuItem_Click(object sender, EventArgs e) => ClickSpeedItem(75);
		private void Speed100MenuItem_Click(object sender, EventArgs e) => ClickSpeedItem(100);
		private void Speed150MenuItem_Click(object sender, EventArgs e) => ClickSpeedItem(150);
		private void Speed200MenuItem_Click(object sender, EventArgs e) => ClickSpeedItem(200);
		private void Speed400MenuItem_Click(object sender, EventArgs e) => ClickSpeedItem(400);

		private void BothHkAndControllerMenuItem_Click(object sender, EventArgs e)
		{
			Config.InputHotkeyOverrideOptions = 0;
			UpdateKeyPriorityIcon();
		}

		private void InputOverHkMenuItem_Click(object sender, EventArgs e)
		{
			Config.InputHotkeyOverrideOptions = 1;
			UpdateKeyPriorityIcon();
		}

		private void HkOverInputMenuItem_Click(object sender, EventArgs e)
		{
			Config.InputHotkeyOverrideOptions = 2;
			UpdateKeyPriorityIcon();
		}

		private void SaveConfigMenuItem_Click(object sender, EventArgs e)
		{
			SaveConfig();
			AddOnScreenMessage("Saved settings");
		}

		private void SaveConfigAsMenuItem_Click(object sender, EventArgs e)
		{
			var path = _getConfigPath();
			using var sfd = new SaveFileDialog
			{
				InitialDirectory = Path.GetDirectoryName(path),
				FileName = Path.GetFileName(path),
				Filter = ConfigFileFSFilterString
			};

			if (this.ShowDialogWithTempMute(sfd).IsOk())
			{
				SaveConfig(sfd.FileName);
				AddOnScreenMessage("Copied settings");
			}
		}

		private void LoadConfigMenuItem_Click(object sender, EventArgs e)
		{
			LoadConfigFile(_getConfigPath());
		}

		private void LoadConfigFromMenuItem_Click(object sender, EventArgs e)
		{
			var path = _getConfigPath();
			using var ofd = new OpenFileDialog
			{
				InitialDirectory = Path.GetDirectoryName(path),
				FileName = Path.GetFileName(path),
				Filter = ConfigFileFSFilterString
			};

			if (this.ShowDialogWithTempMute(ofd).IsOk())
			{
				LoadConfigFile(ofd.FileName);
			}
		}

		private void ToolsSubMenu_DropDownOpened(object sender, EventArgs e)
		{
			RamWatchMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["RAM Watch"].Bindings;
			RamSearchMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["RAM Search"].Bindings;
			HexEditorMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Hex Editor"].Bindings;
			LuaConsoleMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Lua Console"].Bindings;
			TAStudioMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["TAStudio"].Bindings;
			VirtualPadMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Virtual Pad"].Bindings;
			TraceLoggerMenuItem.ShortcutKeyDisplayString = Config.HotkeyBindings["Trace Logger"].Bindings;
			TraceLoggerMenuItem.Enabled = Tools.IsAvailable<TraceLogger>();

			TAStudioMenuItem.Enabled = Tools.IsAvailable<TAStudio>();

			HexEditorMenuItem.Enabled = Tools.IsAvailable<HexEditor>();
			RamSearchMenuItem.Enabled = Tools.IsAvailable<RamSearch>();
			RamWatchMenuItem.Enabled = Tools.IsAvailable<RamWatch>();

			BasicBotMenuItem.Enabled = Tools.IsAvailable<BasicBot>();

			MacroToolMenuItem.Enabled = MovieSession.Movie.IsActive() && Tools.IsAvailable<MacroInputTool>();
			VirtualPadMenuItem.Enabled = Emulator.ControllerDefinition.Any();
		}

		private void ExternalToolMenuItem_DropDownOpening(object sender, EventArgs e)
		{
			ExternalToolMenuItem.DropDownItems.Clear();

			foreach (var item in ExtToolManager.ToolStripMenu)
			{
				if (item.Tag is ValueTuple<string, string> tuple)
				{
					if (item.Enabled)
					{
						item.Click += (clickEventSender, clickEventArgs) => Tools.LoadExternalToolForm(tuple.Item1, tuple.Item2);
					}
				}
				else
				{
					item.Image = Properties.Resources.ExclamationRed;
				}

				ExternalToolMenuItem.DropDownItems.Add(item);
			}

			if (ExternalToolMenuItem.DropDownItems.Count == 0)
			{
				ExternalToolMenuItem.DropDownItems.Add("None");
			}
		}

		private void AboutSubMenu_Click(object sender, EventArgs e)
		{
			ShowMessageBox(owner: null, "Current GBAHawk version: " + VersionInfo.MainVersion);
		}

		private void RamWatchMenuItem_Click(object sender, EventArgs e)
		{
			Tools.LoadRamWatch(true);
		}

		private void RamSearchMenuItem_Click(object sender, EventArgs e) => Tools.Load<RamSearch>();

		private void LuaConsoleMenuItem_Click(object sender, EventArgs e)
		{
			OpenLuaConsole();
		}

		private void TAStudioMenuItem_Click(object sender, EventArgs e)
		{
			if (!Emulator.CanPollInput())
			{
				ShowMessageBox(owner: null, "Current core does not support input polling. TAStudio can't be used.");
				return;
			}
			const int DONT_PROMPT_BEFORE_FRAME = 2 * 60 * 60; // 2 min @ 60 fps
			if (!MovieSession.Movie.IsActive() && Emulator.Frame > DONT_PROMPT_BEFORE_FRAME // if playing casually (not recording) AND played for enough frames (prompting always would be annoying)...
				&& !this.ModalMessageBox2("This will reload the rom without saving. Launch TAStudio anyway?", "Confirmation")) // ...AND user responds "No" to "Open TAStudio?", then cancel
			{
				return;
			}
			Tools.Load<TAStudio>();
		}

		private void HexEditorMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<HexEditor>();
		}

		private void TraceLoggerMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<TraceLogger>();
		}

		private void MacroToolMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<MacroInputTool>();
		}

		private void VirtualPadMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<VirtualpadTool>();
		}

		private void BasicBotMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<BasicBot>();
		}

		private void MultidiskBundlerMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<MultiDiskBundler>();
		}

		private void GbCoreSettingsMenuItem_Click(object sender, EventArgs e)
		{
			OpenGenericCoreConfig("Gameboy Settings");
		}

		private void GbGpuViewerMenuItem_Click(object sender, EventArgs e)
		{
			Tools.Load<GbGpuView>();
		}

		private void GblSettingsMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void OpenGenericCoreConfig(string title)
			=> GenericCoreConfig.DoDialog(this, title, isMovieActive: MovieSession.Movie.IsActive());

		private void GenericCoreSettingsMenuItem_Click(object sender, EventArgs e)
		{
			var coreName = ((CoreAttribute) Attribute.GetCustomAttribute(Emulator.GetType(), typeof(CoreAttribute))).CoreName;
			OpenGenericCoreConfig($"{coreName} Settings");
		}

		private void MainFormContextMenu_Opening(object sender, System.ComponentModel.CancelEventArgs e)
		{
			MaybePauseFromMenuOpened();

			OpenRomContextMenuItem.Visible = Emulator.IsNull();

			bool showMenuVisible = !MainMenuStrip.Visible; // need to always be able to restore this as an emergency measure

			if (_argParser._chromeless)
			{
				showMenuVisible = true; // I decided this was always possible in chrome-less mode, we'll see what they think
			}

			var movieIsActive = MovieSession.Movie.IsActive();

			ShowMenuContextMenuItem.Visible =
				ShowMenuContextMenuSeparator.Visible =
				showMenuVisible;

			LoadLastRomContextMenuItem.Visible = Emulator.IsNull();

			StopAVContextMenuItem.Visible = _currAviWriter != null;

			ContextSeparator_AfterMovie.Visible =
				ContextSeparator_AfterUndo.Visible =
				ScreenshotContextMenuItem.Visible =
				CloseRomContextMenuItem.Visible =
				UndoSavestateContextMenuItem.Visible =
				!Emulator.IsNull();

			RecordMovieContextMenuItem.Visible =
				PlayMovieContextMenuItem.Visible =
				LoadLastMovieContextMenuItem.Visible =
				!Emulator.IsNull() && !movieIsActive;

			RestartMovieContextMenuItem.Visible =
				StopMovieContextMenuItem.Visible =
				ViewSubtitlesContextMenuItem.Visible =
				ViewCommentsContextMenuItem.Visible =
				SaveMovieContextMenuItem.Visible =
				SaveMovieAsContextMenuItem.Visible =
					movieIsActive;

			BackupMovieContextMenuItem.Visible = movieIsActive;

			StopNoSaveContextMenuItem.Visible = movieIsActive && MovieSession.Movie.Changes;

			AddSubtitleContextMenuItem.Visible = !Emulator.IsNull() && movieIsActive && !MovieSession.ReadOnly;

			ConfigContextMenuItem.Visible = false;

			ClearSRAMContextMenuItem.Visible = File.Exists(Config.PathEntries.SaveRamAbsolutePath(Game, MovieSession.Movie));

			ContextSeparator_AfterROM.Visible = OpenRomContextMenuItem.Visible || LoadLastRomContextMenuItem.Visible;

			LoadLastRomContextMenuItem.Enabled = !Config.RecentRoms.Empty;
			LoadLastMovieContextMenuItem.Enabled = !Config.RecentMovies.Empty;

			if (movieIsActive)
			{
				if (MovieSession.ReadOnly)
				{
					ViewSubtitlesContextMenuItem.Text = "View Subtitles";
					ViewCommentsContextMenuItem.Text = "View Comments";
				}
				else
				{
					ViewSubtitlesContextMenuItem.Text = "Edit Subtitles";
					ViewCommentsContextMenuItem.Text = "Edit Comments";
				}
			}

			var file = new FileInfo($"{SaveStatePrefix()}.QuickSave{Config.SaveSlot}.State.bak");

			if (file.Exists)
			{
				UndoSavestateContextMenuItem.Enabled = true;
				if (_stateSlots.IsRedo(MovieSession.Movie, Config.SaveSlot))
				{
					UndoSavestateContextMenuItem.Text = $"Redo Save to slot {Config.SaveSlot}";
					UndoSavestateContextMenuItem.Image = Properties.Resources.Redo;
				}
				else
				{
					UndoSavestateContextMenuItem.Text = $"Undo Save to slot {Config.SaveSlot}";
					UndoSavestateContextMenuItem.Image = Properties.Resources.Undo;
				}
			}
			else
			{
				UndoSavestateContextMenuItem.Enabled = false;
				UndoSavestateContextMenuItem.Text = "Undo Savestate";
				UndoSavestateContextMenuItem.Image = Properties.Resources.Undo;
			}

			ShowMenuContextMenuItem.Text = MainMenuStrip.Visible ? "Hide Menu" : "Show Menu";
		}

		private void MainFormContextMenu_Closing(object sender, ToolStripDropDownClosingEventArgs e)
			=> MaybeUnpauseFromMenuClosed();

		private void DisplayConfigMenuItem_Click(object sender, EventArgs e)
		{
			using var window = new DisplayConfig(Config, GL);
			if (window.ShowDialog().IsOk())
			{
				DisplayManager.RefreshUserShader();
				FrameBufferResized();
				SynchChrome();
				if (window.NeedReset)
				{
					AddOnScreenMessage("Restart program for changed settings");
				}
			}
		}

		private void LoadLastRomContextMenuItem_Click(object sender, EventArgs e)
		{
			LoadRomFromRecent(Config.RecentRoms.MostRecent);
		}

		private void LoadLastMovieContextMenuItem_Click(object sender, EventArgs e)
		{
			LoadMoviesFromRecent(Config.RecentMovies.MostRecent);
		}

		private void BackupMovieContextMenuItem_Click(object sender, EventArgs e)
		{
			MovieSession.Movie.SaveBackup();
			AddOnScreenMessage("Backup movie saved.");
		}

		private void ViewSubtitlesContextMenuItem_Click(object sender, EventArgs e)
		{
			if (MovieSession.Movie.IsActive())
			{
				using var form = new EditSubtitlesForm(this, MovieSession.Movie, MovieSession.ReadOnly);
				form.ShowDialog();
			}
		}

		private void AddSubtitleContextMenuItem_Click(object sender, EventArgs e)
		{
			// TODO: rethink this?
			var subForm = new SubtitleMaker();
			subForm.DisableFrame();

			int index = -1;
			var sub = new Subtitle();
			for (int i = 0; i < MovieSession.Movie.Subtitles.Count; i++)
			{
				sub = MovieSession.Movie.Subtitles[i];
				if (Emulator.Frame == sub.Frame)
				{
					index = i;
					break;
				}
			}

			if (index < 0)
			{
				sub = new Subtitle { Frame = Emulator.Frame };
			}

			subForm.Sub = sub;

			if (subForm.ShowDialog().IsOk())
			{
				if (index >= 0)
				{
					MovieSession.Movie.Subtitles.RemoveAt(index);
				}

				MovieSession.Movie.Subtitles.Add(subForm.Sub);
			}
		}

		private void ViewCommentsContextMenuItem_Click(object sender, EventArgs e)
		{
			if (MovieSession.Movie.IsActive())
			{
				using var form = new EditCommentsForm(MovieSession.Movie, MovieSession.ReadOnly);
				form.ShowDialog();
			}
		}

		private void UndoSavestateContextMenuItem_Click(object sender, EventArgs e)
		{
			_stateSlots.SwapBackupSavestate(MovieSession.Movie, $"{SaveStatePrefix()}.QuickSave{Config.SaveSlot}.State", Config.SaveSlot);
			AddOnScreenMessage($"Save slot {Config.SaveSlot} restored.");
		}

		private void ClearSramContextMenuItem_Click(object sender, EventArgs e)
		{
			CloseRom(clearSram: true);
		}

		private void ShowMenuContextMenuItem_Click(object sender, EventArgs e)
		{
			MainMenuStrip.Visible ^= true;
			FrameBufferResized();
		}

		private void DumpStatusButton_Click(object sender, EventArgs e)
		{
			string details = Emulator.RomDetails();
			if (string.IsNullOrWhiteSpace(details))
			{
				details = _defaultRomDetails;
			}

			if (!string.IsNullOrEmpty(details))
			{
				Tools.Load<LogWindow>();
				((LogWindow) Tools.Get<LogWindow>()).ShowReport("Dump Status Report", details);
			}
		}

		private void SlotStatusButtons_MouseUp(object sender, MouseEventArgs e)
		{
			int slot = 0;
			if (sender == Slot1StatusButton) slot = 1;
			if (sender == Slot2StatusButton) slot = 2;
			if (sender == Slot3StatusButton) slot = 3;
			if (sender == Slot4StatusButton) slot = 4;
			if (sender == Slot5StatusButton) slot = 5;
			if (sender == Slot6StatusButton) slot = 6;
			if (sender == Slot7StatusButton) slot = 7;
			if (sender == Slot8StatusButton) slot = 8;
			if (sender == Slot9StatusButton) slot = 9;
			if (sender == Slot0StatusButton) slot = 0;

			if (e.Button == MouseButtons.Left)
			{
				if (HasSlot(slot))
				{
					LoadQuickSave($"QuickSave{slot}");
				}
			}
			else if (e.Button == MouseButtons.Right)
			{
				SaveQuickSave($"QuickSave{slot}");
			}
		}

		private void KeyPriorityStatusLabel_Click(object sender, EventArgs e)
		{
			Config.InputHotkeyOverrideOptions = Config.InputHotkeyOverrideOptions switch
			{
				1 => 2,
				2 => Config.NoMixedInputHokeyOverride ? 1 : 0,
				_ => 1,
			};
			UpdateKeyPriorityIcon();
		}

		private void LinkConnectStatusBarButton_Click(object sender, EventArgs e)
		{
			// toggle Link status (only outside of a movie session)
			if (!MovieSession.Movie.IsPlaying())
			{
				Emulator.AsLinkable().LinkConnected ^= true;
				Console.WriteLine("Cable connect status to {0}", Emulator.AsLinkable().LinkConnected);
			}
		}

		private void MainForm_Activated(object sender, EventArgs e)
		{
			if (!Config.RunInBackground) MaybeUnpauseFromMenuClosed();
		}

		private void MainForm_Deactivate(object sender, EventArgs e)
		{
			if (!Config.RunInBackground) MaybePauseFromMenuOpened();
		}

		private void TimerMouseIdle_Tick(object sender, EventArgs e)
		{

		}

		private void MainForm_Enter(object sender, EventArgs e)
		{
			AutohideCursor(false);
		}

		private void MainForm_Resize(object sender, EventArgs e)
		{
			_presentationPanel.Resized = true;
		}

		private void MainForm_Shown(object sender, EventArgs e)
		{
			if (Config.RecentWatches.AutoLoad)
			{
				Tools.LoadRamWatch(!Config.DisplayRamWatch);
			}

			Tools.AutoLoad();
			HandlePlatformMenus();
		}

		protected override void OnClosed(EventArgs e)
		{
			_windowClosedAndSafeToExitProcess = true;
			base.OnClosed(e);
		}

		private void MainformMenu_MenuActivate(object sender, EventArgs e)
		{
			HandlePlatformMenus();
			MaybePauseFromMenuOpened();
		}

		public void MaybePauseFromMenuOpened()
		{
			if (!Config.PauseWhenMenuActivated) return;
			_wasPaused = EmulatorPaused;
			PauseEmulator();
			_didMenuPause = true; // overwrites value set during PauseEmulator call
		}

		private void MainformMenu_MenuDeactivate(object sender, EventArgs e) => MaybeUnpauseFromMenuClosed();

		public void MaybeUnpauseFromMenuClosed()
		{
			if (_wasPaused || !Config.PauseWhenMenuActivated) return;
			UnpauseEmulator();
		}

		private static void FormDragEnter(object sender, DragEventArgs e)
		{
			e.Set(DragDropEffects.Copy);
		}

		private void FormDragDrop(object sender, DragEventArgs e)
			=> PathsFromDragDrop = (string[]) e.Data.GetData(DataFormats.FileDrop);
	}
}
