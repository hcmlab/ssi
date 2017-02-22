using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace SSIXmlEditor.Command
{
    class LoadSnippetsCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public LoadSnippetsCommand(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

            m_App = app;
        }

        #region ICommand Members

        public void Execute()
        {
            if (!Directory.Exists("./snippets"))
                return;

            foreach (var snippet in Directory.GetFiles("./snippets", "*.xml"))
                m_App.SnippetManager.Load(snippet);
        }

        #endregion
    }
}
