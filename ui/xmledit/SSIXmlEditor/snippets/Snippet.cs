using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Snippets
{
    public class ID
    {
        public string Name { get; set; }
        public string Value { get; set; }
    }

    public class Snippet
    {
        public string Shortcut { get; set; }
        public string Code { get; set; }
        public List<ID> Declaration { get; set; }
    }
}
