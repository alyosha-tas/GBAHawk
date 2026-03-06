using System;
using System.IO;
using System.Windows.Forms;

using BizHawk.Bizware.BizwareGL;
using BizHawk.Client.Common;
using BizHawk.Client.Common.Filters;
using BizHawk.Common;

namespace BizHawk.Client.GBAHawk
{
	public partial class DisplayConfig : Form
	{
		private readonly Config _config;

		private readonly IGL _gl;

		private string _pathSelection;

		public bool NeedReset { get; set; }

		public DisplayConfig(Config config, IGL gl)
		{
			_config = config;
			_gl = gl;

			InitializeComponent();

			rbNone.Checked = _config.TargetDisplayFilter == 0;
			rbHq2x.Checked = _config.TargetDisplayFilter == 1;
			rbScanlines.Checked = _config.TargetDisplayFilter == 2;
			rbUser.Checked = _config.TargetDisplayFilter == 3;

			_pathSelection = _config.DispUserFilterPath ?? "";
			RefreshState();

			tbScanlineIntensity.Value = _config.TargetScanlineFilterIntensity;
			checkLetterbox.Checked = _config.DispFixAspectRatio;
			checkPadInteger.Checked = _config.DispFixScaleInteger;

			cbAlternateVsync.Checked = _config.DispAlternateVsync;

			cbStaticWindowTitles.Checked = _config.UseStaticWindowTitles;

			rbOpenGL.Checked = _config.DispMethod == EDispMethod.OpenGL;
			rbGDIPlus.Checked = _config.DispMethod == EDispMethod.GdiPlus;
			rbD3D9.Checked = _config.DispMethod == EDispMethod.SlimDX9;

			nudPrescale.Value = _config.DispPrescale;

			if (_config.DispManagerAR == EDispManagerAR.None)
				rbUseRaw.Checked = true;
			else if (_config.DispManagerAR == EDispManagerAR.System)
				rbUseSystem.Checked = true;
			else if (_config.DispManagerAR == EDispManagerAR.Custom)
				rbUseCustom.Checked = true;
			else if (_config.DispManagerAR == EDispManagerAR.CustomRatio)
				rbUseCustomRatio.Checked = true;

			if(_config.DispCustomUserARWidth != -1)
				txtCustomARWidth.Text = _config.DispCustomUserARWidth.ToString();
			if (_config.DispCustomUserARHeight != -1)
				txtCustomARHeight.Text = _config.DispCustomUserARHeight.ToString();
			if (_config.DispCustomUserArx != -1)
				txtCustomARX.Text = _config.DispCustomUserArx.ToString();
			if (_config.DispCustomUserAry != -1)
				txtCustomARY.Text = _config.DispCustomUserAry.ToString();

			txtCropLeft.Text = _config.DispCropLeft.ToString();
			txtCropTop.Text = _config.DispCropTop.ToString();
			txtCropRight.Text = _config.DispCropRight.ToString();
			txtCropBottom.Text = _config.DispCropBottom.ToString();

			RefreshAspectRatioOptions();

			if (!HostCapabilityDetector.HasDirectX)
			{
				rbD3D9.Enabled = false;
				rbD3D9.AutoCheck = false;
				cbAlternateVsync.Enabled = false;
				label13.Enabled = false;
				label8.Enabled = false;
			}
		}

		private void BtnOk_Click(object sender, EventArgs e)
		{
			if (rbNone.Checked)
				_config.TargetDisplayFilter = 0;
			if (rbHq2x.Checked)
				_config.TargetDisplayFilter = 1;
			if (rbScanlines.Checked)
				_config.TargetDisplayFilter = 2;
			if (rbUser.Checked)
				_config.TargetDisplayFilter = 3;

			_config.DispPrescale = (int)nudPrescale.Value;

			_config.TargetScanlineFilterIntensity = tbScanlineIntensity.Value;
			_config.DispFixAspectRatio = checkLetterbox.Checked;
			_config.DispFixScaleInteger = checkPadInteger.Checked;

			_config.DispAlternateVsync = cbAlternateVsync.Checked;

			_config.UseStaticWindowTitles = cbStaticWindowTitles.Checked;

			if (rbUseRaw.Checked)
				_config.DispManagerAR = EDispManagerAR.None;
			else if (rbUseSystem.Checked)
				_config.DispManagerAR = EDispManagerAR.System;
			else if (rbUseCustom.Checked)
				_config.DispManagerAR = EDispManagerAR.Custom;
			else if (rbUseCustomRatio.Checked)
				_config.DispManagerAR = EDispManagerAR.CustomRatio;

			if (!string.IsNullOrWhiteSpace(txtCustomARWidth.Text))
			{
				if (int.TryParse(txtCustomARWidth.Text, out int dispCustomUserARWidth))
				{
					_config.DispCustomUserARWidth = dispCustomUserARWidth;
				}
			}
			else
			{
				_config.DispCustomUserARWidth = -1;
			}

			if (!string.IsNullOrWhiteSpace(txtCustomARHeight.Text))
			{
				if (int.TryParse(txtCustomARHeight.Text, out int dispCustomUserARHeight))
				{
					_config.DispCustomUserARHeight = dispCustomUserARHeight;
				}
			}
			else
			{
				_config.DispCustomUserARHeight = -1;
			}

			if (!string.IsNullOrWhiteSpace(txtCustomARX.Text))
			{
				if (float.TryParse(txtCustomARX.Text, out float dispCustomUserArx))
				{
					_config.DispCustomUserArx = dispCustomUserArx;
				}
			}
			else
			{
				_config.DispCustomUserArx = -1;
			}

			if (!string.IsNullOrWhiteSpace(txtCustomARY.Text))
			{
				if (float.TryParse(txtCustomARY.Text, out float dispCustomUserAry))
				{
					_config.DispCustomUserAry = dispCustomUserAry;
				}
			}
			else
			{
				_config.DispCustomUserAry = -1;
			}

			var oldDisplayMethod = _config.DispMethod;
			if(rbOpenGL.Checked)
				_config.DispMethod = EDispMethod.OpenGL;
			if(rbGDIPlus.Checked)
				_config.DispMethod = EDispMethod.GdiPlus;
			if(rbD3D9.Checked)
				_config.DispMethod = EDispMethod.SlimDX9;

			if (int.TryParse(txtCropLeft.Text, out int dispCropLeft))
			{
				_config.DispCropLeft = dispCropLeft;
			}

			if (int.TryParse(txtCropTop.Text, out int dispCropTop))
			{
				_config.DispCropTop = dispCropTop;
			}

			if (int.TryParse(txtCropRight.Text, out int dispCropRight))
			{
				_config.DispCropRight = dispCropRight;
			}

			if (int.TryParse(txtCropBottom.Text, out int dispCropBottom))
			{
				_config.DispCropBottom = dispCropBottom;
			}

			if (oldDisplayMethod != _config.DispMethod)
			{
				NeedReset = true;
			}

			_config.DispUserFilterPath = _pathSelection;

			DialogResult = DialogResult.OK;
			Close();
		}

		private void RefreshState()
		{
			lblUserFilterName.Text = Path.GetFileNameWithoutExtension(_pathSelection);
		}

		private void BtnSelectUserFilter_Click(object sender, EventArgs e)
		{
			using var ofd = new OpenFileDialog
			{
				Filter = new FilesystemFilter(".CGP Files", new[] { "cgp" }).ToString(),
				FileName = _pathSelection
			};
			if (ofd.ShowDialog() == DialogResult.OK)
			{
				rbUser.Checked = true;
				var choice = Path.GetFullPath(ofd.FileName);
				
				//test the preset
				using (var stream = File.OpenRead(choice))
				{
					var cgp = new RetroShaderPreset(stream);

					// try compiling it
					bool ok = false;
					string errors = "";
					try
					{
						var filter = new RetroShaderChain(_gl, cgp, Path.GetDirectoryName(choice));
						ok = filter.Available;
						errors = filter.Errors;
					}
					catch {}
					if (!ok)
					{
						using var errorForm = new ExceptionBox(errors);
						errorForm.ShowDialog();
						return;
					}
				}

				_pathSelection = choice;
				RefreshState();
			}
		}

		private void CheckLetterbox_CheckedChanged(object sender, EventArgs e)
		{
			RefreshAspectRatioOptions();
		}

		private void CheckPadInteger_CheckedChanged(object sender, EventArgs e)
		{
			RefreshAspectRatioOptions();
		}

		private void RbUseRaw_CheckedChanged(object sender, EventArgs e)
		{
			RefreshAspectRatioOptions();
		}

		private void RbUseSystem_CheckedChanged(object sender, EventArgs e)
		{
			RefreshAspectRatioOptions();
		}

		private void RefreshAspectRatioOptions()
		{
			grpARSelection.Enabled = checkLetterbox.Checked;
			checkPadInteger.Enabled = checkLetterbox.Checked;
		}

		public void TbScanlineIntensity_Scroll(object sender, EventArgs e)
		{
			_config.TargetScanlineFilterIntensity = tbScanlineIntensity.Value;
			lblScanlines.Text = $"{_config.TargetScanlineFilterIntensity / 256.0:P2}";
		}

		private void LinkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			System.Diagnostics.Process.Start("https://tasvideos.org/Bizhawk/DisplayConfig");
		}

		private void Label13_Click(object sender, EventArgs e)
		{
			cbAlternateVsync.Checked ^= true;
		}

		private void BtnDefaults_Click(object sender, EventArgs e)
		{
			nudPrescale.Value = 1;
			rbNone.Checked = true;
			checkLetterbox.Checked = true;
			rbUseSystem.Checked = true;
		}
	}
}
