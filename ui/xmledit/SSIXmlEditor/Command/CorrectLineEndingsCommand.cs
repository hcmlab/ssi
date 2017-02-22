using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace SSIXmlEditor.Command
{
    class CorrectLineEndingsCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public CorrectLineEndingsCommand(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

            m_App = app;
        }

        #region ICommand Members

        public void Execute()
        {
            var doc = m_App.ActiveDocument;
			if(null == doc)
				return;

            doc.InputHandler.CorrectLineEndings();        
        }

        #endregion
    }
}
