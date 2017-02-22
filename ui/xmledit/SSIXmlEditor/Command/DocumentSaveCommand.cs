using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Command
{
    class DocumentSaveCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public DocumentSaveCommand(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

            m_App = app;
        }

        #region ICommand Members

        public void Execute()
        {
            var doc = m_App.ActiveDocument;
            if(null == doc)
				return;
            
            if (doc.File == null)
                m_App.InvokeCommand("SaveAs");
            else
                doc.Save();
        }

        #endregion
    }
}
