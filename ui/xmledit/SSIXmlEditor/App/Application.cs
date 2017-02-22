using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.Document;

namespace SSIXmlEditor.App
{
    public class Application : PluginSystem.IPluginHost
    {
        public event EventHandler ActiveDocumentChanged;

        private static Application m_Instance = null;
        private Dictionary<Type, View.IView> m_Views;
        private Dictionary<Type, Model.IModel> m_Models;
        private Dictionary<string, Command.ICommand> m_Commands;
        private Document.ADocument m_ActiveDocument;
		
        public static Application Instance
        {
            get { return m_Instance ?? (m_Instance = new Application()); }
        }
        
        
		public Repository.IModelRepository ModelRepository { get; private set; }
        public View.IMainView MainView { get; set; }
        public View.IDocumentsView DocumentsView { get; set; }

        public Documents Documents { get; internal set; }
        public Snippets.SnippetManager SnippetManager { get; set; }
        
        public string DllPath { get; set; }
        public string PipelineApp { get; set; }
        public List<string> DllNames { get; set; }
        
        public ADocument ActiveDocument 
        {
            get { return m_ActiveDocument; }
            set
            {
                m_ActiveDocument = value;
				
                if (null != ActiveDocumentChanged)
                    ActiveDocumentChanged(this, EventArgs.Empty);
            }
        }

        private Application()
        {
            m_Views = new Dictionary<Type, SSIXmlEditor.View.IView>();
            m_Models = new Dictionary<Type, SSIXmlEditor.Model.IModel>();
            m_Commands = new Dictionary<string, SSIXmlEditor.Command.ICommand>();
        }

        public void Init(Command.ICommand startUpCommand)
        {
            if (null == startUpCommand)
                throw new ArgumentNullException("startUpCommand");

			//new System.Threading.Thread(new System.Threading.ThreadStart(() => startUpCommand.Execute())).Start();
            startUpCommand.Execute();
			ModelRepository = new Repository.ModelRepository("./xmledit.xml", this.GetModel<Model.SSIModel>());
        }

        public void RegisterView<T>(View.IView view) where T : View.IView 
        {
            if (null == view)
                throw new ArgumentNullException("view");

            m_Views.Add(typeof(T), view);
        }

        public void RegisterModel<T>(Model.IModel model) where T : Model.IModel
        {
            if (null == model)
                throw new ArgumentNullException("model");

            m_Models.Add(typeof(T), model);
        }

        public void RegisterCommand(string name, Command.ICommand cmd)
        {
            if(string.IsNullOrEmpty(name))
                throw new ArgumentNullException("name");
            if (null == cmd)
                throw new ArgumentNullException("cmd");

            m_Commands.Add(name, cmd);
        }

        public void InvokeCommand(string name)
        {
            m_Commands[name].Execute();
        }

        public T GetView<T>() where T : View.IView 
        {
            return (T)m_Views[typeof(T)];
        }

        public T GetModel<T>() where T : Model.IModel
        {
            return (T)m_Models[typeof(T)];
        }
    }
}
