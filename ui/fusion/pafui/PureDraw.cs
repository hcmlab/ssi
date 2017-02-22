using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FusionUI
{
    public partial class PureDraw : Form
    {

        private Main _parentForm = null;

        public PureDraw(Main parentForm, RectangleF gridBoundariesInCartesianCoordinates)
        {
            _parentForm = parentForm;
            InitializeComponent();

            dblBufferedDrawControl1.SetDrawingSurfaceRangeinCartesianCoordinates(gridBoundariesInCartesianCoordinates);
        }

        public void drawWorks(DblBufferedDrawControl ctrl, Graphics graphics)
        {
            _parentForm.drawWorks(ctrl, graphics);
            //ctrl._paintHelper.DrawGrid();            
        }

        private void PureDraw_Load(object sender, EventArgs e)
        {
            dblBufferedDrawControl1.OnPaintReceived += new DblBufferOnPaintEventHandler(drawWorks);
        }

        private void PureDraw_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (e.CloseReason == CloseReason.UserClosing)
            {
                _parentForm.RedockChartToolStripMenuItem();
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (this.Visible == true)
            {
                dblBufferedDrawControl1.Refresh();
            }
        }

        private void PureDraw_FormClosed(object sender, FormClosedEventArgs e)
        {
            
            _parentForm.ChartFormClosed();
        }

        private void PureDraw_VisibleChanged(object sender, EventArgs e)
        {
            
        }

        public void RefreshDrawingControl()
        {
            dblBufferedDrawControl1.Refresh();
        }

        public void ChangeCartesianGridBoundaries(PointF upperLeftCorner, PointF lowerRightCorner)
        {
            dblBufferedDrawControl1.SetDrawingSurfaceRangeInCartesianCoordinates(upperLeftCorner, lowerRightCorner);
        }

        private void dblBufferedDrawControl1_MouseDown(object sender, MouseEventArgs e)
        {
            _parentForm.dblBufferedDrawControl1_MouseDown(sender, e);
        }

        private void dblBufferedDrawControl1_MouseUp(object sender, MouseEventArgs e)
        {
            _parentForm.dblBufferedDrawControl1_MouseUp(sender, e);
        }

        private void dblBufferedDrawControl1_MouseMove(object sender, MouseEventArgs e)
        {
            _parentForm.dblBufferedDrawControl1_MouseMove(sender, e);
        }
    }
}
