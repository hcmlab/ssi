using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace SSIXmlEditor.Snippets
{
	public class SnippetManager
	{
		private static SnippetManager m_Instance = null;

        public ISnippetService Service { set; private get; }

		private Dictionary<string, string> m_LookUp;
        private Dictionary<string, Snippet> m_Snippets;
		
		public static SnippetManager Instance 
		{
			get
			{
				if(null == m_Instance)
					m_Instance = new SnippetManager();
					
				return m_Instance;
			}
		}

        private SnippetManager()
        {
            m_Snippets = new Dictionary<string, Snippet>();
        }

        public void Load(string path)
        {
            var snippet = Service.Load(path);
            string shortCut = snippet.Shortcut;

            if (String.IsNullOrEmpty(shortCut))
                shortCut = System.IO.Path.GetFileNameWithoutExtension(path);

            m_Snippets.Add(shortCut, snippet);
        }

        public bool ContainsSnippet(string name)
        {
            return m_Snippets.ContainsKey(name);
        }

        public Snippet GetSnippet(string name)
        {
            return m_Snippets[name];
        }

        public bool GetSnippet(string value, out Snippet snippet)
        {
            if (m_Snippets.ContainsKey(value))
            {
                snippet = m_Snippets[value];
                //snippet.Load();
                return true;
            }

            snippet = null;
            return false;
        }
		
		public void Initialize()
		{
			m_LookUp = new Dictionary<string,string>();
			
			foreach(var file in System.IO.Directory.GetFiles("./snippets/", "*.xml"))
			{
				var fi = new System.IO.FileInfo(file);
				
				m_LookUp.Add(fi.Name.Substring(0, fi.Name.LastIndexOf(".")), fi.FullName);
			}

            m_Snippets = new Dictionary<string, Snippet>();
            foreach (var file in System.IO.Directory.GetFiles("./snippets/", "*.snippet"))
            {
                //m_Snippets.Add(System.IO.Path.GetFileNameWithoutExtension(file), new Snippet(new Snippets.Service.XmlSnippetService(file)));
            }
		}
		
		public String ExtractSnippet(string name)
		{
            try
            {
                var reader = new System.Xml.XmlTextReader(m_LookUp[name]);
                reader.WhitespaceHandling = WhitespaceHandling.None;
                string code = "";
                while (reader.Read())
                {
                    if (!reader.Name.Equals("Code"))
                        continue;

                    reader.Read();
                    code = reader.Value;
                    reader.Close();

                    break;
                }
                
                return code;
            }
            catch (Exception ex)
            {
                
            }

            return String.Empty;
		}
		
		public String ExtractSnippet(string name, string replace)
		{
			string code = ExtractSnippet(name);
			return code.Replace("$name$", replace);			
		}
	
	}
}
