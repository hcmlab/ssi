using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Document
{
    class TempDocument : IDocument
    {
        public TempDocument(string value)
        {
            DocumentText = value;
        }

        #region IDocument Members

        public System.IO.FileInfo File { get; private set; }

        public string DocumentText { get; private set; }

        public void Save()
        {
            var fileName = System.IO.Path.GetTempFileName() + ".pipeline";
            File = new System.IO.FileInfo(fileName);
            using (var stream = new System.IO.StreamWriter(fileName))
            {
                stream.Write(DocumentText);
            }           
        }

        public void SaveAs()
        {
            throw new NotImplementedException();
        }

        #endregion

        #region IDisposable Members

        public void Dispose()
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
