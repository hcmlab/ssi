using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Pipeline
{
    interface IPipelineExecuter
    {
        Document.IDocument Document { get; set; }

        void Execute();
    }
}
