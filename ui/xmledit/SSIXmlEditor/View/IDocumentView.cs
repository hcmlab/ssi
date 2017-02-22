using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.View
{
    public interface IDocumentView : IView
    {
        int CaretPosition { get; set; }
        string DocumentName { get; set; }
    }
}
