namespace FusionUI
{
    partial class PureDraw
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
            if (disposing)
            {
                dblBufferedDrawControl1.Dispose();
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
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.dblBufferedDrawControl1 = new FusionUI.DblBufferedDrawControl();
            this.SuspendLayout();
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // dblBufferedDrawControl1
            // 
            this.dblBufferedDrawControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dblBufferedDrawControl1.Location = new System.Drawing.Point(0, 0);
            this.dblBufferedDrawControl1.MinimumSize = new System.Drawing.Size(50, 50);
            this.dblBufferedDrawControl1.Name = "dblBufferedDrawControl1";
            this.dblBufferedDrawControl1.Size = new System.Drawing.Size(479, 419);
            this.dblBufferedDrawControl1.TabIndex = 10;
            this.dblBufferedDrawControl1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.dblBufferedDrawControl1_MouseMove);
            this.dblBufferedDrawControl1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.dblBufferedDrawControl1_MouseDown);
            this.dblBufferedDrawControl1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.dblBufferedDrawControl1_MouseUp);
            // 
            // PureDraw
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(479, 419);
            this.Controls.Add(this.dblBufferedDrawControl1);
            this.Name = "PureDraw";
            this.Text = "PureDraw";
            this.Load += new System.EventHandler(this.PureDraw_Load);
            this.VisibleChanged += new System.EventHandler(this.PureDraw_VisibleChanged);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.PureDraw_FormClosed);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.PureDraw_FormClosing);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Timer timer1;
        private DblBufferedDrawControl dblBufferedDrawControl1;
    }
}