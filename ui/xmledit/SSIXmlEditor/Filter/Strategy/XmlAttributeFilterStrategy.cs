using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace SSIXmlEditor.Filter.Strategy
{
    public class XmlAttributeFilterStrategy : IFilterStrategy
    {   
        private Regex m_CheckRegex;
        private Regex m_AttributeRegex;

        public XmlAttributeFilterStrategy(string attribute)
        {
            if (string.IsNullOrEmpty(attribute))
                throw new ArgumentNullException("attribute");

            var pattern = String.Format(@"<\w+\s+{0}=", attribute);
            var patternAtt = String.Format("<\\w+\\s+{0}=\"(?<attribute>\\w+)\"", attribute);
            m_CheckRegex = new Regex(pattern, RegexOptions.Compiled);
            m_AttributeRegex = new Regex(patternAtt, RegexOptions.Compiled);
        }

        #region IFilterStrategy Members

        public bool Matches(string text, out string value)
        {
            if (m_CheckRegex.IsMatch(text))
            {
                value = m_AttributeRegex.Match(text).Groups["attribute"].Value;
                if (string.IsNullOrEmpty(value))
                    return false;

                return true;
            }

            value = String.Empty;
            return false;
        }

        #endregion
    }
}
