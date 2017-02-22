using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Command
{
    class CloseActiveDocumentCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public CloseActiveDocumentCommand(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

            m_App = app;
        }

        #region ICommand Members

        public void Execute()
        {
            var doc = m_App.ActiveDocument;
            if (doc == null)
                return;

            if (doc.Changed)
            {
                if (System.Windows.Forms.MessageBox.Show("Save changes?", "Question", System.Windows.Forms.MessageBoxButtons.YesNo, System.Windows.Forms.MessageBoxIcon.Question) == System.Windows.Forms.DialogResult.Yes)
                    m_App.InvokeCommand("Save");
            }
            m_App.Documents.Close(doc);
        }

        #endregion
    }
}
