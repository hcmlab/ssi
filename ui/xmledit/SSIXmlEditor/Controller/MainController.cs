using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.Extension;
using SSIXmlEditor.InputHandler;

namespace SSIXmlEditor.Controller
{
    class MainController
    {
        private App.SingleInstanceApp Application { get; set; }
        private View.IMainView View { get; set; }

        private InputHandler.XmlEditorLogic m_EditorLogik;

        public MainController(App.SingleInstanceApp app, View.IMainView view)
        {
            if (null == app)
                throw new ArgumentNullException("app");
            if (null == view)
                throw new ArgumentNullException("view");

            Application = app;
            View = view;
            View.Caption = "SSI/XmlEditor v0.9.7 (http://openssi.net)";
            
            m_EditorLogik = new XmlEditorLogic(app.GetModel<Model.SSIModel>(), app.ModelRepository);//new Repository.ModelRepository("./ClassDef.xml", app.GetModel<Model.SSIModel>()));

            Application.ActiveDocumentChanged += new EventHandler(Application_ActiveDocumentChanged);

			View.RegisterCommand("New", true);
			View.RegisterCommand("Execute", false);
            View.RegisterCommand("CorrectLineEndings", false);
			View.RegisterCommand("Open", true);
			View.RegisterCommand("Save", false);
			View.RegisterCommand("SaveAs", false);
			View.RegisterCommand("Close", false);
			View.RegisterCommand("Exit", true);
        }

        void Application_ActiveDocumentChanged(object sender, EventArgs e)
        {
            var activeDoc = Application.ActiveDocument;

			if (null == activeDoc)
			{
				View.ActivateCommand("New", true);
				View.ActivateCommand("Execute", false);
                View.ActivateCommand("CorrectLineEndings", false);
				View.ActivateCommand("Open", true);
				View.ActivateCommand("Save", false);
				View.ActivateCommand("SaveAs", false);
				View.ActivateCommand("Close", false);
				View.ActivateCommand("Exit", true);
			}
			else
            {
				m_EditorLogik.InputHandler = activeDoc.InputHandler;

				View.ActivateCommand("New", true);
				View.ActivateCommand("Execute", true);
                View.ActivateCommand("CorrectLineEndings", true);
				View.ActivateCommand("Open", true);
				View.ActivateCommand("Save", true);
				View.ActivateCommand("SaveAs", true);
				View.ActivateCommand("Close", true);
				View.ActivateCommand("Exit", true);
			}            
        }
    }
}
