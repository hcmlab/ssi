using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Windows.Forms;
using System.Windows.Input;
using Microsoft.Shell;
using SSIXmlEditor.App;
using SSIXmlEditor.Command;

namespace SSIXmlEditor
{
    public static class Program
    {
        private const string Unique = "6DA31DA2-1B16-43BF-BEAE-5E7C32693A5A";

        /// <summary>
        ///     The main entry point for the application.
        /// </summary>
        [STAThread]
        private static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);


            var openAnotherInstance = Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift);
            if (openAnotherInstance || SingleInstance<SingleInstanceApp>.InitializeAsFirstInstance(Unique))
            {
                Application.ThreadException += Application_ThreadException;
                AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;

                var application = new SingleInstanceApp();

                // set working directory to process path.
                Process process = Process.GetCurrentProcess();
                string processPath = process.Modules[0].FileName;
                var processDir = Path.GetDirectoryName(processPath);
                if (processDir != null) Directory.SetCurrentDirectory(processDir);
                application.Init(new StartUpCommand(application));
                application.MainView = new MainForm(application);
                application.OnReceiveExternalClArgs(Environment.GetCommandLineArgs());
                Application.Run(application.MainView as Form);
                // Allow single instance code to perform cleanup operations
                SingleInstance<SingleInstanceApp>.Cleanup();
            }
        }

        private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            MessageBox.Show(e.ExceptionObject + Environment.NewLine + (e.ExceptionObject as Exception).StackTrace);
        }

        private static void Application_ThreadException(object sender, ThreadExceptionEventArgs e)
        {
            MessageBox.Show(e.Exception.Message + Environment.NewLine + e.Exception.StackTrace);
        }
    }
}