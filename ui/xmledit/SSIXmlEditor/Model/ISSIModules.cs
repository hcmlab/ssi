using System;
using System.Collections.Generic;
namespace SSIXmlEditor.Model
{
    public interface ISSIModules : Model.IModel 
    {
        event EventHandler Loaded;
        event EventHandler SelectionChanged;
        
        Service.IMetaDataService Service { set; }
        MetaData Selected { get; }
        IEnumerable<MetaData> Modules { get; }
        
        void Select(string module);
        void Load();

        MetaData GetMetaData(string value);
    }
}
