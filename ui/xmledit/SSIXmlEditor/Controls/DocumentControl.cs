using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;

namespace SSIXmlEditor.Controls
{
	public class DocumentControl : TabControl
	{
		public DocumentControl()
		{
			this.SetStyle(ControlStyles.UserPaint, true);
			this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
			this.SetStyle(ControlStyles.DoubleBuffer, true);
			this.SetStyle(ControlStyles.ResizeRedraw, true);
			this.SetStyle(ControlStyles.SupportsTransparentBackColor, true);			
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);
			var g = e.Graphics;
			var rcTabCtrlArea = this.ClientRectangle;
			var rcTabArea = this.DisplayRectangle;
			
			int nDelta = SystemInformation.Border3DSize.Width;
			
			using(Pen border = new Pen(SystemColors.ControlDark))
			{
				rcTabArea.Inflate(nDelta, nDelta);
				g.DrawRectangle(border, rcTabArea);			
			}
			
			int nWidth = rcTabArea.Width + 5;
			var rsaved = g.Clip;
			var rcRegion = new Rectangle(rcTabArea.Left, rcTabCtrlArea.Top, nWidth - 5, rcTabCtrlArea.Height);
			g.SetClip(rcRegion);
			
			for(int i = 0; i < this.TabPages.Count; ++i)
			{
				var rcBounds = this.GetTabRect(i);
				var tabTextArea = (RectangleF)this.GetTabRect(i);
				
				Point[] pt = new Point[7];
				
				pt[0] = new Point(rcBounds.Left, rcBounds.Bottom);
				pt[1] = new Point(rcBounds.Left, rcBounds.Top + 3);
				pt[2] = new Point(rcBounds.Left + 3, rcBounds.Top);
				pt[3] = new Point(rcBounds.Right - 3, rcBounds.Top);
				pt[4] = new Point(rcBounds.Right, rcBounds.Top + 3);
				pt[5] = new Point(rcBounds.Right, rcBounds.Bottom);
				pt[6] = new Point(rcBounds.Left, rcBounds.Bottom);
			
				Brush br = new SolidBrush(Color.Red);
				g.FillPolygon(br, pt);
				g.DrawPolygon(SystemPens.ControlDarkDark, pt);
				br.Dispose();
				
				var sf = new StringFormat();
				sf.Alignment = StringAlignment.Center;
				sf.LineAlignment = StringAlignment.Center;
				
				br = new SolidBrush(TabPage.DefaultForeColor);
				g.DrawString(TabPages[i].Text, Font, br, tabTextArea, sf);
			}
								
			g.Clip = rsaved;
		}
		
		
	}
}
