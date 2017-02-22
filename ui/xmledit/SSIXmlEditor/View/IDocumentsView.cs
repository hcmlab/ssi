using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.View
{
    public interface IDocumentsView : IView 
    {
        IEnumerable<IDocumentView> Documents { set; }

        void Add(Document.ADocument document);
        void Remove(Document.ADocument document);
        void Show(int id);
    }
}
