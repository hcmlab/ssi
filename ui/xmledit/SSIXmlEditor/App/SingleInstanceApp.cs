using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using Microsoft.Shell;
using SSIXmlEditor.Document;
using System.IO;

namespace SSIXmlEditor.App
{
    public class SingleInstanceApp : Application, ISingleInstanceApp, PluginSystem.IPluginHost
    {
        public event EventHandler ActiveDocumentChanged;

        private Dictionary<Type, View.IView> m_Views;
        private Dictionary<Type, Model.IModel> m_Models;
        private Dictionary<string, Command.ICommand> m_Commands;
        private ADocument m_ActiveDocument;
		
  
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

        public SingleInstanceApp()
        {
            m_Views = new Dictionary<Type, View.IView>();
            m_Models = new Dictionary<Type, Model.IModel>();
            m_Commands = new Dictionary<string, Command.ICommand>();
        }

        public void Init(Command.ICommand startUpCommand)
        {
            ReceiveExternalClArgs += (app, args) =>
            {
                Documents.OpenAndActivate(args.Args.ToArray());
            };

            if (null == startUpCommand)
                throw new ArgumentNullException("startUpCommand");

			//new System.Threading.Thread(new System.Threading.ThreadStart(() => startUpCommand.Execute())).Start();
            startUpCommand.Execute();

            if (!File.Exists("./xmledit.xml"))
            {
                System.Windows.Forms.MessageBox.Show("Init file 'xmledit.xml' is missing.", "ERROR", System.Windows.Forms.MessageBoxButtons.OK, System.Windows.Forms.MessageBoxIcon.Error);
                System.Environment.Exit(1);
            }

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

        public bool SignalExternalCommandLineArgs(IList<string> args)
        {
            Documents.OpenAndActivate(args.ToArray());
            return true;
        }

        public delegate void ClArgsEventHandler(SingleInstanceApp app, ExternalClArgs e);

        public event ClArgsEventHandler ReceiveExternalClArgs;
        public void OnReceiveExternalClArgs(IList<string> clargs)
        {
            var handler = ReceiveExternalClArgs;
            if (handler != null) handler(this, new ExternalClArgs() { Args = clargs });
        }
    }

    public class ExternalClArgs : EventArgs
    {
        public IList<string> Args { get; set; }
    }
}
