using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace SSIXmlEditor.Command
{
    class ExecutePipelineCommand : ICommand
    {
        private App.SingleInstanceApp m_App;

        public ExecutePipelineCommand(App.SingleInstanceApp app)
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

			m_App.InvokeCommand("Save");

            try
            {
                doc.Save();

                var process = new System.Diagnostics.Process();         
                process.StartInfo.FileName = m_App.PipelineApp;
                process.StartInfo.Arguments = string.Format("\"{0}\"", doc.File.FullName);
                //process.StartInfo.UseShellExecute = false;
                //process.StartInfo.RedirectStandardError = true;
                process.Start();
                
                //StreamReader reader = process.StandardOutput;
                
                //System.Diagnostics.Debug.Write(process.StandardError.ReadToEnd());
                
                process.WaitForExit();
                int result = process.ExitCode;

                if (result != 0)
                {
                    string err = File.ReadAllText(doc.File.Directory.FullName + "\\ssi_last.err");
                    throw new Exception(err);
                }

            }
            catch (InvalidOperationException ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }            
        }

        #endregion
    }
}
