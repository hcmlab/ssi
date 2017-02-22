using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace SSIXmlEditor.Service
{
	[Serializable()]
	class MetaDataCache
	{
		public DateTime Time;
		public XMLReader.MetaData Data;

		public override string ToString()
		{
			return Data.Lib;
		}
	}

    class MetaDataService : IMetaDataService
    {
        private List<string> m_DllNames { get; set; }
        private List<MetaData> m_MetaDatas;

        public MetaDataService(List<string> DllNames)
        {
            m_DllNames = DllNames;
        }

        #region IMetaDataService Members

        public IEnumerable<MetaData> MetaDatas
        {
            get { return m_MetaDatas; }
        }

        public void Load()
        {
            if (MetaDatas == null)
                m_MetaDatas = new List<MetaData>();
            else 
				m_MetaDatas.Clear();
            
            var reader = new XMLReader.SSIReader();

            List<XMLReader.MetaData> metaData = new List<XMLReader.MetaData>();
            var all = new Dictionary<String, String>();

            //foreach (var fi in getDlls())
            foreach (var fi in m_DllNames)
            {
                try
                {
					System.Diagnostics.Debug.WriteLine(String.Format("Loading {0}", fi));
                    List<string> objects = reader.ReadDll(fi);
                    foreach (string s in objects) {
                        if (!all.ContainsKey(s))
                        {
                            all.Add(s, fi);
                        }
                    }
                }
                catch (Exception e)
                {
                    System.Diagnostics.Debug.WriteLine(String.Format("ERROR {0}", e.ToString ()));
                }
            }

            foreach (var item in all.Keys)
            {
				XMLReader.MetaData md = null;
				
				try
				{
					System.Diagnostics.Debug.WriteLine(String.Format("Reading metadata of {0}", item));
					md = reader.ReadMetaData(item);
				}
				catch(Exception e)
				{
                    System.Diagnostics.Debug.WriteLine(String.Format("ERROR {0}", e.ToString()));
				}
                
                if(md != null)
                {
					md.Lib = all[item];
					metaData.Add(md);
				}
            }

			var cacheObjs = new List<MetaDataCache>();
			var cache = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();
			
			foreach(var md in m_MetaDatas)
				cacheObjs.Add(new MetaDataCache() { Data = md.Reference, Time = new FileInfo(md.Lib).CreationTime });
			
            metaData.ForEach(md => 
				{
					m_MetaDatas.Add(new MetaData(md));
					cacheObjs.Add(new MetaDataCache() { Data = md, Time = new FileInfo(all[md.Name]).CreationTime });
				});
				
        }

        #endregion
        
  
    }
}
