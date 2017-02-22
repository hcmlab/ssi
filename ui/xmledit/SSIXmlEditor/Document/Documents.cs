using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.App;
using System.IO;

namespace SSIXmlEditor.Document
{
    public class DocumentsEventArgs : EventArgs
    {
        public bool Added { get; private set; }
        public ADocument Document { get; private set; }

        public DocumentsEventArgs(bool bAdded, ADocument document)
        {
            Added = bAdded;
            Document = document;
        }
    }

    public class Documents : Model.IModel, IEnumerable<ADocument>
    {
        public event EventHandler<DocumentsEventArgs> Added;
        public event EventHandler<DocumentsEventArgs> Removed;

        private List<ADocument> m_Items;

        public SingleInstanceApp Application { get; private set; }

        public Document.IDocumentTemplate DocumentTemplate { set; private get; }

        internal Documents(SingleInstanceApp app)
        {
            if(null == app)
                throw new ArgumentNullException("app");

            Application = app;
            m_Items = new List<ADocument>();
        }

        private void AddDocument(ADocument doc)
        {
            m_Items.Add(doc);

            if (null != Added)
                Added(this, new DocumentsEventArgs(true, doc));
        }

        public ADocument Create(string name)
        {
            if (string.IsNullOrEmpty("name"))
                throw new ArgumentNullException("name");

            var doc = new ADocument(name, Application);
            AddDocument(doc);

            doc.Text = DocumentTemplate.getTemplate();
            return doc;
        }

        public void Close(ADocument doc)
        {
            m_Items.Remove(doc);

            if (null != Removed)
                Removed(this, new DocumentsEventArgs(false, doc));
        }

        public ADocument Open(string file)
        {
            if (string.IsNullOrEmpty(file))
                throw new ArgumentNullException("file");

            //string text = null;
            //using (var stream = System.IO.File.OpenText(file))
            //{
            //    text = stream.ReadToEnd();
            //    stream.Close();
            //}
            
            var result = m_Items.Where(dc => dc.File != null && dc.File.FullName.Equals(file));
			if(result.Count() > 0)
				return result.Single();

            var doc = new ADocument(new FileInfo(file), Application);
            AddDocument(doc);
            
            //doc.Text = text;            
            return doc;
        }

        #region IEnumerable<ADocument> Members

        public IEnumerator<ADocument> GetEnumerator()
        {
            return m_Items.GetEnumerator();
        }

        #endregion

        #region IEnumerable Members

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return m_Items.GetEnumerator();
        }

        #endregion

        public void OpenAndActivate(string[] fileList)
        {
            ADocument lastDocument = null;
            foreach (var file in fileList)
            {
                var extension = Path.GetExtension(file);
                if (extension == null || extension.ToLower() != ".pipeline")
                {
                 continue;
                }

                lastDocument = Open(file);
            }

            if (lastDocument != null)
            {
                lastDocument.Activate();
            }
        }

        public void OpenAndActivate(string filepath)
        {
            OpenAndActivate(new []{filepath});
        }
    }
}
