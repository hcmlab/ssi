namespace FusionUI
{
    partial class DblBufferedDrawControl
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
                if (ManagedBackBuffer != NO_MANAGED_BACK_BUFFER)
                {
                    ManagedBackBuffer.Dispose();
                    ManagedBackBuffer = NO_MANAGED_BACK_BUFFER;
                }

                if (_paintHelper != null)
                {
                    _paintHelper.Dispose();
                    _paintHelper = null;
                }
            }

            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // DblBufferedDrawControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Cursor = System.Windows.Forms.Cursors.Cross;
            this.MinimumSize = new System.Drawing.Size(50, 50);
            this.Name = "DblBufferedDrawControl";
            this.Size = new System.Drawing.Size(340, 300);
            this.Load += new System.EventHandler(this.DblBufferedDrawControl_Load);
            this.SizeChanged += new System.EventHandler(this.DoubleBufferControl_Resize);
            this.ResumeLayout(false);

        }

        #endregion
    }
}
