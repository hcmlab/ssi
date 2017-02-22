using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace SSIXmlEditor
{
    class Matcher : IFilter
    {
        private Regex m_XmlOpenTagRegex = new Regex(@"<(?<tag>\w+)\s*", RegexOptions.Compiled);
        private Regex m_XmlCloseTagRegex = new Regex(@"</(?<tag>\w+)>", RegexOptions.Compiled);
		
        public bool Matches(string value)
        {
            return m_XmlOpenTagRegex.IsMatch(value) == true ? true : m_XmlCloseTagRegex.IsMatch(value);
        }

        public void Process(int line)
        {

        }

        public string getTagName(string value)
        {
            if (m_XmlOpenTagRegex.IsMatch(value))
                return m_XmlOpenTagRegex.Match(value).Groups["tag"].Value;
            else
                return m_XmlCloseTagRegex.Match(value).Groups["tag"].Value;
        }
    }
}
