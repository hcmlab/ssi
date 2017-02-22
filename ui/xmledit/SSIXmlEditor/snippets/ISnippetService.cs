using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Snippets
{
    public interface ISnippetService
    {
        Snippet Load(string snippetPath);
    }
}
