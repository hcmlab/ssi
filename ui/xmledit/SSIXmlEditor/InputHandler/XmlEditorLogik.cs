using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
//using SSIXmlEditor.Visitor;
using SSIXmlEditor.Extension;
using SSIXmlEditor.Model;
using SSIXmlEditor.View;

namespace SSIXmlEditor.InputHandler
{
    class XmlEditorLogic
    {
        private Model.IInputHandler m_InputHandler;
        private InputHandler.KeyWordDetector m_KeyWordDetector;
        private Repository.IModelRepository m_ModelRepository;
        private Matcher m_Filter = new Matcher();
        private Timer m_Timer;

        public Model.IInputHandler InputHandler
        {
            private get { return m_InputHandler; }
            set 
            {
                if (m_InputHandler != null)
                {
                    m_InputHandler.CaretChanged -= m_InputHandler_CaretChanged;
                    m_InputHandler.TextChanged -= m_InputHandler_TextChanged;
					m_InputHandler.RightMouseClick -= m_InputHandler_RightMouseClick;
                }

                m_InputHandler = value;
                m_InputHandler.CaretChanged += m_InputHandler_CaretChanged;
                m_InputHandler.TextChanged += m_InputHandler_TextChanged;
				m_InputHandler.RightMouseClick += m_InputHandler_RightMouseClick;
                                
                m_KeyWordDetector.InputHandler = value; 
                m_ModelRepository.InputHandler = m_InputHandler;
				
				m_LastLine = -1;
				m_Model.Select(null);
            }
        }

		void m_InputHandler_RightMouseClick(object sender, EventArgs e)
		{
			//TODO: create class handling contextmenus
			string line = InputHandler.GetCurrentLineText();

			string tag = m_Filter.getTagName(line);
			if (string.IsNullOrEmpty(tag))
				return;

			var model = m_ModelRepository.getModel(tag);

			//if (!m_LookUp.ContainsKey(tag))
			//    return;
				
			
			//var model = m_LookUp[tag];
			//var entries = new List<KeyValuePair<string, Action>>();
			//var linePos = InputHandler.GetEndOffsetOfLine(InputHandler.CurrentLine);


			//if (model is SensorModel)
			//{
			//    foreach (ProviderModel provider in (model as SensorModel).Provider)
			//    {
			//        entries.Add(new KeyValuePair<string, Action>(String.Format("add provider({0})", provider.Channel),
			//                    () => InputHandler.Insert(linePos,
			//                        String.Format(Environment.NewLine + "<provider channel=\"{0}\" pin=\"\" event=\"\"/>", provider.Channel))));
			//    }
			//}

			//if (entries.Count > 0)
			//    InputHandler.CreateAndShowContextMenu(entries);
		}

        private Model.SSIModel m_Model;

        public XmlEditorLogic(Model.SSIModel model, Repository.IModelRepository modelRepository)
        {
            m_Model = model;
            m_KeyWordDetector = new KeyWordDetector(new Action<string>(KeyWordFilterAction));
			m_ModelRepository = modelRepository;
			m_Model.ModelChanged += new EventHandler<SSIModelChangedEventArgs>(m_Model_ModelChanged);

            m_Timer = new Timer(new TimerCallback(Update), null, 1000, 200);
        }

        void m_Model_ModelChanged(object sender, SSIModelChangedEventArgs e)
        {
			var item = e.Model as Model.BaseObject;
			if(null == item)
			{
				System.Diagnostics.Debug.WriteLine("Not of expected type. " + e.Model.GetType().ToString());
				return;
			}

			if(e.Category == "Channels")
			{
				
				var currentLine = item.Line + 1;
				bool bInserted = false;
				
				do
				{
					var line = m_InputHandler.GetTextByLine(currentLine);
					
					if(line.Contains("</sensor"))
						break;
					if(line.Contains(string.Format("channel=\"{0}\"", e.Property)) || line.Contains("channel=\"\""))
					{
						var providers = item["Provider"] as System.Collections.ICollection;
						foreach(BaseObject provider in providers)
						{
							if(String.IsNullOrEmpty(provider["channel"].ToString()))
							{
								provider["channel"] = e.Property;
								provider.ChangeModel(m_InputHandler, "channel", "");
							}
								
							if(!provider["channel"].Equals(e.Property))
								continue;
								
							provider["pin"] = e.NewValue.ToString();
							provider.ChangeModel(m_InputHandler, "pin", e.OldValue == null ? "" : e.OldValue.ToString());
							bInserted = true;
							break;
						}
											
						break;
					}
					
					currentLine++;
				}while(currentLine < m_InputHandler.Lines);
				
				if(!bInserted)
				{
					m_InputHandler.Insert(m_InputHandler.GetOffsetOfLine(item.Line + 1), 
						string.Format("<provider channel=\"{0}\" pin=\"{1}\"/>" + Environment.NewLine, e.Property, e.NewValue.ToString()));
					
				}
				
			}
			else
				item.ChangeModel(m_InputHandler, e.Property, e.OldValue == null ? "" : e.OldValue.ToString());			
        }

		private volatile int m_LastLine = 0;
		private volatile bool m_bAnalyze = false;
        void m_InputHandler_CaretChanged(object sender, EventArgs e)
		{
			if(m_InputHandler == null || m_bAnalyze)
				return;
			
			if(m_LastLine == m_InputHandler.CurrentLine)
				return;
				
			m_bChanged = true;
			m_LastLine = m_InputHandler.CurrentLine;
        }

        private bool m_bChanged = false;
        private DateTime m_LastChanged;
        void m_InputHandler_TextChanged(object sender, EventArgs e)
        {
			m_bChanged = true;
            m_LastChanged = DateTime.Now;
        }

		private object m_Lock = new object();
		
        private void Update(object param)
        {
            if (null == InputHandler)
                return;

            if (!m_bChanged)
                return;
           
			m_bChanged = false;

			Monitor.Enter(m_Lock);
			{
				try
				{
					m_bAnalyze = true;
					Analyze(InputHandler.CurrentLine);
				}
				catch (Exception ex)
				{
					System.Diagnostics.Debug.WriteLine(DateTime.Now + ": " + ex.Message);
				}
		
				m_bAnalyze = false;
				Monitor.Exit(m_Lock);
			}	
        }
        
        private void KeyWordFilterAction(string value)
        {
        }

        public void Analyze(int line)
        {   
            string text = m_InputHandler.GetTextByLine(line);

            if (m_Filter.Matches(text))
            {
                var tag = m_Filter.getTagName(text);
                if (!String.IsNullOrEmpty(tag))
                {
					var item = m_ModelRepository.getModel(tag) as Model.BaseObject;
					if(null == item)
					{
						m_Model.Select(null);
						return;
					}
					
					if(!item.Selectable)
					{
						//Analyze(line - 1);
						m_Model.Select(null);
						return;
					}
					
                    System.Diagnostics.Debug.WriteLine(DateTime.Now + ": " + tag + " found");
                        
					item.SyncModel(m_ModelRepository, m_InputHandler, line);
                    
                    m_Model.Select(item);
                }
            }
			else
				m_Model.Select(null);
        }
    }
}
