namespace BizHawk.Client.GBAHawk
{
	partial class DisplayConfig
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DisplayConfig));
            this.btnCancel = new System.Windows.Forms.Button();
            this.btnOk = new System.Windows.Forms.Button();
            this.linkLabel1 = new System.Windows.Forms.LinkLabel();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.btnDefaults = new System.Windows.Forms.Button();
            this.tpDispMethod = new System.Windows.Forms.TabPage();
            this.label6 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.label13 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.cbAlternateVsync = new System.Windows.Forms.CheckBox();
            this.label8 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.rbD3D9 = new System.Windows.Forms.RadioButton();
            this.label7 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.rbGDIPlus = new System.Windows.Forms.RadioButton();
            this.label5 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.rbOpenGL = new System.Windows.Forms.RadioButton();
            this.tpAR = new System.Windows.Forms.TabPage();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.label16 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.label15 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.txtCropBottom = new System.Windows.Forms.TextBox();
            this.label17 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.txtCropRight = new System.Windows.Forms.TextBox();
            this.txtCropTop = new System.Windows.Forms.TextBox();
            this.label14 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.txtCropLeft = new System.Windows.Forms.TextBox();
            this.label11 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.lblScanlines = new BizHawk.WinForms.Controls.LocLabelEx();
            this.lblUserFilterName = new BizHawk.WinForms.Controls.LocLabelEx();
            this.btnSelectUserFilter = new System.Windows.Forms.Button();
            this.rbUser = new System.Windows.Forms.RadioButton();
            this.tbScanlineIntensity = new BizHawk.Client.GBAHawk.TransparentTrackBar();
            this.rbNone = new System.Windows.Forms.RadioButton();
            this.rbScanlines = new System.Windows.Forms.RadioButton();
            this.rbHq2x = new System.Windows.Forms.RadioButton();
            this.label10 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.checkLetterbox = new System.Windows.Forms.CheckBox();
            this.nudPrescale = new System.Windows.Forms.NumericUpDown();
            this.checkPadInteger = new System.Windows.Forms.CheckBox();
            this.grpARSelection = new System.Windows.Forms.GroupBox();
            this.txtCustomARY = new System.Windows.Forms.TextBox();
            this.label12 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.txtCustomARX = new System.Windows.Forms.TextBox();
            this.rbUseCustomRatio = new System.Windows.Forms.RadioButton();
            this.label4 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.txtCustomARHeight = new System.Windows.Forms.TextBox();
            this.label3 = new BizHawk.WinForms.Controls.LocLabelEx();
            this.txtCustomARWidth = new System.Windows.Forms.TextBox();
            this.rbUseCustom = new System.Windows.Forms.RadioButton();
            this.rbUseRaw = new System.Windows.Forms.RadioButton();
            this.rbUseSystem = new System.Windows.Forms.RadioButton();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.flpStaticWindowTitles = new BizHawk.WinForms.Controls.LocSzSingleColumnFLP();
            this.cbStaticWindowTitles = new BizHawk.WinForms.Controls.CheckBoxEx();
            this.lblStaticWindowTitles = new BizHawk.WinForms.Controls.LocLabelEx();
            this.tpDispMethod.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.tpAR.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tbScanlineIntensity)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPrescale)).BeginInit();
            this.grpARSelection.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.flpStaticWindowTitles.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(473, 390);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // btnOk
            // 
            this.btnOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOk.Location = new System.Drawing.Point(392, 390);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 4;
            this.btnOk.Text = "OK";
            this.btnOk.UseVisualStyleBackColor = true;
            this.btnOk.Click += new System.EventHandler(this.BtnOk_Click);
            // 
            // linkLabel1
            // 
            this.linkLabel1.AutoSize = true;
            this.linkLabel1.Location = new System.Drawing.Point(12, 404);
            this.linkLabel1.Name = "linkLabel1";
            this.linkLabel1.Size = new System.Drawing.Size(79, 13);
            this.linkLabel1.TabIndex = 18;
            this.linkLabel1.TabStop = true;
            this.linkLabel1.Text = "Documentation";
            this.linkLabel1.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.LinkLabel1_LinkClicked);
            // 
            // btnDefaults
            // 
            this.btnDefaults.Location = new System.Drawing.Point(447, 262);
            this.btnDefaults.Name = "btnDefaults";
            this.btnDefaults.Size = new System.Drawing.Size(75, 23);
            this.btnDefaults.TabIndex = 18;
            this.btnDefaults.Text = "Defaults";
            this.toolTip1.SetToolTip(this.btnDefaults, "Unless I forgot to update the button\'s code when I changed a default");
            this.btnDefaults.UseVisualStyleBackColor = true;
            this.btnDefaults.Click += new System.EventHandler(this.BtnDefaults_Click);
            // 
            // tpDispMethod
            // 
            this.tpDispMethod.Controls.Add(this.flpStaticWindowTitles);
            this.tpDispMethod.Controls.Add(this.label6);
            this.tpDispMethod.Controls.Add(this.groupBox3);
            this.tpDispMethod.Location = new System.Drawing.Point(4, 22);
            this.tpDispMethod.Name = "tpDispMethod";
            this.tpDispMethod.Size = new System.Drawing.Size(528, 342);
            this.tpDispMethod.TabIndex = 2;
            this.tpDispMethod.Text = "Display Method";
            this.tpDispMethod.UseVisualStyleBackColor = true;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(3, 258);
            this.label6.Name = "label6";
            this.label6.Text = "Changes require restart of program to take effect.\r\n";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.label13);
            this.groupBox3.Controls.Add(this.cbAlternateVsync);
            this.groupBox3.Controls.Add(this.label8);
            this.groupBox3.Controls.Add(this.rbD3D9);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.rbGDIPlus);
            this.groupBox3.Controls.Add(this.label5);
            this.groupBox3.Controls.Add(this.rbOpenGL);
            this.groupBox3.Location = new System.Drawing.Point(6, 5);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(415, 241);
            this.groupBox3.TabIndex = 16;
            this.groupBox3.TabStop = false;
            // 
            // label13
            // 
            this.label13.Location = new System.Drawing.Point(45, 60);
            this.label13.Name = "label13";
            this.label13.Text = resources.GetString("label13.Text");
            this.label13.Click += new System.EventHandler(this.Label13_Click);
            this.label13.DoubleClick += new System.EventHandler(this.Label13_Click);
            // 
            // cbAlternateVsync
            // 
            this.cbAlternateVsync.AutoSize = true;
            this.cbAlternateVsync.Location = new System.Drawing.Point(28, 60);
            this.cbAlternateVsync.Name = "cbAlternateVsync";
            this.cbAlternateVsync.Size = new System.Drawing.Size(15, 14);
            this.cbAlternateVsync.TabIndex = 21;
            this.cbAlternateVsync.UseVisualStyleBackColor = true;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(21, 30);
            this.label8.Name = "label8";
            this.label8.Text = " • Best compatibility\r\n • May have trouble with OpenGL-based cores (N64)\r\n";
            // 
            // rbD3D9
            // 
            this.rbD3D9.AutoSize = true;
            this.rbD3D9.Checked = true;
            this.rbD3D9.Location = new System.Drawing.Point(6, 10);
            this.rbD3D9.Name = "rbD3D9";
            this.rbD3D9.Size = new System.Drawing.Size(73, 17);
            this.rbD3D9.TabIndex = 19;
            this.rbD3D9.TabStop = true;
            this.rbD3D9.Text = "Direct3D9";
            this.rbD3D9.UseVisualStyleBackColor = true;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(21, 191);
            this.label7.Name = "label7";
            this.label7.Text = " • Slow; Mainly for compatibility purposes\r\n • Missing many features\r\n • Works be" +
    "tter over Remote Desktop, etc.\r\n";
            // 
            // rbGDIPlus
            // 
            this.rbGDIPlus.AutoSize = true;
            this.rbGDIPlus.Checked = true;
            this.rbGDIPlus.Location = new System.Drawing.Point(6, 171);
            this.rbGDIPlus.Name = "rbGDIPlus";
            this.rbGDIPlus.Size = new System.Drawing.Size(50, 17);
            this.rbGDIPlus.TabIndex = 17;
            this.rbGDIPlus.TabStop = true;
            this.rbGDIPlus.Text = "GDI+";
            this.rbGDIPlus.UseVisualStyleBackColor = true;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(21, 123);
            this.label5.Name = "label5";
            this.label5.Text = " • May malfunction on some systems.\r\n • May have increased performance for OpenGL" +
    "-based emulation cores.\r\n • May have reduced performance on some systems.\r\n";
            // 
            // rbOpenGL
            // 
            this.rbOpenGL.AutoSize = true;
            this.rbOpenGL.Checked = true;
            this.rbOpenGL.Location = new System.Drawing.Point(6, 103);
            this.rbOpenGL.Name = "rbOpenGL";
            this.rbOpenGL.Size = new System.Drawing.Size(65, 17);
            this.rbOpenGL.TabIndex = 3;
            this.rbOpenGL.TabStop = true;
            this.rbOpenGL.Text = "OpenGL";
            this.rbOpenGL.UseVisualStyleBackColor = true;
            // 
            // tpAR
            // 
            this.tpAR.Controls.Add(this.groupBox6);
            this.tpAR.Controls.Add(this.btnDefaults);
            this.tpAR.Controls.Add(this.label11);
            this.tpAR.Controls.Add(this.groupBox1);
            this.tpAR.Controls.Add(this.label10);
            this.tpAR.Controls.Add(this.checkLetterbox);
            this.tpAR.Controls.Add(this.nudPrescale);
            this.tpAR.Controls.Add(this.checkPadInteger);
            this.tpAR.Controls.Add(this.grpARSelection);
            this.tpAR.Location = new System.Drawing.Point(4, 22);
            this.tpAR.Name = "tpAR";
            this.tpAR.Padding = new System.Windows.Forms.Padding(3);
            this.tpAR.Size = new System.Drawing.Size(528, 342);
            this.tpAR.TabIndex = 0;
            this.tpAR.Text = "Scaling & Filtering";
            this.tpAR.UseVisualStyleBackColor = true;
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.label16);
            this.groupBox6.Controls.Add(this.label15);
            this.groupBox6.Controls.Add(this.txtCropBottom);
            this.groupBox6.Controls.Add(this.label17);
            this.groupBox6.Controls.Add(this.txtCropRight);
            this.groupBox6.Controls.Add(this.txtCropTop);
            this.groupBox6.Controls.Add(this.label14);
            this.groupBox6.Controls.Add(this.txtCropLeft);
            this.groupBox6.Location = new System.Drawing.Point(218, 195);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(302, 61);
            this.groupBox6.TabIndex = 9;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Cropping";
            // 
            // label16
            // 
            this.label16.Location = new System.Drawing.Point(217, 25);
            this.label16.Name = "label16";
            this.label16.Text = "Bottom:";
            // 
            // label15
            // 
            this.label15.Location = new System.Drawing.Point(74, 25);
            this.label15.Name = "label15";
            this.label15.Text = "Top:";
            // 
            // txtCropBottom
            // 
            this.txtCropBottom.Location = new System.Drawing.Point(261, 22);
            this.txtCropBottom.Name = "txtCropBottom";
            this.txtCropBottom.Size = new System.Drawing.Size(34, 20);
            this.txtCropBottom.TabIndex = 28;
            this.txtCropBottom.Text = "8000";
            // 
            // label17
            // 
            this.label17.Location = new System.Drawing.Point(144, 25);
            this.label17.Name = "label17";
            this.label17.Text = "Right:";
            // 
            // txtCropRight
            // 
            this.txtCropRight.Location = new System.Drawing.Point(180, 22);
            this.txtCropRight.Name = "txtCropRight";
            this.txtCropRight.Size = new System.Drawing.Size(34, 20);
            this.txtCropRight.TabIndex = 25;
            this.txtCropRight.Text = "8000";
            // 
            // txtCropTop
            // 
            this.txtCropTop.Location = new System.Drawing.Point(104, 22);
            this.txtCropTop.Name = "txtCropTop";
            this.txtCropTop.Size = new System.Drawing.Size(34, 20);
            this.txtCropTop.TabIndex = 24;
            this.txtCropTop.Text = "8000";
            // 
            // label14
            // 
            this.label14.Location = new System.Drawing.Point(6, 25);
            this.label14.Name = "label14";
            this.label14.Text = "Left:";
            // 
            // txtCropLeft
            // 
            this.txtCropLeft.Location = new System.Drawing.Point(34, 22);
            this.txtCropLeft.Name = "txtCropLeft";
            this.txtCropLeft.Size = new System.Drawing.Size(34, 20);
            this.txtCropLeft.TabIndex = 15;
            this.txtCropLeft.Text = "8000";
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(140, 11);
            this.label11.Name = "label11";
            this.label11.Text = "X";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.lblScanlines);
            this.groupBox1.Controls.Add(this.lblUserFilterName);
            this.groupBox1.Controls.Add(this.btnSelectUserFilter);
            this.groupBox1.Controls.Add(this.rbUser);
            this.groupBox1.Controls.Add(this.tbScanlineIntensity);
            this.groupBox1.Controls.Add(this.rbNone);
            this.groupBox1.Controls.Add(this.rbScanlines);
            this.groupBox1.Controls.Add(this.rbHq2x);
            this.groupBox1.Location = new System.Drawing.Point(6, 33);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(193, 132);
            this.groupBox1.TabIndex = 7;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Scaling Filter";
            // 
            // lblScanlines
            // 
            this.lblScanlines.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblScanlines.Location = new System.Drawing.Point(104, 39);
            this.lblScanlines.Name = "lblScanlines";
            this.lblScanlines.Text = "%";
            // 
            // lblUserFilterName
            // 
            this.lblUserFilterName.Location = new System.Drawing.Point(6, 114);
            this.lblUserFilterName.Name = "lblUserFilterName";
            this.lblUserFilterName.Text = "Will contain user filter name";
            // 
            // btnSelectUserFilter
            // 
            this.btnSelectUserFilter.Location = new System.Drawing.Point(83, 88);
            this.btnSelectUserFilter.Name = "btnSelectUserFilter";
            this.btnSelectUserFilter.Size = new System.Drawing.Size(75, 23);
            this.btnSelectUserFilter.TabIndex = 5;
            this.btnSelectUserFilter.Text = "Select";
            this.btnSelectUserFilter.UseVisualStyleBackColor = true;
            this.btnSelectUserFilter.Click += new System.EventHandler(this.BtnSelectUserFilter_Click);
            // 
            // rbUser
            // 
            this.rbUser.AutoSize = true;
            this.rbUser.Location = new System.Drawing.Point(6, 88);
            this.rbUser.Name = "rbUser";
            this.rbUser.Size = new System.Drawing.Size(47, 17);
            this.rbUser.TabIndex = 4;
            this.rbUser.TabStop = true;
            this.rbUser.Text = "User";
            this.rbUser.UseVisualStyleBackColor = true;
            // 
            // tbScanlineIntensity
            // 
            this.tbScanlineIntensity.LargeChange = 32;
            this.tbScanlineIntensity.Location = new System.Drawing.Point(83, 55);
            this.tbScanlineIntensity.Maximum = 256;
            this.tbScanlineIntensity.Name = "tbScanlineIntensity";
            this.tbScanlineIntensity.Size = new System.Drawing.Size(70, 45);
            this.tbScanlineIntensity.TabIndex = 3;
            this.tbScanlineIntensity.TickFrequency = 32;
            this.tbScanlineIntensity.TickStyle = System.Windows.Forms.TickStyle.TopLeft;
            this.tbScanlineIntensity.Scroll += new System.EventHandler(this.TbScanlineIntensity_Scroll);
            this.tbScanlineIntensity.ValueChanged += new System.EventHandler(this.TbScanlineIntensity_Scroll);
            // 
            // rbNone
            // 
            this.rbNone.AutoSize = true;
            this.rbNone.Location = new System.Drawing.Point(6, 19);
            this.rbNone.Name = "rbNone";
            this.rbNone.Size = new System.Drawing.Size(51, 17);
            this.rbNone.TabIndex = 2;
            this.rbNone.TabStop = true;
            this.rbNone.Text = "None";
            this.rbNone.UseVisualStyleBackColor = true;
            // 
            // rbScanlines
            // 
            this.rbScanlines.AutoSize = true;
            this.rbScanlines.Location = new System.Drawing.Point(6, 65);
            this.rbScanlines.Name = "rbScanlines";
            this.rbScanlines.Size = new System.Drawing.Size(71, 17);
            this.rbScanlines.TabIndex = 1;
            this.rbScanlines.TabStop = true;
            this.rbScanlines.Text = "Scanlines";
            this.rbScanlines.UseVisualStyleBackColor = true;
            // 
            // rbHq2x
            // 
            this.rbHq2x.AutoSize = true;
            this.rbHq2x.Location = new System.Drawing.Point(6, 42);
            this.rbHq2x.Name = "rbHq2x";
            this.rbHq2x.Size = new System.Drawing.Size(50, 17);
            this.rbHq2x.TabIndex = 0;
            this.rbHq2x.TabStop = true;
            this.rbHq2x.Text = "Hq2x";
            this.rbHq2x.UseVisualStyleBackColor = true;
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(7, 11);
            this.label10.Name = "label10";
            this.label10.Text = "User Prescale:";
            // 
            // checkLetterbox
            // 
            this.checkLetterbox.AutoSize = true;
            this.checkLetterbox.Location = new System.Drawing.Point(209, 12);
            this.checkLetterbox.Name = "checkLetterbox";
            this.checkLetterbox.Size = new System.Drawing.Size(173, 17);
            this.checkLetterbox.TabIndex = 8;
            this.checkLetterbox.Text = "Maintain aspect ratio (letterbox)";
            this.checkLetterbox.UseVisualStyleBackColor = true;
            this.checkLetterbox.CheckedChanged += new System.EventHandler(this.CheckLetterbox_CheckedChanged);
            // 
            // nudPrescale
            // 
            this.nudPrescale.Location = new System.Drawing.Point(93, 7);
            this.nudPrescale.Maximum = new decimal(new int[] {
            16,
            0,
            0,
            0});
            this.nudPrescale.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nudPrescale.Name = "nudPrescale";
            this.nudPrescale.Size = new System.Drawing.Size(45, 20);
            this.nudPrescale.TabIndex = 14;
            this.nudPrescale.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // checkPadInteger
            // 
            this.checkPadInteger.AutoSize = true;
            this.checkPadInteger.Location = new System.Drawing.Point(218, 171);
            this.checkPadInteger.Name = "checkPadInteger";
            this.checkPadInteger.Size = new System.Drawing.Size(250, 17);
            this.checkPadInteger.TabIndex = 9;
            this.checkPadInteger.Text = "Expand pixels by integers only (e.g. no 1.3333x)";
            this.checkPadInteger.UseVisualStyleBackColor = true;
            this.checkPadInteger.CheckedChanged += new System.EventHandler(this.CheckPadInteger_CheckedChanged);
            // 
            // grpARSelection
            // 
            this.grpARSelection.Controls.Add(this.txtCustomARY);
            this.grpARSelection.Controls.Add(this.label12);
            this.grpARSelection.Controls.Add(this.txtCustomARX);
            this.grpARSelection.Controls.Add(this.rbUseCustomRatio);
            this.grpARSelection.Controls.Add(this.label4);
            this.grpARSelection.Controls.Add(this.txtCustomARHeight);
            this.grpARSelection.Controls.Add(this.label3);
            this.grpARSelection.Controls.Add(this.txtCustomARWidth);
            this.grpARSelection.Controls.Add(this.rbUseCustom);
            this.grpARSelection.Controls.Add(this.rbUseRaw);
            this.grpARSelection.Controls.Add(this.rbUseSystem);
            this.grpARSelection.Location = new System.Drawing.Point(218, 35);
            this.grpARSelection.Name = "grpARSelection";
            this.grpARSelection.Size = new System.Drawing.Size(302, 130);
            this.grpARSelection.TabIndex = 13;
            this.grpARSelection.TabStop = false;
            this.grpARSelection.Text = "Aspect Ratio Selection";
            // 
            // txtCustomARY
            // 
            this.txtCustomARY.Location = new System.Drawing.Point(220, 102);
            this.txtCustomARY.Name = "txtCustomARY";
            this.txtCustomARY.Size = new System.Drawing.Size(72, 20);
            this.txtCustomARY.TabIndex = 19;
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(202, 107);
            this.label12.Name = "label12";
            this.label12.Text = ":";
            // 
            // txtCustomARX
            // 
            this.txtCustomARX.Location = new System.Drawing.Point(124, 102);
            this.txtCustomARX.Name = "txtCustomARX";
            this.txtCustomARX.Size = new System.Drawing.Size(72, 20);
            this.txtCustomARX.TabIndex = 18;
            // 
            // rbUseCustomRatio
            // 
            this.rbUseCustomRatio.AutoSize = true;
            this.rbUseCustomRatio.Location = new System.Drawing.Point(16, 103);
            this.rbUseCustomRatio.Name = "rbUseCustomRatio";
            this.rbUseCustomRatio.Size = new System.Drawing.Size(102, 17);
            this.rbUseCustomRatio.TabIndex = 16;
            this.rbUseCustomRatio.TabStop = true;
            this.rbUseCustomRatio.Text = "Use custom AR:";
            this.rbUseCustomRatio.UseVisualStyleBackColor = true;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(13, 41);
            this.label4.Name = "label4";
            this.label4.Text = "Allow pixel distortion (e.g. 2x1 pixels, for better AR fit):";
            // 
            // txtCustomARHeight
            // 
            this.txtCustomARHeight.Location = new System.Drawing.Point(220, 79);
            this.txtCustomARHeight.Name = "txtCustomARHeight";
            this.txtCustomARHeight.Size = new System.Drawing.Size(72, 20);
            this.txtCustomARHeight.TabIndex = 15;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(202, 84);
            this.label3.Name = "label3";
            this.label3.Text = "x";
            // 
            // txtCustomARWidth
            // 
            this.txtCustomARWidth.Location = new System.Drawing.Point(124, 79);
            this.txtCustomARWidth.Name = "txtCustomARWidth";
            this.txtCustomARWidth.Size = new System.Drawing.Size(72, 20);
            this.txtCustomARWidth.TabIndex = 14;
            // 
            // rbUseCustom
            // 
            this.rbUseCustom.AutoSize = true;
            this.rbUseCustom.Location = new System.Drawing.Point(16, 80);
            this.rbUseCustom.Name = "rbUseCustom";
            this.rbUseCustom.Size = new System.Drawing.Size(105, 17);
            this.rbUseCustom.TabIndex = 13;
            this.rbUseCustom.TabStop = true;
            this.rbUseCustom.Text = "Use custom size:";
            this.rbUseCustom.UseVisualStyleBackColor = true;
            // 
            // rbUseRaw
            // 
            this.rbUseRaw.AutoSize = true;
            this.rbUseRaw.Location = new System.Drawing.Point(6, 19);
            this.rbUseRaw.Name = "rbUseRaw";
            this.rbUseRaw.Size = new System.Drawing.Size(240, 17);
            this.rbUseRaw.TabIndex = 11;
            this.rbUseRaw.TabStop = true;
            this.rbUseRaw.Text = "Use 1:1 pixel size (for crispness or debugging)";
            this.rbUseRaw.UseVisualStyleBackColor = true;
            this.rbUseRaw.CheckedChanged += new System.EventHandler(this.RbUseRaw_CheckedChanged);
            // 
            // rbUseSystem
            // 
            this.rbUseSystem.AutoSize = true;
            this.rbUseSystem.Location = new System.Drawing.Point(16, 58);
            this.rbUseSystem.Name = "rbUseSystem";
            this.rbUseSystem.Size = new System.Drawing.Size(167, 17);
            this.rbUseSystem.TabIndex = 12;
            this.rbUseSystem.TabStop = true;
            this.rbUseSystem.Text = "Use system\'s recommendation";
            this.rbUseSystem.UseVisualStyleBackColor = true;
            this.rbUseSystem.CheckedChanged += new System.EventHandler(this.RbUseSystem_CheckedChanged);
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tpAR);
            this.tabControl1.Controls.Add(this.tpDispMethod);
            this.tabControl1.Location = new System.Drawing.Point(12, 12);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(536, 368);
            this.tabControl1.TabIndex = 17;
            // 
            // flpStaticWindowTitles
            // 
            this.flpStaticWindowTitles.Controls.Add(this.cbStaticWindowTitles);
            this.flpStaticWindowTitles.Controls.Add(this.lblStaticWindowTitles);
            this.flpStaticWindowTitles.Location = new System.Drawing.Point(6, 280);
            this.flpStaticWindowTitles.Name = "flpStaticWindowTitles";
            this.flpStaticWindowTitles.Size = new System.Drawing.Size(490, 52);
            // 
            // cbStaticWindowTitles
            // 
            this.cbStaticWindowTitles.Name = "cbStaticWindowTitles";
            this.cbStaticWindowTitles.Text = "Keep window titles static";
            // 
            // lblStaticWindowTitles
            // 
            this.lblStaticWindowTitles.Location = new System.Drawing.Point(19, 23);
            this.lblStaticWindowTitles.Margin = new System.Windows.Forms.Padding(19, 0, 3, 0);
            this.lblStaticWindowTitles.Name = "lblStaticWindowTitles";
            this.lblStaticWindowTitles.Text = "Some tools put filenames, status, etc. in their window titles.\nChecking this disa" +
    "bles those features, but may fix problems with window capture (i.e. in OBS).";
            // 
            // DisplayConfig
            // 
            this.AcceptButton = this.btnOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(564, 425);
            this.Controls.Add(this.linkLabel1);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOk);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "DisplayConfig";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Display Configuration";
            this.tpDispMethod.ResumeLayout(false);
            this.tpDispMethod.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.tpAR.ResumeLayout(false);
            this.tpAR.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tbScanlineIntensity)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.nudPrescale)).EndInit();
            this.grpARSelection.ResumeLayout(false);
            this.grpARSelection.PerformLayout();
            this.tabControl1.ResumeLayout(false);
            this.flpStaticWindowTitles.ResumeLayout(false);
            this.flpStaticWindowTitles.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.Button btnOk;
		private System.Windows.Forms.LinkLabel linkLabel1;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.TabPage tpDispMethod;
		private WinForms.Controls.LocLabelEx label6;
		private System.Windows.Forms.GroupBox groupBox3;
		private WinForms.Controls.LocLabelEx label13;
		private System.Windows.Forms.CheckBox cbAlternateVsync;
		private WinForms.Controls.LocLabelEx label8;
		private System.Windows.Forms.RadioButton rbD3D9;
		private WinForms.Controls.LocLabelEx label7;
		private System.Windows.Forms.RadioButton rbGDIPlus;
		private WinForms.Controls.LocLabelEx label5;
		private System.Windows.Forms.RadioButton rbOpenGL;
		private System.Windows.Forms.TabPage tpAR;
		private System.Windows.Forms.GroupBox groupBox6;
		private WinForms.Controls.LocLabelEx label16;
		private WinForms.Controls.LocLabelEx label15;
		private System.Windows.Forms.TextBox txtCropBottom;
		private WinForms.Controls.LocLabelEx label17;
		private System.Windows.Forms.TextBox txtCropRight;
		private System.Windows.Forms.TextBox txtCropTop;
		private WinForms.Controls.LocLabelEx label14;
		private System.Windows.Forms.TextBox txtCropLeft;
		private System.Windows.Forms.Button btnDefaults;
		private WinForms.Controls.LocLabelEx label11;
		private System.Windows.Forms.GroupBox groupBox1;
		private WinForms.Controls.LocLabelEx lblScanlines;
		private WinForms.Controls.LocLabelEx lblUserFilterName;
		private System.Windows.Forms.Button btnSelectUserFilter;
		private System.Windows.Forms.RadioButton rbUser;
		private TransparentTrackBar tbScanlineIntensity;
		private System.Windows.Forms.RadioButton rbNone;
		private System.Windows.Forms.RadioButton rbScanlines;
		private System.Windows.Forms.RadioButton rbHq2x;
		private WinForms.Controls.LocLabelEx label10;
		private System.Windows.Forms.CheckBox checkLetterbox;
		private System.Windows.Forms.NumericUpDown nudPrescale;
		private System.Windows.Forms.CheckBox checkPadInteger;
		private System.Windows.Forms.GroupBox grpARSelection;
		private System.Windows.Forms.TextBox txtCustomARY;
		private WinForms.Controls.LocLabelEx label12;
		private System.Windows.Forms.TextBox txtCustomARX;
		private System.Windows.Forms.RadioButton rbUseCustomRatio;
		private WinForms.Controls.LocLabelEx label4;
		private System.Windows.Forms.TextBox txtCustomARHeight;
		private WinForms.Controls.LocLabelEx label3;
		private System.Windows.Forms.TextBox txtCustomARWidth;
		private System.Windows.Forms.RadioButton rbUseCustom;
		private System.Windows.Forms.RadioButton rbUseRaw;
		private System.Windows.Forms.RadioButton rbUseSystem;
		private System.Windows.Forms.TabControl tabControl1;
		private WinForms.Controls.LocSzSingleColumnFLP flpStaticWindowTitles;
		private WinForms.Controls.CheckBoxEx cbStaticWindowTitles;
		private WinForms.Controls.LocLabelEx lblStaticWindowTitles;
	}
}