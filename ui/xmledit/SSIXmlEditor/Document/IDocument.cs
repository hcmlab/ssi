using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Document
{
	public interface IDocument : IDisposable
	{
        System.IO.FileInfo File { get; }
        String DocumentText { get; }

        void Save();
        void SaveAs();
	}
}
