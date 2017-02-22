using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using SSIXmlEditor.Document;

namespace SSIXmlEditor.Command
{
    class DocumentOpenCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public DocumentOpenCommand(App.SingleInstanceApp app)
        {
            if (null == app)
                throw new ArgumentNullException("app");

            m_App = app;
        }

        #region ICommand Members

        public void Execute()
        {
            var openDialog = new OpenFileDialog();
            openDialog.Filter = "Pipeline (*.pipeline) |*.pipeline";
            openDialog.RestoreDirectory = true;
            openDialog.Multiselect = true;
            if (openDialog.ShowDialog() == DialogResult.OK)
            {
                m_App.Documents.OpenAndActivate(openDialog.FileNames);
            }
        }

        #endregion
    }
}