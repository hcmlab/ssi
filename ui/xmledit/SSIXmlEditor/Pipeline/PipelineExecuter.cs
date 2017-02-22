using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Pipeline
{
    class PipelineExecuter : IPipelineExecuter
    {
        public Document.IDocument Document { get; set; }

        public PipelineExecuter(Document.IDocument document)
        {
            if (null == document)
                throw new ArgumentNullException("document");

            Document = document;
        }

        #region IPipelineExecuter Members

        public void Execute()
        {
            Document.Save();

            var process = new System.Diagnostics.Process();
            process.StartInfo.FileName = Properties.Settings.Default.PipelineApp;
            process.StartInfo.Arguments = string.Format("\"{0}\"", Document.File.FullName);

            process.Start();
        }

        #endregion
    }

    class DefinesPipelineExecuter : IPipelineExecuter
    {
        public Document.IDocument Document { get; set; }
        private IPipelineExecuter Parent { get; set; }

        public DefinesPipelineExecuter(Document.IDocument document, IPipelineExecuter executer)
        {
            if (null == executer)
                throw new ArgumentNullException("executor");

            if (null == document)
                throw new ArgumentNullException("document");

            Parent = executer;
            Document = document;
        }

        #region IPipelineExecuter Members

        public void Execute()
        {
            //search for defines
            //replace with value
            //create temp file and execute templ file
            var text = Document.DocumentText;

            if (text.Contains("<defines>"))
            {
                Dictionary<string, string> defines = new Dictionary<string, string>();

                String name = String.Empty;
                String value = String.Empty;

                byte[] data = System.Text.Encoding.ASCII.GetBytes(text);
                var ms = new System.IO.MemoryStream(data);
                var xmlReader =System.Xml.XmlTextReader.Create(ms);
                xmlReader.ReadToFollowing("defines");
                var subReader = xmlReader.ReadSubtree();
                subReader.Read();

                if (subReader.ReadToFollowing("item") && subReader.AttributeCount == 2)
                {
                    subReader.MoveToAttribute("name");
                    name = subReader.Value;

                    subReader.MoveToAttribute("value");
                    value = subReader.Value;

                    if (defines.ContainsKey(name))
                        throw new Exception("Name muss eindeutig sein");

                    defines.Add(name, value);
                }

                if (defines.Count > 0)
                {
                    foreach (var item in defines)
                        text = text.Replace(String.Format("${0}$", item.Key), item.Value);
                }

                //remove defines
                int startPos = text.IndexOf("<defines>");
                int endPos = text.IndexOf("</defines>") + "</defines>".Length;

                text = text.Remove(startPos, endPos - startPos);
            }

            var doc = new Document.TempDocument(text);
            doc.Save();

            Parent.Document = doc;
            Parent.Execute();
        }

        #endregion

        
    }
}
