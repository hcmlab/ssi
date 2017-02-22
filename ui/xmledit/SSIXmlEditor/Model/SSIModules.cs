using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Model
{
    
    public class SSIModules : ISSIModules 
    {
        public Service.IMetaDataService Service { set; private get; }

        #region ISSIModules Members

        public event EventHandler SelectionChanged;
        public event EventHandler Loaded;

        private IDictionary<string, MetaData> m_Modules;
        public MetaData Selected { get; private set; }
        public IEnumerable<MetaData> Modules { get { return m_Modules.Values; } }
        
        private void Add(MetaData module)
        {
            m_Modules.Add(module.Name, module);
        }

        public void Select(string module)
        {
            Selected = m_Modules[module];
            FireEvent();
        }

        public MetaData GetMetaData(string value)
        {
            if(m_Modules.ContainsKey(value))
                return m_Modules[value];

            return null;
        }

        #endregion
        
        public SSIModules()
        {
            m_Modules = new Dictionary<string, MetaData>();
        }

        public void Load()
        {
            if (null == Service)
                throw new ArgumentNullException("Service");

            Service.Load();
            foreach (var item in Service.MetaDatas)
                Add(item);

            if (null != Loaded)
                Loaded(this, EventArgs.Empty);
        }

        private void FireEvent()
        {
            if (null != SelectionChanged)
                SelectionChanged(this, EventArgs.Empty);
        }
    }
}
