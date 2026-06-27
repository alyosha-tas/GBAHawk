namespace BizHawk.Client.GBAHawk
{
	partial class AutofireConfig
	{
		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.btnDialogOK = new System.Windows.Forms.Button();
			this.btnDialogCancel = new System.Windows.Forms.Button();
			this.nudPatternOn = new System.Windows.Forms.NumericUpDown();
			this.nudPatternOff = new System.Windows.Forms.NumericUpDown();
			this.lblPatternOn = new System.Windows.Forms.Label();
			this.lblPatternOff = new System.Windows.Forms.Label();
			this.flpDialogButtons = new System.Windows.Forms.FlowLayoutPanel();
			this.flpDialog = new System.Windows.Forms.FlowLayoutPanel();
			this.flpPattern = new System.Windows.Forms.FlowLayoutPanel();
			this.lblPatternDesc = new System.Windows.Forms.Label();
			this.cbConsiderLag = new System.Windows.Forms.CheckBox();
			((System.ComponentModel.ISupportInitialize)(this.nudPatternOn)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nudPatternOff)).BeginInit();
			this.flpDialogButtons.SuspendLayout();
			this.flpDialog.SuspendLayout();
			this.flpPattern.SuspendLayout();
			this.SuspendLayout();
			// 
			// btnDialogOK
			// 
			this.btnDialogOK.Name = "btnDialogOK";
			this.btnDialogOK.Size = new System.Drawing.Size(75, 23);
			this.btnDialogOK.Text = "&OK";
			this.btnDialogOK.Click += new System.EventHandler(this.btnDialogOK_Click);
			// 
			// btnDialogCancel
			// 
			this.btnDialogCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnDialogCancel.Name = "btnDialogCancel";
			this.btnDialogCancel.Size = new System.Drawing.Size(75, 23);
			this.btnDialogCancel.Text = "&Cancel";
			this.btnDialogCancel.Click += new System.EventHandler(this.btnDialogCancel_Click);
			// 
			// nudPatternOn
			// 
			this.nudPatternOn.Maximum = new decimal(new int[] {
            512,
            0,
            0,
            0});
			this.nudPatternOn.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudPatternOn.Name = "nudPatternOn";
			this.nudPatternOn.Size = new System.Drawing.Size(48, 20);
			this.nudPatternOn.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// nudPatternOff
			// 
			this.nudPatternOff.Maximum = new decimal(new int[] {
            512,
            0,
            0,
            0});
			this.nudPatternOff.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.nudPatternOff.Name = "nudPatternOff";
			this.nudPatternOff.Size = new System.Drawing.Size(48, 20);
			this.nudPatternOff.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			// 
			// lblPatternOn
			// 
			this.lblPatternOn.Name = "lblPatternOn";
			this.lblPatternOn.Text = "on,";
			this.lblPatternOn.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.lblPatternOn.AutoSize = true;
			// 
			// lblPatternOff
			// 
			this.lblPatternOff.Name = "lblPatternOff";
			this.lblPatternOff.Text = "off";
			this.lblPatternOff.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.lblPatternOff.AutoSize = true;
			// 
			// flpDialogButtons
			// 
			this.flpDialogButtons.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.flpDialogButtons.Controls.Add(this.btnDialogOK);
			this.flpDialogButtons.Controls.Add(this.btnDialogCancel);
			this.flpDialogButtons.Location = new System.Drawing.Point(161, 61);
			this.flpDialogButtons.Name = "flpDialogButtons";
			this.flpDialogButtons.Size = new System.Drawing.Size(162, 29);
			this.flpDialogButtons.WrapContents = false;
			this.flpDialogButtons.Margin = System.Windows.Forms.Padding.Empty;
			// 
			// flpDialog
			// 
			this.flpDialog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.flpDialog.Controls.Add(this.flpPattern);
			this.flpDialog.Controls.Add(this.cbConsiderLag);
			this.flpDialog.Location = new System.Drawing.Point(0, 0);
			this.flpDialog.Name = "flpDialog";
			this.flpDialog.Size = new System.Drawing.Size(323, 55);
			this.flpDialog.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
			this.flpDialog.WrapContents = false;
			this.flpDialog.Margin = System.Windows.Forms.Padding.Empty;
			this.flpDialog.MinimumSize = new System.Drawing.Size(24, 24);
			// 
			// flpPattern
			// 
			this.flpPattern.Controls.Add(this.lblPatternDesc);
			this.flpPattern.Controls.Add(this.nudPatternOn);
			this.flpPattern.Controls.Add(this.lblPatternOn);
			this.flpPattern.Controls.Add(this.nudPatternOff);
			this.flpPattern.Controls.Add(this.lblPatternOff);
			this.flpPattern.Name = "flpPattern";
			this.flpPattern.AutoSize = true;
			this.flpPattern.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
			this.flpPattern.MinimumSize = new System.Drawing.Size(24, 24);
			this.flpPattern.WrapContents = false;
			this.flpPattern.Margin = System.Windows.Forms.Padding.Empty;
			// 
			// lblPatternDesc
			// 
			this.lblPatternDesc.Name = "lblPatternDesc";
			this.lblPatternDesc.Text = "Pattern:";
			this.lblPatternDesc.Anchor = System.Windows.Forms.AnchorStyles.None;
			this.lblPatternDesc.AutoSize = true;
			// 
			// cbConsiderLag
			// 
			this.cbConsiderLag.Name = "cbConsiderLag";
			this.cbConsiderLag.Padding = new System.Windows.Forms.Padding(4, 0, 0, 0);
			this.cbConsiderLag.Text = "Take lag frames into account";
			this.cbConsiderLag.AutoSize = true;
			// 
			// AutofireConfig
			// 
			this.AcceptButton = this.btnDialogOK;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.btnDialogCancel;
			this.ClientSize = new System.Drawing.Size(323, 90);
			this.Controls.Add(this.flpDialog);
			this.Controls.Add(this.flpDialogButtons);
			this.MaximizeBox = false;
			this.MinimumSize = new System.Drawing.Size(339, 129);
			this.Name = "AutofireConfig";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Autofire Configuration";
			this.Load += new System.EventHandler(this.AutofireConfig_Load);
			((System.ComponentModel.ISupportInitialize)(this.nudPatternOn)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nudPatternOff)).EndInit();
			this.flpDialogButtons.ResumeLayout(false);
			this.flpDialog.ResumeLayout(false);
			this.flpDialog.PerformLayout();
			this.flpPattern.ResumeLayout(false);
			this.flpPattern.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button btnDialogOK;
		private System.Windows.Forms.Button btnDialogCancel;
		private System.Windows.Forms.NumericUpDown nudPatternOff;
		private System.Windows.Forms.Label lblPatternOn;
		private System.Windows.Forms.Label lblPatternOff;
		private System.Windows.Forms.FlowLayoutPanel flpPattern;
		private System.Windows.Forms.FlowLayoutPanel flpDialog;
		private System.Windows.Forms.FlowLayoutPanel flpDialogButtons;
		private System.Windows.Forms.Label lblPatternDesc;
		public System.Windows.Forms.NumericUpDown nudPatternOn;
		private System.Windows.Forms.CheckBox cbConsiderLag;
	}
}
