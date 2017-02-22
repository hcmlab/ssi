using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SSIXmlEditor.Command
{
    class DocumentSaveAsCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public DocumentSaveAsCommand(App.SingleInstanceApp app)
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

            var saveAsDialog = new SaveFileDialog();
            saveAsDialog.Filter = "Pipeline (*.pipeline) |*.pipeline";
            saveAsDialog.RestoreDirectory = true;
            if (saveAsDialog.ShowDialog() == DialogResult.OK)
            {
                doc.SaveAs(saveAsDialog.FileName);
            }
        }

        #endregion
    }
}
