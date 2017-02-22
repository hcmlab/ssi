using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Xml;

namespace ssi
{    
    public enum StimuliSourceType
    {
        Blank = 0,
        Text,
        URL,
        Code
    }

    public abstract class IStimuliSource
    {        
        public const int MAX_INFO_CHAR = 50;

        public string Info 
        {
            get {
                string s = getInfo();
                if (s.Length > IStimuliSource.MAX_INFO_CHAR)
                {
                    return s.Substring(0, Math.Min(s.Length, IStimuliSource.MAX_INFO_CHAR)) + "...";
                }
                else
                {
                    return s;
                }                
            }
        }
        public StimuliSourceType Type
        {
            get { return getType(); }
        }

        public abstract void show(WebBrowser browser);
        public abstract void save(XmlWriter xml);
        protected abstract string getInfo();
        protected abstract StimuliSourceType getType();
    }

    public class StimuliSourceBlank : StimuliSourceCode
    {
        public StimuliSourceBlank()
            : base("<html><body></body></html>")
        {
        }

        protected override string getInfo()
        {
            return "";
        }

        protected override StimuliSourceType getType()
        {
            return StimuliSourceType.Blank;
        }

        public override void save(XmlWriter xml)
        {
            base.save(xml);
        }

        public static StimuliSourceBlank Load(XmlReader xml)
        {
            return StimuliSourceBlank.Load(xml);
        }
    }

    public class StimuliSourceText : StimuliSourceCode
    {
        string text;

        public StimuliSourceText(string text, string color, string size)
            : base("<html>\n  <head>    \n<meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\" />\n</head>\n<body>\n<table width=\"100%\" height=\"100%\">\n  <tr><td align=\"center\">\n    <p><font color=\"#" + color + "\" size=\"" + size + "\">" + text + "</font></p>\n   </td></tr>\n</table>\n</body>")
        {
            this.text = text;
        }

        protected override string getInfo()
        {
            return text;
        }

        protected override StimuliSourceType getType()
        {
            return StimuliSourceType.Text;
        }

        public override void save(XmlWriter xml)
        {
            xml.WriteStartAttribute("text");
            xml.WriteString(text.ToString());
            xml.WriteEndAttribute();
        }

        public static StimuliSourceText Load(XmlReader xml)
        {
            string text = xml.GetAttribute("text");
            string color = xml.GetAttribute("color");
            if (color == null)
            {
                color = "000000";
            }
            string size = xml.GetAttribute("size");
            if (size == null)
            {
                size = "7";
            }
            return new StimuliSourceText(text,color, size);
        }
    }

    public class StimuliSourceURL : IStimuliSource
    {
        Uri url;

        public StimuliSourceURL(Uri url)
        {
            this.url = url;
        }

        public override void show(WebBrowser browser)
        {
            browser.Navigate(url);
        }

        protected override string getInfo()
        {
            return url.ToString();            
        }

        protected override StimuliSourceType getType()
        {
            return StimuliSourceType.URL;
        }

        public override void save(XmlWriter xml)
        {
            xml.WriteStartAttribute("url");
            xml.WriteString(url.ToString());
            xml.WriteEndAttribute();
        }

        public static StimuliSourceURL Load(XmlReader xml, RecordHandler handler)
        {
            string u = xml.GetAttribute("url");

            Uri url = new Uri(u, UriKind.RelativeOrAbsolute);

            if (!url.IsAbsoluteUri) //we need absolute uris for the browser
                url = new Uri(handler.project.StimuliDir + "\\" + u);

            return new StimuliSourceURL(url);
        }
    }

    public class StimuliSourceCode : IStimuliSource
    {
        string code;

        public StimuliSourceCode(string code)
        {
            this.code = code;
        }

        public override void show(WebBrowser browser)
        {
            browser.NavigateToString(code);
        }

        protected override string getInfo()
        {
            return code.Substring(0, Math.Min(code.Length, IStimuliSource.MAX_INFO_CHAR)); ;
        }

        protected override StimuliSourceType getType()
        {
            return StimuliSourceType.Code;
        }

        public override void save(XmlWriter xml)
        {
            xml.WriteStartAttribute("code");
            xml.WriteString(code.ToString());
            xml.WriteEndAttribute();
        }

        public static StimuliSourceCode Load(XmlReader xml)
        {
            string code = xml.GetAttribute("code");
            return new StimuliSourceCode(code);
        }
    }
}
