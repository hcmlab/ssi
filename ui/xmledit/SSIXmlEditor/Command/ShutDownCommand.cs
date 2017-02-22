using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Command
{
	//TODO: ShutDownCommand shouldn't call save directly (MacroCommand)
	
	class ShutDownCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public ShutDownCommand(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

            m_App = app;
        }

        #region ICommand Members

        public void Execute()
        {
            foreach (var doc in m_App.Documents)
            {
                doc.Activate();
                m_App.InvokeCommand("Save");
                doc.Close();
            }
        }

        #endregion
    }
}
