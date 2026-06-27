namespace BizHawk.Client.GBAHawk
{
	partial class PathInfo
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
            this.Ok = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.label12 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // Ok
            // 
            this.Ok.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.Ok.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Ok.Location = new System.Drawing.Point(388, 152);
						this.Ok.Name = "OK";
            this.Ok.Size = new System.Drawing.Size(75, 23);
            this.Ok.TabIndex = 0;
            this.Ok.Text = "&OK";
            this.Ok.UseVisualStyleBackColor = true;
            this.Ok.Click += new System.EventHandler(this.Ok_Click);
            // 
            // label1
            // 
            this.label1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label1.Location = new System.Drawing.Point(13, 13);
            this.label1.Name = "label1";
            this.label1.Text = "%recent%";
			this.label1.AutoSize = true;
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(72, 13);
            this.label2.Name = "label2";
            this.label2.Text = "Sets the path to the Windows Recent Path";
			this.label2.AutoSize = true;
			// 
			// label3
			// 
			this.label3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label3.Location = new System.Drawing.Point(13, 33);
            this.label3.Name = "label3";
            this.label3.Text = "%exe%";
			this.label3.AutoSize = true;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(72, 33);
            this.label4.Name = "label4";
            this.label4.Text = "Sets the path of the executable (EmuHawk.exe)";
			this.label4.AutoSize = true;
            // 
            // label5
            // 
            this.label5.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label5.Location = new System.Drawing.Point(13, 68);
            this.label5.Name = "label5";
            this.label5.Text = ".\\";
			this.label5.AutoSize = true;
            // 
            // label6
            // 
            this.label6.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.label6.Location = new System.Drawing.Point(13, 88);
            this.label6.Name = "label6";
            this.label6.Text = "..\\";
			this.label6.AutoSize = true;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(72, 68);
            this.label7.Name = "label7";
            this.label7.Text = "Sets the path to the base path";
			this.label7.AutoSize = true;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(94, 106);
            this.label8.Name = "label8";
            this.label8.Text = "- Setting the global base path to one of these will set it to the path of the .ex" +
    "e";
			this.label8.AutoSize = true;
            // 
            // label9
            // 
            this.label9.Location = new System.Drawing.Point(94, 121);
            this.label9.Name = "label9";
            this.label9.Text = "- Setting a platform base will set it to the global base";
			this.label9.AutoSize = true;
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(94, 136);
            this.label10.Name = "label10";
            this.label10.Text = "- Setting a platform folder will set it to the platform base";
			this.label10.AutoSize = true;
            // 
            // label11
            // 
            this.label11.Location = new System.Drawing.Point(94, 151);
            this.label11.Name = "label11";
            this.label11.Text = "- Setting a tools folder will set it to the global base";
			this.label11.AutoSize = true;
            // 
            // label12
            // 
            this.label12.Location = new System.Drawing.Point(72, 88);
            this.label12.Name = "label12";
            this.label12.Text = "Sets the path to the folder above the base path";
			this.label12.AutoSize = true;
            // 
            // PathInfo
            // 
            this.AcceptButton = this.Ok;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.Ok;
            this.ClientSize = new System.Drawing.Size(466, 177);
            this.Controls.Add(this.label12);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.Ok);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "PathInfo";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Special Commands";
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button Ok;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.Label label10;
		private System.Windows.Forms.Label label11;
		private System.Windows.Forms.Label label12;
	}
}