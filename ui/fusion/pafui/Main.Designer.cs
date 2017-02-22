namespace FusionUI
{
    partial class Main
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Main));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.settingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.listenportToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.alfredControlToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.alfredControlActiveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.undockChartToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.setGridRangesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createEntityToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.label4 = new System.Windows.Forms.Label();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.numericUpDown3 = new System.Windows.Forms.NumericUpDown();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.checkBox2 = new System.Windows.Forms.CheckBox();
            this.checkBox3 = new System.Windows.Forms.CheckBox();
            this.checkBox4 = new System.Windows.Forms.CheckBox();
            this.checkBox5 = new System.Windows.Forms.CheckBox();
            this.checkBox6 = new System.Windows.Forms.CheckBox();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.log_box = new System.Windows.Forms.TextBox();
            this.dblBufferedDrawControl1 = new FusionUI.DblBufferedDrawControl();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown3)).BeginInit();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.settingsToolStripMenuItem,
            this.aboutToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Padding = new System.Windows.Forms.Padding(8, 2, 0, 2);
            this.menuStrip1.Size = new System.Drawing.Size(394, 28);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(44, 24);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // settingsToolStripMenuItem
            // 
            this.settingsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.listenportToolStripMenuItem,
            this.alfredControlToolStripMenuItem,
            this.alfredControlActiveToolStripMenuItem,
            this.undockChartToolStripMenuItem,
            this.setGridRangesToolStripMenuItem});
            this.settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
            this.settingsToolStripMenuItem.Size = new System.Drawing.Size(74, 24);
            this.settingsToolStripMenuItem.Text = "Settings";
            // 
            // listenportToolStripMenuItem
            // 
            this.listenportToolStripMenuItem.Name = "listenportToolStripMenuItem";
            this.listenportToolStripMenuItem.Size = new System.Drawing.Size(216, 24);
            this.listenportToolStripMenuItem.Text = "Listenport";
            this.listenportToolStripMenuItem.Click += new System.EventHandler(this.listenportToolStripMenuItem_Click);
            // 
            // alfredControlToolStripMenuItem
            // 
            this.alfredControlToolStripMenuItem.Name = "alfredControlToolStripMenuItem";
            this.alfredControlToolStripMenuItem.Size = new System.Drawing.Size(216, 24);
            this.alfredControlToolStripMenuItem.Text = "Send Socket Settings";
            this.alfredControlToolStripMenuItem.Click += new System.EventHandler(this.alfredControlToolStripMenuItem_Click);
            // 
            // alfredControlActiveToolStripMenuItem
            // 
            this.alfredControlActiveToolStripMenuItem.CheckOnClick = true;
            this.alfredControlActiveToolStripMenuItem.Name = "alfredControlActiveToolStripMenuItem";
            this.alfredControlActiveToolStripMenuItem.Size = new System.Drawing.Size(216, 24);
            this.alfredControlActiveToolStripMenuItem.Text = "Send Socket Active";
            this.alfredControlActiveToolStripMenuItem.CheckedChanged += new System.EventHandler(this.alfredControlActiveToolStripMenuItem_CheckedChanged);
            // 
            // undockChartToolStripMenuItem
            // 
            this.undockChartToolStripMenuItem.CheckOnClick = true;
            this.undockChartToolStripMenuItem.Name = "undockChartToolStripMenuItem";
            this.undockChartToolStripMenuItem.Size = new System.Drawing.Size(216, 24);
            this.undockChartToolStripMenuItem.Text = "Undock Chart";
            this.undockChartToolStripMenuItem.CheckedChanged += new System.EventHandler(this.undockChartToolStripMenuItem_CheckedChanged);
            // 
            // setGridRangesToolStripMenuItem
            // 
            this.setGridRangesToolStripMenuItem.Name = "setGridRangesToolStripMenuItem";
            this.setGridRangesToolStripMenuItem.Size = new System.Drawing.Size(216, 24);
            this.setGridRangesToolStripMenuItem.Text = "Set Grid Ranges";
            this.setGridRangesToolStripMenuItem.Click += new System.EventHandler(this.setGridRangesToolStripMenuItem_Click);
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.createEntityToolStripMenuItem});
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(57, 24);
            this.aboutToolStripMenuItem.Text = "Tools";
            // 
            // createEntityToolStripMenuItem
            // 
            this.createEntityToolStripMenuItem.Name = "createEntityToolStripMenuItem";
            this.createEntityToolStripMenuItem.Size = new System.Drawing.Size(162, 24);
            this.createEntityToolStripMenuItem.Text = "Create Entity";
            this.createEntityToolStripMenuItem.Click += new System.EventHandler(this.createEntityToolStripMenuItem_Click);
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(13, 507);
            this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(72, 17);
            this.label4.TabIndex = 10;
            this.label4.Text = "Threshold";
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.DecimalPlaces = 3;
            this.numericUpDown1.Increment = new decimal(new int[] {
            1,
            0,
            0,
            196608});
            this.numericUpDown1.Location = new System.Drawing.Point(17, 526);
            this.numericUpDown1.Margin = new System.Windows.Forms.Padding(4);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(89, 22);
            this.numericUpDown1.TabIndex = 11;
            this.numericUpDown1.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDown1.Value = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numericUpDown1.ValueChanged += new System.EventHandler(this.numericUpDownContributionThreshold_ValueChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(136, 507);
            this.label5.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(49, 17);
            this.label5.TabIndex = 14;
            this.label5.Text = "Speed";
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.DecimalPlaces = 3;
            this.numericUpDown2.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.numericUpDown2.Location = new System.Drawing.Point(140, 526);
            this.numericUpDown2.Margin = new System.Windows.Forms.Padding(4);
            this.numericUpDown2.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(98, 22);
            this.numericUpDown2.TabIndex = 15;
            this.numericUpDown2.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDown2.Value = new decimal(new int[] {
            100,
            0,
            0,
            196608});
            this.numericUpDown2.ValueChanged += new System.EventHandler(this.numericUpDown2_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(266, 507);
            this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(90, 17);
            this.label6.TabIndex = 16;
            this.label6.Text = "Refresh (ms)";
            // 
            // numericUpDown3
            // 
            this.numericUpDown3.Location = new System.Drawing.Point(270, 526);
            this.numericUpDown3.Margin = new System.Windows.Forms.Padding(4);
            this.numericUpDown3.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.numericUpDown3.Name = "numericUpDown3";
            this.numericUpDown3.Size = new System.Drawing.Size(102, 22);
            this.numericUpDown3.TabIndex = 17;
            this.numericUpDown3.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDown3.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDown3.ValueChanged += new System.EventHandler(this.numericUpDown3_ValueChanged);
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.BackColor = System.Drawing.Color.Transparent;
            this.checkBox1.ForeColor = System.Drawing.SystemColors.ControlText;
            this.checkBox1.Location = new System.Drawing.Point(17, 436);
            this.checkBox1.Margin = new System.Windows.Forms.Padding(4);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(89, 21);
            this.checkBox1.TabIndex = 18;
            this.checkBox1.Text = "Mass Dot";
            this.checkBox1.UseVisualStyleBackColor = false;
            this.checkBox1.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // checkBox2
            // 
            this.checkBox2.AutoSize = true;
            this.checkBox2.BackColor = System.Drawing.Color.Transparent;
            this.checkBox2.Location = new System.Drawing.Point(17, 465);
            this.checkBox2.Margin = new System.Windows.Forms.Padding(4);
            this.checkBox2.Name = "checkBox2";
            this.checkBox2.Size = new System.Drawing.Size(94, 21);
            this.checkBox2.TabIndex = 19;
            this.checkBox2.Text = "Mass Line";
            this.checkBox2.UseVisualStyleBackColor = false;
            this.checkBox2.CheckedChanged += new System.EventHandler(this.checkBox2_CheckedChanged);
            // 
            // checkBox3
            // 
            this.checkBox3.AutoSize = true;
            this.checkBox3.BackColor = System.Drawing.Color.Transparent;
            this.checkBox3.Location = new System.Drawing.Point(140, 436);
            this.checkBox3.Margin = new System.Windows.Forms.Padding(4);
            this.checkBox3.Name = "checkBox3";
            this.checkBox3.Size = new System.Drawing.Size(98, 21);
            this.checkBox3.TabIndex = 20;
            this.checkBox3.Text = "Fusion Dot";
            this.checkBox3.UseVisualStyleBackColor = false;
            this.checkBox3.CheckedChanged += new System.EventHandler(this.checkBox3_CheckedChanged);
            // 
            // checkBox4
            // 
            this.checkBox4.AutoSize = true;
            this.checkBox4.BackColor = System.Drawing.Color.Transparent;
            this.checkBox4.Location = new System.Drawing.Point(140, 465);
            this.checkBox4.Margin = new System.Windows.Forms.Padding(4);
            this.checkBox4.Name = "checkBox4";
            this.checkBox4.Size = new System.Drawing.Size(103, 21);
            this.checkBox4.TabIndex = 21;
            this.checkBox4.Text = "Fusion Line";
            this.checkBox4.UseVisualStyleBackColor = false;
            this.checkBox4.CheckedChanged += new System.EventHandler(this.checkBox4_CheckedChanged);
            // 
            // checkBox5
            // 
            this.checkBox5.AutoSize = true;
            this.checkBox5.BackColor = System.Drawing.Color.Transparent;
            this.checkBox5.Location = new System.Drawing.Point(270, 436);
            this.checkBox5.Margin = new System.Windows.Forms.Padding(4);
            this.checkBox5.Name = "checkBox5";
            this.checkBox5.Size = new System.Drawing.Size(102, 21);
            this.checkBox5.TabIndex = 22;
            this.checkBox5.Text = "Entities Dot";
            this.checkBox5.UseVisualStyleBackColor = false;
            this.checkBox5.CheckedChanged += new System.EventHandler(this.checkBox5_CheckedChanged);
            // 
            // checkBox6
            // 
            this.checkBox6.AutoSize = true;
            this.checkBox6.BackColor = System.Drawing.Color.Transparent;
            this.checkBox6.Location = new System.Drawing.Point(270, 465);
            this.checkBox6.Margin = new System.Windows.Forms.Padding(4);
            this.checkBox6.Name = "checkBox6";
            this.checkBox6.Size = new System.Drawing.Size(107, 21);
            this.checkBox6.TabIndex = 23;
            this.checkBox6.Text = "Entities Line";
            this.checkBox6.UseVisualStyleBackColor = false;
            this.checkBox6.CheckedChanged += new System.EventHandler(this.checkBox6_CheckedChanged);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 732);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Padding = new System.Windows.Forms.Padding(1, 0, 19, 0);
            this.statusStrip1.Size = new System.Drawing.Size(394, 25);
            this.statusStrip1.TabIndex = 24;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(78, 20);
            this.toolStripStatusLabel1.Text = "Mouse X Y";
            // 
            // log_box
            // 
            this.log_box.Location = new System.Drawing.Point(17, 564);
            this.log_box.Margin = new System.Windows.Forms.Padding(4);
            this.log_box.Multiline = true;
            this.log_box.Name = "log_box";
            this.log_box.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.log_box.Size = new System.Drawing.Size(355, 158);
            this.log_box.TabIndex = 25;
            // 
            // dblBufferedDrawControl1
            // 
            this.dblBufferedDrawControl1.Cursor = System.Windows.Forms.Cursors.Cross;
            this.dblBufferedDrawControl1.Location = new System.Drawing.Point(7, 35);
            this.dblBufferedDrawControl1.Margin = new System.Windows.Forms.Padding(5);
            this.dblBufferedDrawControl1.MinimumSize = new System.Drawing.Size(67, 62);
            this.dblBufferedDrawControl1.Name = "dblBufferedDrawControl1";
            this.dblBufferedDrawControl1.Size = new System.Drawing.Size(380, 380);
            this.dblBufferedDrawControl1.TabIndex = 9;
            this.dblBufferedDrawControl1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.dblBufferedDrawControl1_MouseMove);
            this.dblBufferedDrawControl1.MouseLeave += new System.EventHandler(this.dblBufferedDrawControl1_MouseLeave);
            this.dblBufferedDrawControl1.MouseClick += new System.Windows.Forms.MouseEventHandler(this.dblBufferedDrawControl1_MouseClick);
            this.dblBufferedDrawControl1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.dblBufferedDrawControl1_MouseDown);
            this.dblBufferedDrawControl1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.dblBufferedDrawControl1_MouseUp);
            this.dblBufferedDrawControl1.ClientSizeChanged += new System.EventHandler(this.dblBufferedDrawControl1_ClientSizeChanged);
            // 
            // Main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(394, 757);
            this.Controls.Add(this.log_box);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.checkBox6);
            this.Controls.Add(this.checkBox5);
            this.Controls.Add(this.checkBox4);
            this.Controls.Add(this.checkBox3);
            this.Controls.Add(this.checkBox2);
            this.Controls.Add(this.checkBox1);
            this.Controls.Add(this.numericUpDown3);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.numericUpDown2);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.numericUpDown1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.dblBufferedDrawControl1);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "Main";
            this.Text = "SSI / PAFusion";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown3)).EndInit();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.Timer timer1;
        private DblBufferedDrawControl dblBufferedDrawControl1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem settingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem listenportToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.NumericUpDown numericUpDown3;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.CheckBox checkBox2;
        private System.Windows.Forms.CheckBox checkBox3;
        private System.Windows.Forms.CheckBox checkBox4;
        private System.Windows.Forms.CheckBox checkBox5;
        private System.Windows.Forms.CheckBox checkBox6;
        private System.Windows.Forms.ToolStripMenuItem createEntityToolStripMenuItem;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripMenuItem alfredControlToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem alfredControlActiveToolStripMenuItem;
        private System.Windows.Forms.TextBox log_box;
        private System.Windows.Forms.ToolStripMenuItem undockChartToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem setGridRangesToolStripMenuItem;
    }
}

