using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.GraphicsView
{
    class InputComponent : GraphicsComponent
    {
        public string Text { get; set; }

        public override void OnPaint(System.Windows.Forms.PaintEventArgs e)
        {
			var sm = e.Graphics.SmoothingMode;
			e.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
			
			var brush = new SolidBrush(Color.FromArgb(192, 210, 238));
			var brLight = new LinearGradientBrush(Bounds, Color.FromArgb(100, Color.White), Color.FromArgb(100, Color.Black), 90, false);
			
            e.Graphics.FillEllipse(brush, Bounds);
            e.Graphics.FillEllipse(brLight, Bounds);
            
            e.Graphics.SmoothingMode = sm;
        }
    }
}
