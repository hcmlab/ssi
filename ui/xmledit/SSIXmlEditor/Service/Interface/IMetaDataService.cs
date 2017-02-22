using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Service
{
    public interface IMetaDataService
    {
        IEnumerable<MetaData> MetaDatas { get; }
        void Load();
    }
}
