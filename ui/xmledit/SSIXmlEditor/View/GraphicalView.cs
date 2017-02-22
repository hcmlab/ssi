using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using SSIXmlEditor.App;
using SSIXmlEditor.Extension;
using SSIXmlEditor.Controls.GraphicsView;
using SSIXmlEditor.Document;
using SSIXmlEditor.Model;
using SSIXmlEditor.Repository;

namespace SSIXmlEditor.View
{
	public partial class GraphicalView : Form
	{
		private Matcher m_Filter;
		private Repository.IModelRepository m_ModelRepository;
	    private App.SingleInstanceApp m_App;

		public GraphicalView(SingleInstanceApp app, IModelRepository modelRepository)
		{
			InitializeComponent();
		    m_App = app;
			m_Filter = new Matcher();
			m_ModelRepository = modelRepository;
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

            var activeDoc = m_App.ActiveDocument;
			AnalyseDocument(activeDoc);
		}

		private void AnalyseDocument(ADocument document)
		{
			for(int i = 1; i <= document.InputHandler.Lines; ++i)
			{
				var skip = AnalyseLine(i, document.InputHandler);
				i += skip;
			}
			
			graphicaView1.PerformLinking();
		}
		
		private int AnalyseLine(int line, IInputHandler inputHandler)
		{
			string text = inputHandler.GetTextByLine(line);
			
			if (m_Filter.Matches(text))
			{
				var tag = m_Filter.getTagName(text);
				if(string.IsNullOrEmpty(tag))
					return 0;
					
				tag = tag.ToLower();
				
				if(tag.Equals("sensor") || tag.Equals("transformer") || tag.Equals("consumer"))
				{
					inputHandler.CurrentLine = line;
					Model.BaseObject item = null;
			
					try
					{
						item = m_ModelRepository.getModel(tag) as Model.BaseObject;
					}catch(Exception) {}

					int iSkip = 0;
					string tmpLine = String.Empty;
					if (null == item)
					{
						
						do
						{
							tmpLine = inputHandler.GetTextByLine(line + iSkip);
							iSkip++;
						} while (!tmpLine.Contains("</" + tag));
						
						return iSkip - 1;
					}

					item.SyncModel(m_ModelRepository, inputHandler, line);
					if(tag.Equals("sensor"))
					{
						var comp = new SensorComponent();
						comp.Bounds = new Rectangle(10, 10, 100, 100);
						var providers = item["Provider"] as System.Collections.ICollection;
						foreach (BaseObject provider in providers)
						{
							comp.AddOutputComponent(new OutputComponent() { Text = provider["pin"].ToString() });
						}
						
						graphicaView1.AddComponent(comp);
					}
					
					if(tag.Equals("transformer"))
					{
						var transformerComp = new TransformerComponent();
						transformerComp.Bounds = new Rectangle(200, 10, 100, 100);
						
						var input = item["Input"] as Model.BaseObject;
						if(null != input)
							transformerComp.AddInputComponent(new InputComponent() { Text = input["Pin"].ToString()});
						
						var output = item["Output"] as Model.BaseObject;						
						if(null != output)
							transformerComp.AddOutputComponent(new OutputComponent() { Text = output["Pin"].ToString() });
						
						graphicaView1.AddComponent(transformerComp);
					}
					
					if(tag.Equals("consumer"))
					{
						var consumerComp = new ConsumerComponent();
						consumerComp.Bounds = new Rectangle(400, 10, 100, 100);
						
						var input = item["Input"] as Model.BaseObject;
						consumerComp.AddInputComponent(new InputComponent() { Text = input["Pin"].ToString() });						
						
						graphicaView1.AddComponent(consumerComp);
					}
					
					do
					{
						tmpLine = inputHandler.GetTextByLine(line + iSkip);
						iSkip++;
					}while(!tmpLine.Contains("</" + tag));
					
					return iSkip-1;
				}	
			}
			
			return 0;
		}
	}
}
