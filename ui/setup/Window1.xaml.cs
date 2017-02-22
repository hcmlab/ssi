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
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System.Diagnostics;
using System.Security.Principal;

namespace ssi
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        static string X86_DIRECTORY = "Win32";
        static string X64_DIRECTORY = "x64";        
        static string[] COMPILER = { "vc140", "vc120" };
        static string PATH_VARIABLE = "PATH";

        static string SSI_BIN_DIRECTORY = "bin";
        static string SSI_LIBRARY_VARIABLE = "SSI_LIBS";        
        static string SSI_LIBRARY_DIRECTORY = "libs";
        static string SSI_SHARED_DIRECTORY = "build";
        static string SSI_INCLUDE_VARIABLE = "SSI_INCLUDE";
        static string SSI_INCLUDE_DIRECTORY = "include";
        static string SSI_CORE_DIRECTORY = "core";
        static string SSI_PLUGINS_DIRECTORY = "plugins";
        static string SSI_PIPELINE_EXTENSION = ".pipeline";
        static string SSI_PIPELINE_EXECUTABLE = "xmlpipe.exe";
        static string SSI_STREAM_EXTENSION = ".stream";
        static string SSI_STREAM_EXECUTABLE = "viewui.exe";
        static string SSI_PYTHON_VARIABLE = "SSI_PYTHON";
        static string SSI_PYTHON_DIRECTORY = @"C:\Program Files\Python35";

        public Window1()
        {
            InitializeComponent();

            ComboBoxPlatform.Items.Add(X86_DIRECTORY);
            ComboBoxPlatform.Items.Add(X64_DIRECTORY);
            ComboBoxPlatform.SelectedIndex = 1;           

            for (int i = 0; i < COMPILER.Length; i++) {
                ComboBoxCompiler.Items.Add(COMPILER[i]);
            }

            ButtonCancel.Click += new RoutedEventHandler(ButtonCancel_Click);
            ButtonApply.Click += new RoutedEventHandler(ButtonApply_Click);
            ButtonUndo.Click += new RoutedEventHandler(ButtonUndo_Click);

            if (IsElevated() == false)
            {
                CheckBoxPipeline.IsEnabled = false;
                CheckBoxStream.IsEnabled = false;
            }

            ShowCurrentVariables();
        }

        void ButtonUndo_Click(object sender, RoutedEventArgs e)
        {
            this.Cursor = Cursors.Wait;
            Apply(true);
            this.Cursor = Cursors.Arrow;
        }

        void ButtonApply_Click(object sender, RoutedEventArgs e)
        {
            this.Cursor = Cursors.Wait;
            Apply(false);
            this.Cursor = Cursors.Arrow;
        }

        void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        void Log(string text)
        {
            TextBoxLog.Text += Environment.NewLine + text + Environment.NewLine;
        }

        void ShowCurrentVariables()
        {
            string includeVariable = Environment.GetEnvironmentVariable(SSI_INCLUDE_VARIABLE);
            string libraryVariable = Environment.GetEnvironmentVariable(SSI_LIBRARY_VARIABLE);
            string pathVariable = Environment.GetEnvironmentVariable(PATH_VARIABLE);
            string pythonVariable = Environment.GetEnvironmentVariable(SSI_PYTHON_VARIABLE);

            Log(">> CURRENT");
            Log("$(" + SSI_INCLUDE_VARIABLE + "): " + includeVariable);
            Log("$(" + SSI_LIBRARY_VARIABLE + "): " + libraryVariable);
            Log("$(" + PATH_VARIABLE + "): " + pathVariable);
            Log("$(" + SSI_PYTHON_VARIABLE + "): " + pythonVariable);
        }

        void Apply(bool undo)
        {

            string flag = undo ? "UNDO" : "APPLY";

            if (CheckBoxPath.IsChecked == true)
            {
                Log(">> " + flag + " " + CheckBoxPath.Content);
                if (SetPathVariable(undo))
                {
                    Log(">> SUCCESS");
                }
                else
                {
                    Log(">> ERROR");
                }
            }
            if (CheckBoxInclude.IsChecked == true)
            {
                Log(">> " + flag + " " + CheckBoxInclude.Content);
                if (SetIncludeVariable(undo))
                {
                    Log(">> SUCCESS");
                }
                else
                {
                    Log(">> ERROR");
                }
            }
            if (CheckBoxLibrary.IsChecked == true) 
            {
                Log(">> " + flag + " " + CheckBoxLibrary.Content);
                if (SetLibraryVariable(undo))
                {
                    Log(">> SUCCESS");
                }
                else
                {
                    Log(">> ERROR");
                }
            }
            if (CheckBoxPython.IsChecked == true)
            {
                Log(">> " + flag + " " + CheckBoxPython.Content);
                if (SetPythonVariable(undo))
                {
                    Log(">> SUCCESS");
                }
                else
                {
                    Log(">> ERROR");
                }
            }

            if (CheckBoxPipeline.IsChecked == true)
            {                
                string exe = Directory.GetCurrentDirectory() + "\\" + ApplyPlatformAndCompiler(SSI_BIN_DIRECTORY) + "\\" + SSI_PIPELINE_EXECUTABLE;
                string icon = Directory.GetCurrentDirectory() + "\\core\\build\\tools\\ui.ico";

                if (ApplyFileAssociation(undo, SSI_PIPELINE_EXTENSION, "ssipipe", "SSI Pipeline Association", icon, exe, "SSI Pipeline"))
                {
                    Log(">> " + flag + " associate " + SSI_PIPELINE_EXTENSION + " with " + exe);
                }
            }

            if (CheckBoxStream.IsChecked == true)
            {
                string exe = Directory.GetCurrentDirectory() + "\\" + ApplyPlatformAndCompiler(SSI_BIN_DIRECTORY) + "\\" + SSI_STREAM_EXECUTABLE;
                string icon = Directory.GetCurrentDirectory() + "\\core\\build\\tools\\ui.ico";
             
                if (ApplyFileAssociation(undo, SSI_STREAM_EXTENSION, "ssiview", "SSI Viewer Association", icon, exe, "SSI Viewer"))
                {
                    Log(">> " + flag + " associate " + SSI_STREAM_EXTENSION + " with " + exe);
                }         
            }
        }

        private bool ApplyFileAssociation(bool undo, string extension, string progId, string description, string icon, string exe, string openWithName)
        {             
            AF_FileAssociator assoc = new AF_FileAssociator(extension);

            if (File.Exists(exe))
            {
                assoc.Create(progId,
                description,
                new ProgramIcon(icon),
                new ExecApplication(exe),
                new OpenWithList(new string[] { openWithName }));
            }
            else if (!undo)
            {
                Log(">> ERROR: file not found '" + exe + "'");
                return false;
            }

            if (undo)
            {
                assoc.Delete();
            }

            return true;
        }
         
        private string ApplyPlatformAndCompiler(string dir)
        {
            if (ComboBoxPlatform.SelectedIndex == 1)
            {
                dir += "\\" + X64_DIRECTORY + "\\" + ComboBoxCompiler.SelectedItem;
            }
            else
            {
                dir += "\\" + X86_DIRECTORY + "\\" + ComboBoxCompiler.SelectedItem;
            }

            return dir;
        }

        private bool SetIncludeVariable(bool undo)
        {
            if (undo)
            {
                Environment.SetEnvironmentVariable(SSI_INCLUDE_VARIABLE, null, EnvironmentVariableTarget.User);
                return true;
            }

            string dir = Directory.GetCurrentDirectory() + "\\" + SSI_CORE_DIRECTORY + "\\" + SSI_INCLUDE_DIRECTORY;
            TextBoxLog.Text += "core include path: " + dir + Environment.NewLine;

            if (!Directory.Exists(dir))
            {
                Log("path not found: " + dir);
                return false;
            }

            string dir_shared = Directory.GetCurrentDirectory() + "\\" + SSI_LIBRARY_DIRECTORY + "\\" + SSI_SHARED_DIRECTORY;
            TextBoxLog.Text += "shared include path: " + dir_shared + Environment.NewLine;

            if (!Directory.Exists(dir_shared))
            {
                Log("path not found: " + dir_shared);
                return false;
            }

            string dir_plugins = Directory.GetCurrentDirectory() + "\\" + SSI_PLUGINS_DIRECTORY;
            TextBoxLog.Text += "plugins include path: " + dir_plugins + Environment.NewLine;

            if (!Directory.Exists(dir_plugins))
            {
                Log("path not found: " + dir_plugins);
                return false;
            }

            Log("set $(" + SSI_INCLUDE_VARIABLE + "=: " + dir + ";" + dir_shared + ";" + dir_plugins + ")");
            Environment.SetEnvironmentVariable(SSI_INCLUDE_VARIABLE, dir + ";" + dir_shared + ";" + dir_plugins, EnvironmentVariableTarget.User);

            return true;
        }

        private bool SetLibraryVariable(bool undo)
        {
            if (undo)
            {
                Environment.SetEnvironmentVariable(SSI_LIBRARY_VARIABLE, null, EnvironmentVariableTarget.User);
                return true;
            }

            string dir = Directory.GetCurrentDirectory() + "\\" + ApplyPlatformAndCompiler(SSI_LIBRARY_DIRECTORY);
            TextBoxLog.Text += "library path: " + dir + Environment.NewLine;

            if (!Directory.Exists(dir))
            {
                Log("path not found: " + dir);
                return false;
            }

            Log("set $(" + SSI_LIBRARY_VARIABLE + "): " + dir);
            Environment.SetEnvironmentVariable(SSI_LIBRARY_VARIABLE, dir, EnvironmentVariableTarget.User);

            return true;
        }

        private bool SetPythonVariable(bool undo)
        {
            if (undo)
            {
                Environment.SetEnvironmentVariable(SSI_PYTHON_VARIABLE, null, EnvironmentVariableTarget.User);
                return true;
            }

            string dir = SSI_PYTHON_DIRECTORY;
            TextBoxLog.Text += "python path: " + dir + Environment.NewLine;

            if (!Directory.Exists(dir))
            {
                Log("python not found: " + dir);
                return false;
            }

            Log("set $(" + SSI_PYTHON_VARIABLE + "): " + dir);
            Environment.SetEnvironmentVariable(SSI_PYTHON_VARIABLE, dir, EnvironmentVariableTarget.User);

            return true;
        }

        private bool SetPathVariable(bool undo)
        {
            string dir = Directory.GetCurrentDirectory() + "\\" + ApplyPlatformAndCompiler (SSI_BIN_DIRECTORY);
            TextBoxLog.Text += "bin path: " + dir + Environment.NewLine;

            if (!Directory.Exists(dir))
            {
                Log ("path not found: " + dir);
                return false;
            }

            string path = Environment.GetEnvironmentVariable(PATH_VARIABLE, EnvironmentVariableTarget.User);
            if (path == null)
            {
                path = "";
            }
            Log("current $(" + PATH_VARIABLE + "): " + path);
            bool contains = PathContainsDir(path, dir);
            if (!undo && !contains)
            {
                if (path.Length == 0)
                {
                    path = dir;
                }
                else
                {
                    //path += (path.Length > 0 && path[path.Length - 1] == ';' ? "" : ";") + dir;
                    path = dir + ";" + path;
                }
            }
            else if (undo && contains)
            {
                path = PathRemoveDir(path, dir);
            }            
            Log("new $(" + PATH_VARIABLE + "): " + path);
            Environment.SetEnvironmentVariable(PATH_VARIABLE, path, EnvironmentVariableTarget.User);

            return true;

        }

        bool PathContainsDir(string path, string dir)
        {           
            string dirWithSlash = dir + "\\";
            string[] dirs = path.Split(new char[] { ';' });
            foreach (string d in dirs) 
            {
                string dReplace = d.Replace('/', '\\');
                if (dir.Equals(dReplace) || dirWithSlash.Equals(dReplace))
                {
                    return true;
                }
            }
            return false;
        }

        string PathRemoveDir(string path, string dir)
        {
            string dirWithSlash = dir + "\\";
            string[] dirs = path.Split(new char[] { ';' });
            string newPath = "";
            foreach (string d in dirs)
            {
                string dReplace = d.Replace('/', '\\');
                if (! (dir.Equals(dReplace) || dirWithSlash.Equals(dReplace)))
                {
                    newPath += d + ";";
                }
            }
            if (newPath.Length > 0 && newPath[newPath.Length - 1] == ';')
            {
                newPath = newPath.Substring(0, newPath.Length - 1);
            }

            return newPath;
        }

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
            e.Handled = true;

        }

        static internal bool IsElevated()
        {
            WindowsIdentity id = WindowsIdentity.GetCurrent();
            WindowsPrincipal p = new WindowsPrincipal(id);
            return p.IsInRole(WindowsBuiltInRole.Administrator);
        }

        
    }    
}
