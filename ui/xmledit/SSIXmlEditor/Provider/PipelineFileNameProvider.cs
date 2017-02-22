using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Provider
{
    class PipelineFileNameCounterProvider : IFileNameProvider
    {
        private const string m_FileExt = ".pipeline";
        private string m_DefFileName;
        private int m_Index = 1;


        public PipelineFileNameCounterProvider(string defFileName)
        {
            m_DefFileName = defFileName;

            if (string.IsNullOrEmpty(defFileName))
                m_DefFileName = "new";
            
        }

        public string CreateName()
        {
            return String.Format("{0}{1}{2}", m_DefFileName, m_Index++, m_FileExt);
        }
    }
}
