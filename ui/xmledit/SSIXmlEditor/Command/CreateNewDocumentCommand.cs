using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.App;

namespace SSIXmlEditor.Command
{
    public class CreateNewDocumentCommand : ICommand
    {
        private SingleInstanceApp m_App;
        private Provider.IFileNameProvider m_Provider;

        public CreateNewDocumentCommand(SingleInstanceApp app, Provider.IFileNameProvider provider)
        {
            if (null == app)
                throw new ArgumentNullException("app");
            if (null == provider)
                throw new ArgumentNullException("provider");

            m_App = app;
            m_Provider = provider;
        }

        #region ICommand Members

        public void Execute()
        {
            var doc = m_App.Documents.Create(m_Provider.CreateName());
            doc.Activate();
        }

        #endregion
    }
}
