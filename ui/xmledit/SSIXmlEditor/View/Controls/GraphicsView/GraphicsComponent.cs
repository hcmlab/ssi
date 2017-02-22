using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.GraphicsView
{
    internal abstract class GraphicsComponent
    {
        public virtual Rectangle Bounds { get; set; }
        public Brush BackgroundBrush { get; set; }

        public abstract void OnPaint(PaintEventArgs e);

        public static GraphicsPath CreateRoundRect(float X, float Y, float width, float height, float radius)
        {
            GraphicsPath gp = new GraphicsPath();

            gp.AddLine(X + radius, Y, X + width - (radius * 2), Y);

            gp.AddArc(X + width - (radius * 2), Y, radius * 2, radius * 2, 270, 90);

            gp.AddLine(X + width, Y + radius, X + width, Y + height - (radius * 2));

            gp.AddArc(X + width - (radius * 2), Y + height - (radius * 2), radius * 2, radius * 2, 0, 90);

            gp.AddLine(X + width - (radius * 2), Y + height, X + radius, Y + height);

            gp.AddArc(X, Y + height - (radius * 2), radius * 2, radius * 2, 90, 90);

            gp.AddLine(X, Y + height - (radius * 2), X, Y + radius);

            gp.AddArc(X, Y, radius * 2, radius * 2, 180, 90);

            gp.CloseFigure();
            return gp;
        }
    }
}
