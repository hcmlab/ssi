using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.Filter;

namespace SSIXmlEditor.InputHandler
{
    class KeyWordDetector 
    {
        private Filter.KeyWordFilter m_Filter;
        private Model.IInputHandler m_InputHandler;
        private Action<string> m_OnFilteredAction;

        public Model.IInputHandler InputHandler
        {
            private get { return m_InputHandler; }
            set
            {
                if (null == value)
                    throw new ArgumentNullException("InputHandler");

                if (m_InputHandler != null)
                {
                    m_InputHandler.TextChanged -= inputHandler_TextChanged;
                }

                m_InputHandler = value;
                value.TextChanged += inputHandler_TextChanged;
            }
        }

        public KeyWordDetector(Action<string> onFilteredAction)
        {
            m_Filter = new KeyWordFilter(new string[] { "ssi_sensor_Audio" }) { FilterStrategy = new Filter.Strategy.XmlAttributeFilterStrategy("create") };
            m_Filter.Found += new EventHandler<KeyWordFilterEventArgs>(m_Filter_Found);
            m_OnFilteredAction = onFilteredAction;
        }

        void inputHandler_TextChanged(object sender, EventArgs e)
        {
            m_Filter.Filter(m_InputHandler.GetCurrentLineText());
        }

        void m_Filter_Found(object sender, KeyWordFilterEventArgs e)
        {
            m_OnFilteredAction(e.Filtered);
        }

        
    }
}
