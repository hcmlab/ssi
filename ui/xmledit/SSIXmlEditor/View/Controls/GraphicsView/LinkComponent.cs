using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;

namespace SSIXmlEditor.Controls.GraphicsView
{
    class LinkComponent : GraphicsComponent
    {
        public OutputComponent Output { get; set; }
        public InputComponent Input { get; set; }

		public override Rectangle Bounds
		{
			get
			{
				return new Rectangle(Output.Bounds.Right, 
									 Output.Bounds.Top + 10,
									 Output.Bounds.Right - Input.Bounds.Left, 
									 Output.Bounds.Top - Input.Bounds.Top);
			}
			set
			{
				//base.Bounds = value;
			}
		}

        public override void OnPaint(System.Windows.Forms.PaintEventArgs e)
        {
            var rcLine = new Rectangle(Output.Bounds.Right, Output.Bounds.Top, Input.Bounds.Left, Input.Bounds.Y + Input.Bounds.Height);

			var sm = e.Graphics.SmoothingMode;
			e.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
			
            e.Graphics.DrawLine(Pens.Black, new Point(Output.Bounds.Right, Output.Bounds.Top + 10), 
											new Point(Input.Bounds.Left, Input.Bounds.Top + 10));

			e.Graphics.SmoothingMode = sm;
            rcLine.Offset(0, -15);
            
            e.Graphics.DrawString(Input.Text, SystemFonts.DefaultFont, Brushes.Black, rcLine);
        }
    }
}
