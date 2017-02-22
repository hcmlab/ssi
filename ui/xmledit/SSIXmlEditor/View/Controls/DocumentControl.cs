using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using SSIXmlEditor.View;

namespace SSIXmlEditor.Controls
{
	public class DocumentControl : System.Windows.Forms.TabControl, View.IDocumentsView
	{
        private Controller.DocumentsController m_Controller;

        #region IDocumentsView Members

        public IEnumerable<IDocumentView> Documents
        {
            set 
            {
                if (null == value)
                    throw new ArgumentNullException("Documents");
            }
        }

        public void Add(Document.ADocument document)
        {
            var ctrl = new EditorTabPage(document);
            ctrl.Name = document.ID.ToString();
            ctrl.Tag = document.ID;
           
            Controls.Add(ctrl);
            
			this.SelectedTab = ctrl;
			//AddPage(ctrl);
        }

        public void Remove(Document.ADocument document)
        {
			var ctrl = Controls[document.ID.ToString()];
            Controls.Remove(ctrl);
            //RemovePage(document.ID.ToString());
        }

        #endregion

        public DocumentControl(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

            m_Controller = new Controller.DocumentsController(app, app.Documents, this);
        }

        protected override void OnSelectedIndexChanged(EventArgs e)
        {
            base.OnSelectedIndexChanged(e);

            var tab = this.SelectedTab;
            if (null == tab)
                return;

            //int id = (int)tab.Control.Tag;
			int id = (int)tab.Tag;
			
            if (this.SelectedTab != null)
                m_Controller.Activate(id);
        }


        #region IDocumentsView Members


        public void Show(int id)
        {
            var tab = Controls[id.ToString()];
            //var tab = GetByIndex(id.ToString());
			
            this.SelectedTab = tab as TabPage;
        }

        #endregion
    }
}
