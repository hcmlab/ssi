using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.TabControl
{
    public class TabRenderer : ITabRenderer
    {
        private StringFormat m_StringFormat;

        internal MyTabControl TabControl { get; set; }


        public TabRenderer()
        {
            m_StringFormat = new StringFormat();
            m_StringFormat.Alignment = StringAlignment.Near;
            m_StringFormat.LineAlignment = StringAlignment.Center;
            m_StringFormat.Trimming = StringTrimming.Character;
        }

        public void OnPaint(PaintEventArgs e)
        {
            int x = 0;
            var g = e.Graphics;
            var smoothingMode = g.SmoothingMode;
            g.SmoothingMode = SmoothingMode.AntiAlias;

            foreach (var tab in TabControl.TabPages)
            {
                var cText = e.Graphics.MeasureString(tab.Text, SystemFonts.DefaultFont);
                var rcTabPage = new Rectangle(x, 0, (int)cText.Width + 10, 24);
                tab.Bounds = rcTabPage;

				x += rcTabPage.Width;
				if (!e.ClipRectangle.IntersectsWith(rcTabPage))
					continue;

                switch (tab.State)
                {
                    case TabPageState.Active:
                        DrawActiveTab(tab, rcTabPage, e);
                        break;
                    case TabPageState.NoneHover:
                        DrawInactiveTab(tab, rcTabPage, e);
                        DrawHoveredTab(tab, rcTabPage, e);
                        break;
                    case TabPageState.ActiveHover:
                        DrawActiveTab(tab, rcTabPage, e);
                        DrawHoveredTab(tab, rcTabPage, e);
                        break;
                    case TabPageState.None:
                        DrawInactiveTab(tab, rcTabPage, e);
                        break;
                }
            }

            g.SmoothingMode = smoothingMode;
        }

        private void DrawActiveTab(TabPage page, Rectangle bounds, PaintEventArgs e)
        {
            //var brush = Brushes.Blue;
            var brush = new SolidBrush(Color.FromArgb(255, 192, 210, 238));

            var path = CreateRoundRect(bounds, 4);
            e.Graphics.FillPath(brush, path);

            var rcText = bounds;
            rcText.Inflate(-2, 0);

            e.Graphics.DrawString(page.Text, SystemFonts.DefaultFont, Brushes.Black, rcText, m_StringFormat);
        }

        private void DrawHoveredTab(TabPage page, Rectangle bounds, PaintEventArgs e)
        {
            //var brush = new SolidBrush(Color.FromArgb(50, Color.White));
            var brush = new LinearGradientBrush(bounds, Color.FromArgb(100, Color.Black), Color.FromArgb(100, Color.White), 90, false);

            var path = CreateRoundRect(bounds, 4);
            e.Graphics.FillPath(brush, path);

            var rcText = bounds;
            rcText.Inflate(-2, 0);

            //e.Graphics.DrawString(page.Text, SystemFonts.DefaultFont, Brushes.Black, rcText, m_StringFormat);
        }

        private void DrawInactiveTab(TabPage page, Rectangle bounds, PaintEventArgs e)
        {
            var brush = new SolidBrush(Color.FromArgb(10, Color.Blue));

            var path = CreateRoundRect(bounds, 4);
            e.Graphics.FillPath(brush, path);

            var rcText = bounds;
            rcText.Inflate(-2, 0);

            e.Graphics.DrawString(page.Text, SystemFonts.DefaultFont, Brushes.White, rcText, m_StringFormat);
        }

        private GraphicsPath CreateRoundRect(Rectangle bounds, int radius)
        {
            GraphicsPath path = new GraphicsPath();

            path.AddLine(bounds.X + radius, bounds.Y, bounds.X + bounds.Width - (radius << 1), bounds.Y);
            path.AddArc(bounds.X + bounds.Width - (radius << 1), bounds.Y, radius << 1, radius << 1, 270, 90);
            path.AddLine(bounds.X + bounds.Width, bounds.Y + radius, bounds.X + bounds.Width, bounds.Y + bounds.Height);
            path.AddLine(bounds.X + bounds.Width, bounds.Y + bounds.Height, bounds.X, bounds.Y + bounds.Height);
            path.AddLine(bounds.X, bounds.Y + bounds.Height, bounds.X, bounds.Y + radius);
            path.AddArc(bounds.X, bounds.Y, radius << 1, radius << 1, 180, 90);

            path.CloseFigure();

            return path;
        }
    }
}
