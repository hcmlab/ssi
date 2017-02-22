using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Filter
{
    public interface IFilterStrategy
    {
        bool Matches(string text, out string value);
    }
}
