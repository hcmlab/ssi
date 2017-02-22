using SSIXmlEditor.Controls.GraphicsView;

namespace SSIXmlEditor.View
{
	partial class GraphicalView
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
            this.graphicaView1 = new SSIXmlEditor.Controls.GraphicsView.GraphicaView();
            this.SuspendLayout();
            // 
            // graphicaView1
            // 
            this.graphicaView1.AutoScroll = true;
            this.graphicaView1.AutoScrollMinSize = new System.Drawing.Size(500, 110);
            this.graphicaView1.BackColor = System.Drawing.Color.White;
            this.graphicaView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.graphicaView1.Location = new System.Drawing.Point(0, 0);
            this.graphicaView1.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.graphicaView1.Name = "graphicaView1";
            this.graphicaView1.Size = new System.Drawing.Size(1114, 755);
            this.graphicaView1.TabIndex = 0;
            // 
            // GraphicalView
            // 
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1114, 755);
            this.Controls.Add(this.graphicaView1);
            this.Margin = new System.Windows.Forms.Padding(4, 5, 4, 5);
            this.Name = "GraphicalView";
            this.Text = "GraphicalView";
            this.ResumeLayout(false);

		}

		#endregion

		private GraphicaView graphicaView1;
	}
}