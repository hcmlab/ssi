using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.Document;

namespace SSIXmlEditor.Controller
{
    public class DocumentController
    {
        private View.IDocumentView View { get; set; }
        private Document.ADocument Document { get; set; }

        public DocumentController(Document.ADocument document, View.IDocumentView view)
        {
            if (null == document)
                throw new ArgumentNullException("document");
            if (null == view)
                throw new ArgumentNullException("view");

            Document = document;
            View = view;

            View.DocumentName = document.FileName;
            ChangeState(document.Changed);

            Document.StateChanged += new EventHandler<DocumentStateChangedEventArg>(Document_StateChanged);
        }

        void Document_StateChanged(object sender, DocumentStateChangedEventArg e)
        {
            ChangeState(e.Modified);
        }

        void ChangeState(bool bState)
        {
            if (bState)
                View.DocumentName = Document.FileName + "*";
            else
                View.DocumentName = Document.FileName;
        }
    }
}
