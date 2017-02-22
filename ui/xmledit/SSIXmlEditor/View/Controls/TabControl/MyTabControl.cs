using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.TabControl
{
    internal delegate void MouseDelegate(MouseEventArgs e);

    public partial class MyTabControl : System.Windows.Forms.Control
    {
        private Rectangle m_MainBounds;
        private int m_HeaderHeight;
        private int m_FirstTab;

        private ITabRenderer m_TabRenderer;
        private TabPage m_ActiveTabPage;
        private List<TabPage> m_Pages;
        

        public override Color BackColor
        {
            get
            {
                return Color.Transparent;
            }
            set
            {
                //base.BackColor = value;
            }
        }

        internal Rectangle TabBounds
        {
            get { return new Rectangle(0, 0, Width - 50, m_HeaderHeight); }
        }

        internal Rectangle TabMovementBounds
        {
            get { return new Rectangle(Width - 50, 0, 50, m_HeaderHeight); }
        }

        internal IEnumerable<TabPage> TabPages 
        { 
            get 
            {
                return m_Pages.Skip(m_FirstTab);
            } 
        }

        private int TabPagesWidth
        {
            get
            {
                int tpWidth = 0;
                m_Pages.ForEach(tp => tpWidth += tp.Bounds.Width);
                return tpWidth;
            }
        }

        internal TabPage SelectedTab
        {
            get { return m_ActiveTabPage; }
            set
            {
				if(!(m_ActiveTabPage == null) && m_ActiveTabPage == value)
					return;
            
                int index = 0;
                if (null != m_ActiveTabPage)
                {
                    m_ActiveTabPage.State = TabPageState.None;
                    
                    index = Controls[0].Controls.GetChildIndex(m_ActiveTabPage.Control);
                    Controls[0].Controls[index].Visible = false;
                }               
                                
                m_ActiveTabPage = value;
                m_ActiveTabPage.State = TabPageState.Active;

                if (!Controls[0].Controls.Contains(m_ActiveTabPage.Control))
                {
                    Controls[0].Controls.Add(m_ActiveTabPage.Control);
                }

                index = Controls[0].Controls.GetChildIndex(m_ActiveTabPage.Control);
                Controls[0].Controls[index].Visible = true;
                
                OnSelectedIndexChanged(EventArgs.Empty);
            }
        }

        internal MouseDelegate MouseMoveDelegate { get; set; }
        internal MouseDelegate MouseDownDelegate { get; set; }
        internal MouseDelegate MouseUpDelegate { get; set; }

        public MyTabControl()
        {
            this.SetStyle(ControlStyles.UserPaint, true);
            this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            this.SetStyle(ControlStyles.DoubleBuffer, true);
            this.SetStyle(ControlStyles.ResizeRedraw, true);
            this.SetStyle(ControlStyles.Selectable, true);
            this.SetStyle(ControlStyles.SupportsTransparentBackColor, true);

            m_HeaderHeight = 24;
            m_FirstTab = 0;
            m_Pages = new List<TabPage>();
																
            Controls.Add(new Panel() { Bounds = m_MainBounds });

            m_TabRenderer = new TabRenderer() { TabControl = this };

            InitializeComponent();

            this.BackColor = Color.Transparent;
        }        

        public void AddPage(Control ctrl)
        {
            var tp = new TabPage() { TabControl = this, Control = ctrl };
            m_Pages.Add(tp);
            
            ctrl.TextChanged += delegate { Invalidate(tp.Bounds); };

            if (SelectedTab == null)
				SelectedTab = tp;
			
			Invalidate(TabBounds);
        }
        
        public void RemovePage(string name)
        {
			var page = m_Pages.Where(p => p.Control.Name.Equals(name)).Single();
			m_Pages.Remove(page);  			
			
			if(page == SelectedTab)
				m_ActiveTabPage = null;
				
			Controls[0].Controls.Remove(page.Control);
			Invalidate();
        }
        
        internal TabPage GetByIndex(string index)
		{
			return m_Pages.Where(p => p.Control.Name.Equals(index)).Single();
		}
        
        protected virtual void OnSelectedIndexChanged(EventArgs e)
        {
        
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);

            m_MainBounds = new Rectangle(0, m_HeaderHeight, Width, Height - m_HeaderHeight);
            Controls[0].Bounds = m_MainBounds;
        }

        protected override void OnPaintBackground(PaintEventArgs pevent)
        {
            base.OnPaintBackground(pevent);            
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            var g = e.Graphics;
            
            if(m_Pages.Count == 0)
            {
				g.FillRectangle(Brushes.White, ClientRectangle);
				return;
			}
            
            g.FillRectangle(Brushes.White, m_MainBounds);
            
            var clip = e.Graphics.Clip;
            var rcTabs = this.TabBounds;
            
            e.Graphics.SetClip(rcTabs);
            m_TabRenderer.OnPaint(e);

            e.Graphics.Clip = clip;

            if (TabPagesWidth >= rcTabs.Width)
            {
                var sm = e.Graphics.SmoothingMode;
                e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;

                var rcMovement = TabMovementBounds;
                var leftArrow = new GraphicsPath();
                leftArrow.AddLine(rcMovement.X + 5, (rcMovement.Y + rcMovement.Height) >> 1, rcMovement.X + 20, rcMovement.Y + 5);
                leftArrow.AddLine(rcMovement.X + 20, rcMovement.Y + 5, rcMovement.X + 20, (rcMovement.Y + rcMovement.Height) - 5);
                leftArrow.AddLine(rcMovement.X + 20, (rcMovement.Y + rcMovement.Height) - 5, rcMovement.X + 5, (rcMovement.Y + rcMovement.Bottom) >> 1);
                leftArrow.CloseFigure();

                var rightArrow = leftArrow.Clone() as GraphicsPath;
                
                var matrix = new Matrix();
                matrix.RotateAt(180, new PointF(rcMovement.X + (25 / 2), (rcMovement.Y + rcMovement.Height) / 2));
                matrix.Translate(-20, 0);                
                rightArrow.Transform(matrix);

                e.Graphics.FillPath(Brushes.Red, leftArrow);
                e.Graphics.FillPath(Brushes.Red, rightArrow);

                e.Graphics.SmoothingMode = sm;
            }
        }

		protected override void OnMouseLeave(EventArgs e)
		{
			base.OnMouseLeave(e);
			
			if(null != MouseMoveDelegate)
				MouseMoveDelegate(new MouseEventArgs(MouseButtons.None, 0, MousePosition.X, MousePosition.Y, 0));
		}       

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);

            if (null == MouseMoveDelegate)
            {
                if (m_MainBounds.Contains(e.Location))
                    return;

                var tabPages = TabPages.Where(tp => TabBounds.Contains(e.Location) && tp.Bounds.Contains(e.Location));
                if (tabPages.Count() == 0)
                    return;

                var tabPage = tabPages.First();
                MouseMoveDelegate = tabPage.MouseMoveDelegate;
                MouseDownDelegate = tabPage.MouseDownDelegate;
            }

            MouseMoveDelegate(e);
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown(e);

            if (null != MouseDownDelegate)
                MouseDownDelegate(e);
            else if (TabMovementBounds.Contains(e.Location))
            {
                var rcMoveLeft = new Rectangle(TabMovementBounds.X + 5, TabMovementBounds.Y, 20, m_HeaderHeight);

                if (rcMoveLeft.Contains(e.Location))
                {
                    m_FirstTab = m_FirstTab >= m_Pages.Count - 1 ? m_FirstTab = m_Pages.Count - 1 : m_FirstTab + 1;
                }
                else
                    m_FirstTab = m_FirstTab < 0 ? m_FirstTab = 0 : m_FirstTab - 1;


                Invalidate(TabBounds);
            }
        }
    }
}
