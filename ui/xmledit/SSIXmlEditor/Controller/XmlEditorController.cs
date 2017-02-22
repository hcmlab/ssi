using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ICSharpCode.AvalonEdit;

namespace SSIXmlEditor.Controller
{
    public class XmlEditorController
    {
        private Model.IInputHandler m_Model;
        private View.ITextEditorView m_View;
        
        public XmlEditorController(Model.IInputHandler model, View.ITextEditorView view)
        {
            if (null == model)
                throw new ArgumentNullException("model");
            if (null == view)
                throw new ArgumentNullException("view");

            m_View = view;
            m_Model = model;

            view.CaretChanged += new EventHandler(view_CaretChanged);
            view.TextChanged += new EventHandler(view_TextChanged);
            view.TextEntered += new EventHandler(view_TextEntered);
            view.TextEntering += new EventHandler(view_TextEntering);
        }

        void view_TextEntering(object sender, EventArgs e)
        {
            //m_Model.TextEntering("");
        }

        void view_TextEntered(object sender, EventArgs e)
        {
            //m_Model.TextEntered("");
        }

        void view_TextChanged(object sender, EventArgs e)
        {
            //m_Model.TextChanged();
        }

        void view_CaretChanged(object sender, EventArgs e)
        {
            //m_Model.CaretChanged(m_View.CaretPosition);
        }
    }
}
