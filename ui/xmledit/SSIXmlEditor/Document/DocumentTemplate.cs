using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace SSIXmlEditor.Document
{
    class DocumentTemplate : IDocumentTemplate
    {
        public string getTemplate()
        {
            XDocument doc = new XDocument(
                new XDeclaration("1.0", "utf-8", "yes"),
                new XElement("pipeline",
                new XElement("register")));

            var sb = new StringBuilder();
            using (var stream = new System.IO.StringWriter(sb))
            {
                doc.Save(stream);
                stream.Close();
            }

            return sb.ToString();
        }
    }
}
