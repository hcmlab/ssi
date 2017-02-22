using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Serialization;

namespace SSIXmlEditor.Snippets.Service
{
    public class XmlSnippetService : ISnippetService
    {
        public XmlSnippetService()
        {
            
        }

        #region ISnippetService Members

        public Snippet Load(string snippetPath)
        {
            var stream = new System.IO.StreamReader(snippetPath);
            var xmlSerializer = new XmlSerializer(typeof(Snippet));
            var obj = xmlSerializer.Deserialize(stream);
            stream.Close();

            return obj as Snippet;            
        }

        
        #endregion
    }
}
