using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FusionUI
{
    public delegate void DblBufferOnPaintEventHandler(DblBufferedDrawControl ctrl, Graphics graphics);

    public partial class DblBufferedDrawControl : UserControl
    {
        const Graphics NO_BUFFER_GRAPHICS = null;
        const Bitmap NO_BACK_BUFFER = null;
        const BufferedGraphics NO_MANAGED_BACK_BUFFER = null;

        public PaintHelper _paintHelper = null;
        private RectangleF _defaultGridRangeInCartesianCoordinates = new RectangleF(-1.0f, -1.0f, 2.0f, 2.0f);

        BufferedGraphicsContext GraphicManager = new BufferedGraphicsContext ();
        BufferedGraphics ManagedBackBuffer;

        public event DblBufferOnPaintEventHandler OnPaintReceived;

        public DblBufferedDrawControl() :
            this(new RectangleF(-1.0f, -1.0f, 2.0f, 2.0f))
        {
        }

        public DblBufferedDrawControl(RectangleF tmpRect)
        {
            InitializeComponent();

            this.SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint, true);

            //this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            GraphicManager = BufferedGraphicsManager.Current;
            GraphicManager.MaximumBuffer = new Size(this.Width + 1, this.Height + 1);
            Graphics tmpGraphics = this.CreateGraphics();
            ManagedBackBuffer = GraphicManager.Allocate(tmpGraphics, ClientRectangle);
            tmpGraphics.Dispose();

            Application.ApplicationExit += new EventHandler(MemoryCleanup);

            
            tmpGraphics = null;
            try
            {
                tmpGraphics = ManagedBackBuffer.Graphics;
            }
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show("Exception: " + ex.Message, "Error!", MessageBoxButtons.OK);
                tmpGraphics = null;
            }

            _paintHelper = new PaintHelper(tmpGraphics, this.ClientSize, tmpRect);
            
            
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            try
            {
                OnPaintReceived(this, ManagedBackBuffer.Graphics);

                // paint the picture in from the back buffer into the form draw area
                ManagedBackBuffer.Render(e.Graphics);
            }
            catch (Exception Exp) { Console.WriteLine(Exp.Message); }

        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            //base.OnPaintBackground(e);

        }

        private void MemoryCleanup(object sender, EventArgs e)
        {
            if (ManagedBackBuffer != NO_MANAGED_BACK_BUFFER)
            {
                ManagedBackBuffer.Dispose();
                ManagedBackBuffer = NO_MANAGED_BACK_BUFFER;
            }
        }

        private void DoubleBufferControl_Resize(object sender, EventArgs e)
        {
            GraphicManager.MaximumBuffer = new Size(this.Width + 1, this.Height + 1);

            if (ManagedBackBuffer != NO_MANAGED_BACK_BUFFER)
            {
                ManagedBackBuffer.Dispose();
                ManagedBackBuffer = NO_MANAGED_BACK_BUFFER;
            }

            Graphics tmpGraphics = this.CreateGraphics();
            ManagedBackBuffer = GraphicManager.Allocate(tmpGraphics, ClientRectangle);
            tmpGraphics.Dispose();

            tmpGraphics = null;
            try
            {
                tmpGraphics = ManagedBackBuffer.Graphics;
            }
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show("Exception: " + ex.Message, "Error!", MessageBoxButtons.OK);
                tmpGraphics = null;
            }

            RectangleF tmpRect;
            
            if (_paintHelper != null)
            {
                tmpRect = _paintHelper.SizeOfDrawingSpaceCartesian;
            }
            else
            {
                tmpRect = _defaultGridRangeInCartesianCoordinates;
                
            }
            //tmpRect = new RectangleF(-10.35f, -2.1f, 103.0f, 22.56f);
            SetDrawingSurfaceRangeinCartesianCoordinates(tmpRect);

            this.Refresh();
        }

        public void SetDrawingSurfaceRangeinCartesianCoordinates(RectangleF range)
        {
            if (_paintHelper != null)
            {
                _paintHelper.Dispose();
                _paintHelper = null;
            }
            Graphics tmpGraphics = null;
            try
            {
                tmpGraphics = ManagedBackBuffer.Graphics;
            }
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show("Exception: " + ex.Message, "Error!", MessageBoxButtons.OK);
                tmpGraphics = null;
            }
            _paintHelper = new PaintHelper(tmpGraphics, this.ClientSize, range, false);
        }

        public void SetDrawingSurfaceRangeInCartesianCoordinates(PointF upperLeftCorner, PointF lowerRightCorner)
        {
            RectangleF range = new RectangleF(upperLeftCorner.X, lowerRightCorner.Y, lowerRightCorner.X - upperLeftCorner.X, upperLeftCorner.Y - lowerRightCorner.Y);
            SetDrawingSurfaceRangeinCartesianCoordinates(range);
        }
        
        private void DblBufferedDrawControl_Load(object sender, EventArgs e)
        {
            
        }

    }
}
