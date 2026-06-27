namespace BizHawk.Client.GBAHawk
{
    partial class HexColorsForm
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.HexHighlight = new System.Windows.Forms.Panel();
            this.HexForegrnd = new System.Windows.Forms.Panel();
            this.label1 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.HexMenubar = new System.Windows.Forms.Panel();
            this.label2 = new System.Windows.Forms.Label();
            this.HexBackgrnd = new System.Windows.Forms.Panel();
            this.colorDialog1 = new System.Windows.Forms.ColorDialog();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.HexHighlight);
            this.groupBox1.Controls.Add(this.HexForegrnd);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.HexMenubar);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.HexBackgrnd);
            this.groupBox1.Location = new System.Drawing.Point(3, 2);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(144, 192);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(0, 0);
            this.label6.Name = "label6";
			this.label6.AutoSize = true;
			// 
			// label5
			// 
			this.label5.Location = new System.Drawing.Point(0, 0);
            this.label5.Name = "label5";
			this.label5.AutoSize = true;
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(30, 108);
            this.label4.Name = "label4";
            this.label4.Text = "Highlight Color";
			this.label4.AutoSize = true;
			// 
			// HexHighlight
			// 
			this.HexHighlight.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.HexHighlight.Location = new System.Drawing.Point(5, 104);
            this.HexHighlight.Name = "HexHighlight";
            this.HexHighlight.Size = new System.Drawing.Size(20, 20);
            this.HexHighlight.TabIndex = 12;
            this.HexHighlight.MouseClick += new System.Windows.Forms.MouseEventHandler(this.HexHighlight_Click);
            // 
            // HexForegrnd
            // 
            this.HexForegrnd.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.HexForegrnd.Location = new System.Drawing.Point(5, 42);
            this.HexForegrnd.Name = "HexForegrnd";
            this.HexForegrnd.Size = new System.Drawing.Size(20, 20);
            this.HexForegrnd.TabIndex = 7;
            this.HexForegrnd.MouseClick += new System.Windows.Forms.MouseEventHandler(this.HexForeground_Click);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(30, 15);
            this.label1.Name = "label1";
            this.label1.Text = "Background Color";
			this.label1.AutoSize = true;
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(30, 77);
            this.label3.Name = "label3";
            this.label3.Text = "Menubar Color";
			this.label3.AutoSize = true;
			// 
			// HexMenubar
			// 
			this.HexMenubar.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.HexMenubar.Location = new System.Drawing.Point(5, 73);
            this.HexMenubar.Name = "HexMenubar";
            this.HexMenubar.Size = new System.Drawing.Size(20, 20);
            this.HexMenubar.TabIndex = 8;
            this.HexMenubar.MouseClick += new System.Windows.Forms.MouseEventHandler(this.HexMenuBar_Click);
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(30, 46);
            this.label2.Name = "label2";
            this.label2.Text = "Font Color";
			this.label2.AutoSize = true;
            // 
            // HexBackgrnd
            // 
            this.HexBackgrnd.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.HexBackgrnd.Location = new System.Drawing.Point(5, 11);
            this.HexBackgrnd.Name = "HexBackgrnd";
            this.HexBackgrnd.Size = new System.Drawing.Size(20, 20);
            this.HexBackgrnd.TabIndex = 6;
            this.HexBackgrnd.MouseClick += new System.Windows.Forms.MouseEventHandler(this.HexBackground_Click);
            // 
            // HexColorsForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(149, 197);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "HexColorsForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Colors";
            this.Load += new System.EventHandler(this.HexColors_Form_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Panel HexForegrnd;
        private System.Windows.Forms.Panel HexBackgrnd;
        private System.Windows.Forms.ColorDialog colorDialog1;
		private System.Windows.Forms.Panel HexMenubar;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Panel HexHighlight;

    }
}