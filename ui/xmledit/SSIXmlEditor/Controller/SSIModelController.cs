using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.View;

namespace SSIXmlEditor.Controller
{
    public class SSIModelController
    {
        private Model.SSIModel m_Model;
        private View.ISSIModelView m_View;

        public SSIModelController(Model.SSIModel model, View.ISSIModelView view)
        {
            if (null == model)
                throw new ArgumentNullException("model");
            if (null == view)
                throw new ArgumentNullException("view");

            m_Model = model;
            m_View = view;

            m_Model.SelectionChanged += new EventHandler(m_Model_SelectionChanged);
            m_View.ModelChanged += new EventHandler<SSIModelChangedEventArgs>(m_View_ModelChanged);
            
        }

        void m_View_ModelChanged(object sender, SSIModelChangedEventArgs e)
        {
			m_Model.Change(e);
        }

        void m_Model_SelectionChanged(object sender, EventArgs e)
        {
			m_View.Selected = m_Model.Selected;
        }
    }
}
