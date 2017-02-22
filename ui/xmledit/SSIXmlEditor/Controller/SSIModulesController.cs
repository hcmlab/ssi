using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.App;
using SSIXmlEditor.View;

namespace SSIXmlEditor.Controller
{
    class SSIModulesController
    {
        private App.SingleInstanceApp m_App;
        private Model.ISSIModules m_Model;
        private View.ISSIModulesView m_View;

        public SSIModulesController(App.SingleInstanceApp app, Model.ISSIModules model, View.ISSIModulesView view)
        {
            if (null == app)
                throw new ArgumentNullException("app");
            if (null == model)
                throw new ArgumentNullException("model");
            if (null == view)
                throw new ArgumentNullException("view");

            m_App = app;
            m_Model = model;
            m_View = view;

            m_Model.Loaded += new EventHandler(m_Model_Loaded);
            m_View.SSIModuleActivated += new EventHandler<SSIModuleEventArgs>(m_View_SSIModuleActivated);
            m_View.SSIModuleSelected += new EventHandler<SSIModuleEventArgs>(m_View_SSIModuleSelected);

            if(null != model.Modules)
                m_View.SSIModules = model.Modules;
        }

        void m_Model_Loaded(object sender, EventArgs e)
        {
            m_View.SSIModules = m_Model.Modules;
        }

        void m_View_SSIModuleSelected(object sender,SSIModuleEventArgs e)
        {
            m_Model.Select(e.SSIModule.Name);
        }

        void m_View_SSIModuleActivated(object sender, SSIModuleEventArgs e)
        {
            if(null == m_App.ActiveDocument)
                return;

            var type = Tools.ObjectTypeToTag(e.SSIModule.Type);
            m_App.ActiveDocument.InputHandler.InsertSnippet(type, new string[] { e.SSIModule.Name });
        }
    }
}
