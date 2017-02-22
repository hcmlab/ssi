using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace SSIXmlEditor
{
	public static class Tools
	{
		public static string ObjectTypeToTag(ObjectType type)
		{
			switch(type)
			{				
				case ObjectType.SSI_CONSUMER: return "consumer";
				case ObjectType.SSI_TRANSFORMER:
				case ObjectType.SSI_FEATURE:
				case ObjectType.SSI_FILTER: return "transformer";
				case ObjectType.SSI_SENSOR: return "sensor";                                                
				default: return "object";				
			}
		}

        public static void PerformInvoke(System.Windows.Forms.Control ctrl, Action act)
        {
            if (ctrl.InvokeRequired)
                ctrl.Invoke(act);
            else
                act();
        }

        public static class Xml
        {
            private static Regex m_XmlOpenTagRegex = new Regex(@"</*(?<tag>\w+)\s*", RegexOptions.Compiled);
            
            public static string GetTag(string text)
            {
                return m_XmlOpenTagRegex.Match(text).Groups["tag"].Value;
            }
        }
	}
}
