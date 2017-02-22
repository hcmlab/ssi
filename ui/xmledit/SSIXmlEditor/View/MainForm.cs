using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.Integration;
using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.Highlighting;
using SSIXmlEditor.Controller;
using SSIXmlEditor.Controls;
using SSIXmlEditor.Document;
using SSIXmlEditor.View;

namespace SSIXmlEditor
{
    public partial class MainForm : Form, View.IMainView, View.ISSIModulesView, View.ISSIModelView
    {
        private App.SingleInstanceApp m_Application;
        private DocumentControl m_DocumentsView;
        private Controller.MainController m_Controller;
        private Controller.SSIModulesController m_SSIModulesController;
        private Controller.SSIModelController m_SSIModuleController;
        private Dictionary<string, ToolStripItem> m_MenuItems;
                
        #region IMainView Members

        public string Caption
        {
            set { Text = value; }
        }

		public void RegisterCommand(string cmd, bool active)
		{
			m_MenuItems[cmd].Enabled = active;
		}
		
		public void ActivateCommand(string cmd, bool activate)
		{
			m_MenuItems[cmd].Enabled = activate;
		}

        #endregion

        private void PerformInvoke(Control ctrl, Action act)
        {
            if (ctrl.InvokeRequired)
                ctrl.Invoke(act);
            else
                act();
        }

        #region ISSIModulesView Members

        public event EventHandler<SSIModuleEventArgs> SSIModuleSelected;
        public event EventHandler<SSIModuleEventArgs> SSIModuleActivated;

        public IEnumerable<MetaData> SSIModules
        {
            set 
            {
                TreeNode pNode = null;
                foreach (var item in value)
                {
                    var tag = Tools.ObjectTypeToTag(item.Type);

                    if (tag == null)
                    {
                        continue;
                    }

                    if (treeView1.Nodes.ContainsKey(tag))
                        pNode = treeView1.Nodes[tag];
                    else
                    {
                        pNode = new TreeNode(tag) { Name = tag };

                        PerformInvoke(treeView1, () => treeView1.Nodes.Add(pNode));
                    }

                    var tnNode = new TreeNode(item.Name);
                    tnNode.Tag = item;
                    PerformInvoke(treeView1, () => pNode.Nodes.Add(tnNode));                    
                }

                PerformInvoke(treeView1, () => treeView1.Sort());
            }
        }

        #endregion

        #region ISSIModelView Members

		public object Selected
		{
			set
			{
				PerformInvoke(propertyGrid1, () =>
				{
					propertyGrid1.SelectedObject = value;
					propertyGrid1.ExpandAllGridItems();
				});
			}
		}

        public event EventHandler<SSIModelChangedEventArgs> ModelChanged;

        #endregion

        public static PropertyGrid Prop { get; set; }

        public MainForm(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

			//this.BackColor = Color.FromArgb(255, 63, 63, 63);

            InitializeComponent();

            m_Application = app;
            
            m_DocumentsView = new DocumentControl(app) { Dock = DockStyle.Fill };
            splitContainer2.Panel1.Controls.Add(m_DocumentsView);
            
            app.DocumentsView = m_DocumentsView;
            
            RegisterViews();  
            
            //menuStrip1.Renderer = new ToolStripProfessionalRenderer(new SSIXmlEditor.View.ColorTable());
            m_MenuItems = new Dictionary<string,ToolStripItem>();
            foreach(ToolStripMenuItem menuItem in menuStrip1.Items)
			{
				//menuItem.ForeColor = Color.White;
				foreach(ToolStripItem item in menuItem.DropDownItems)
				{
					if (item.Tag != null)
						m_MenuItems.Add(item.Tag.ToString(), item);
				}
			}

            m_Controller = new MainController(m_Application, this);
            Init();
        }

        private void Init()
        {   
            m_SSIModulesController = new SSIModulesController(m_Application, m_Application.GetModel<Model.ISSIModules>(), this);
            m_SSIModuleController = new SSIModelController(m_Application.GetModel<Model.SSIModel>(), this);

            //TODO: remove from here
            var mod1 = m_Application.GetModel<Model.ISSIModules>();
            var mod2 = m_Application.GetModel<Model.SSIModel>();

            mod2.SSIModules = mod1;
            //~TODO

            treeView1.AfterSelect += delegate
            {
                MetaData data = treeView1.SelectedNode.Tag as MetaData;
                
                if (data != null)
                    FireModuleSelectedEvent(data);
            };

            treeView1.NodeMouseDoubleClick += delegate
            {
                if (null == treeView1.SelectedNode)
                    return;

                MetaData data = treeView1.SelectedNode.Tag as MetaData;
                if (data != null)
                    FireModuleActivatedEvent(data);
            };

            propertyGrid1.PropertyValueChanged += new PropertyValueChangedEventHandler(propertyGrid1_PropertyValueChanged);

            
        }

        void propertyGrid1_PropertyValueChanged(object s, PropertyValueChangedEventArgs e)
        {
            var item = e.ChangedItem.Parent.Value as Model.BaseObject ?? propertyGrid1.SelectedObject as Model.BaseObject;
											  
            if (null != ModelChanged)
                ModelChanged(this, new SSIModelChangedEventArgs(item, e.ChangedItem.Parent.Label, e.ChangedItem.Label, e.ChangedItem.Value, e.OldValue));
        }

        protected override void OnLoad(EventArgs e)
        {
            //base.OnLoad(e);
            AllowDrop = true;
            DragEnter += (sender, args) =>
            {
                args.Effect = args.Data.GetDataPresent(DataFormats.FileDrop) ? DragDropEffects.All : DragDropEffects.None;
            };

            DragDrop += (sender, args) =>
            {
                var fileList = (string[])args.Data.GetData(DataFormats.FileDrop, false);
                m_Application.Documents.OpenAndActivate(fileList);
            };
        }

        private void RegisterViews()
        {
            m_Application.RegisterView<View.ISSIModulesView>(this);
            m_Application.RegisterView<View.ISSIModelView>(this);
        }

        private void FireModuleSelectedEvent(MetaData selected)
        {
            if (null != SSIModuleSelected)
                SSIModuleSelected(this, new SSIModuleEventArgs(selected));
        }

        private void FireModuleActivatedEvent(MetaData selected)
        {
            if (null != SSIModuleActivated)
                SSIModuleActivated(this, new SSIModuleEventArgs(selected));
        }

        private void MenuClicked(object sender, EventArgs e)
        {
            var ctrl = sender as Component;
            
            m_Application.InvokeCommand("New");
        }

        private void executeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_Application.InvokeCommand("Execute");
        }

        private void correctLineEndingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_Application.InvokeCommand("CorrectLineEndings");
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_Application.InvokeCommand("SaveAs");
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_Application.InvokeCommand("Open");
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_Application.InvokeCommand("Save");
        }

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            m_Application.InvokeCommand("Close");
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);

            m_Application.InvokeCommand("Exit");
        }

		private void beendenToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void graphicalToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if(m_Application.ActiveDocument == null)
				return;
		
			var graph = new GraphicalView(m_Application, m_Application.ModelRepository);
			graph.ShowDialog(); 
		}
	}
}
