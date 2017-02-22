using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.Document;

namespace SSIXmlEditor.Controller
{
    class DocumentsController
    {
        private App.SingleInstanceApp m_App;
        private Document.Documents m_Model;
        private View.IDocumentsView m_View;

        public DocumentsController(App.SingleInstanceApp app, Document.Documents model, View.IDocumentsView view)
        {
            if (null == app)
                throw new ArgumentNullException();
            if (null == model)
                throw new ArgumentNullException("model");
            if (null == view)
                throw new ArgumentNullException("view");

            m_App = app;
            m_Model = model;
            m_View = view;

            app.ActiveDocumentChanged += new EventHandler(app_ActiveDocumentChanged);
            m_Model.Added += new EventHandler<DocumentsEventArgs>(m_Model_Added);
            m_Model.Removed += new EventHandler<DocumentsEventArgs>(m_Model_Removed);
        }

        void app_ActiveDocumentChanged(object sender, EventArgs e)
        {
            var doc = m_App.ActiveDocument;
            
            if(null != doc)
				m_View.Show(doc.ID);
        }

        void m_Model_Removed(object sender, DocumentsEventArgs e)
        {
            m_View.Remove(e.Document);

            Document.ADocument doc = null;
            try
            {
                doc = m_Model.Last();
            }
            catch (Exception) {}

            if (null != doc)
                doc.Activate();
            else
                m_App.ActiveDocument = null;            
        }

        void m_Model_Added(object sender,  DocumentsEventArgs e)
        {
            m_View.Add(e.Document);
        }

        public void Activate(int id)
        {
            try
            {
                var doc = m_Model.Single(c => c.ID == id);
                doc.Activate();
            }
            catch (Exception)
            {
            }
        }
    }
}
