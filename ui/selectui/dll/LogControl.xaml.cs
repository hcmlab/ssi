using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.Windows.Threading;

namespace ssi
{
    /// <summary>
    /// Interaction logic for LogControl.xaml
    /// </summary>
    public partial class LogControl : UserControl
    {
        Dispatcher dispatcher = null;

        public LogControl()
        {
            InitializeComponent();

            dispatcher =  Dispatcher.CurrentDispatcher;

            ssi.MlpXmlRun.processOutputEvent += writeToLogSafe;
            ssi.MlpXmlTrain.processOutputEvent += writeToLogSafe;
        }

        private void writeToLog(string line)
        {
            try
            {
                logText.AppendText(line + Environment.NewLine);
            }
            catch (Exception e)
            {
                Error.Show(e.Message);
            }
        }

        private void writeToLogSafe(string line)
        {
            ssi.ProcessOutputDelegate d = new ssi.ProcessOutputDelegate(writeToLog);
            dispatcher.Invoke(d, new object[] { line });           
        }
    }
}
