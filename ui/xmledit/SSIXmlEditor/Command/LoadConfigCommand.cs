using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.App;
using System.IO;
using System.Windows.Forms;

namespace SSIXmlEditor.Command
{
    class LoadConfigCommand : ICommand
    {
        private SingleInstanceApp m_Editor;

        public LoadConfigCommand(SingleInstanceApp editor)
        {
            if (null == editor)
                throw new ArgumentNullException("editor");

            m_Editor = editor;
        }

        #region ICommand Members

        public void Execute()
        {
       
#if DEBUG
            if (!File.Exists("xmleditd.ini"))
            {
                MessageBox.Show("Init file 'xmleditd.ini' is missing.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                System.Environment.Exit(1);
            }
            var config = new IniFile("xmleditd.ini");
#else
            if (!File.Exists("xmledit.ini"))
            {
                MessageBox.Show("Init file 'xmledit.ini' is missing.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                System.Environment.Exit(1);
            }
            var config = new IniFile("xmledit.ini");
#endif
            var path = config.IniReadValue("env", "ssi_path");
            var pipelineApp = config.IniReadValue("env", "pipeline_app");

            if (pipelineApp.Contains("%"))
            {
                int iStart = pipelineApp.IndexOf('%');
                var key = pipelineApp.Substring(iStart, pipelineApp.LastIndexOf('%') - iStart + 1);
                var _key = key.Substring(1, key.Length - 2);

                pipelineApp = pipelineApp.Replace(key, config.IniReadValue("env", _key));
            }

            Properties.Settings.Default.Path = path;
            Properties.Settings.Default.PipelineApp = pipelineApp;
            
            m_Editor.DllPath = path;
            m_Editor.PipelineApp = pipelineApp;
        }

        #endregion
    }
}
