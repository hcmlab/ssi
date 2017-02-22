using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace SSIXmlEditor.Controls.TabControl
{
    public enum TabPageState
    {
        None,
        Active,
        ActiveHover,
        NoneHover
    }

    internal class TabPage
    {
        private MouseDelegate m_MouseMoveDelegate;
        private MouseDelegate m_MouseDownDelegate;
        private TabPageState m_State;
        private TabPageState? m_PreviousState;

        internal MyTabControl TabControl { get; set; }
        internal object Tag { get; set; }
        internal Rectangle Bounds { get; set; }
       
        public string Text { get { return Control.Text; } }
        public TabPageState State
        {
            get { return m_State; }
            set
            {
                if (m_State == value)
                    return;

                m_State = value;
                TabControl.Invalidate(TabControl.TabBounds);
            }
        }

        public Control Control { get; set; }

        internal MouseDelegate MouseMoveDelegate
        {
            get { return m_MouseMoveDelegate ?? (m_MouseMoveDelegate = new MouseDelegate(OnMouseMove)); }
        }

        internal MouseDelegate MouseDownDelegate
        {
            get { return m_MouseDownDelegate ?? (m_MouseDownDelegate = new MouseDelegate(OnMouseDown)); }
        }

        public TabPage()
        {
            State = TabPageState.None;
        }

        private void OnMouseMove(MouseEventArgs e)
        {
            if (!m_PreviousState.HasValue)
                m_PreviousState = State;

            if (!Bounds.Contains(e.Location))
            {
                State = m_PreviousState.Value;
                m_PreviousState = null;

                TabControl.MouseMoveDelegate = null;
                TabControl.MouseDownDelegate = null;
                return;
            }

            State = State == TabPageState.Active || State == TabPageState.ActiveHover ? TabPageState.ActiveHover : TabPageState.NoneHover;        
        }

        private void OnMouseDown(MouseEventArgs e)
        {
			if(State != TabPageState.Active && State != TabPageState.ActiveHover)
			{
	        	State = TabPageState.Active;
				m_PreviousState = State;
				TabControl.SelectedTab = this;
			}
        }

        private void OnMouseUp(MouseEventArgs e)
        {

        }

    }
}
