using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.View
{
    public interface ITextEditorView : IView
    {
        event EventHandler TextChanged;
        event EventHandler TextEntering;
        event EventHandler TextEntered;
        event EventHandler CaretChanged;

        int CaretPosition { get; set; }

        string GetCurrentLineText();
    }
}
