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
using System.IO;
using Microsoft.Win32;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace ssi
{
    public partial class MainWindow : Window
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct SHELLEXECUTEINFO
        {
            public int cbSize;
            public uint fMask;
            public IntPtr hwnd;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpVerb;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpFile;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpParameters;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpDirectory;
            public int nShow;
            public IntPtr hInstApp;
            public IntPtr lpIDList;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpClass;
            public IntPtr hkeyClass;
            public uint dwHotKey;
            public IntPtr hIcon;
            public IntPtr hProcess;
        }

        public enum ShowCommands : int
        {
            SW_HIDE = 0,
            SW_SHOWNORMAL = 1,
            SW_NORMAL = 1,
            SW_SHOWMINIMIZED = 2,
            SW_SHOWMAXIMIZED = 3,
            SW_MAXIMIZE = 3,
            SW_SHOWNOACTIVATE = 4,
            SW_SHOW = 5,
            SW_MINIMIZE = 6,
            SW_SHOWMINNOACTIVE = 7,
            SW_SHOWNA = 8,
            SW_RESTORE = 9,
            SW_SHOWDEFAULT = 10,
            SW_FORCEMINIMIZE = 11,
            SW_MAX = 11
        }

        [DllImport("shell32.dll", CharSet = CharSet.Auto)]
        static extern bool ShellExecuteEx(ref SHELLEXECUTEINFO lpExecInfo);

        Setting setting;
        IniFile inifile;
        Config config;

        bool changed = false;
        bool Changed
        {
            get { return changed; }
            set
            {
                changed = value;
                saveButton.IsEnabled = changed;
                saveAndRunButton.Content = changed ? "Save and Run" : "Run";
            }
        }

        public MainWindow()
        {
            InitializeComponent();
            Closing += new CancelEventHandler(MainWindow_Closing);

            config = new Config();
            setting = new Setting();
            inifile = new IniFile(Define.INI_FILE_NAME);            
 
            config.PropertyChanged += new PropertyChangedEventHandler(config_PropertyChanged);

            pipeSearchDirTextBox.DataContext = setting;
            pipeSearchDirTextBox.TextChanged += new TextChangedEventHandler(pipeSearchDirTextBox_TextChanged);
            configSearchDirTextBox.DataContext = setting;
            configSearchDirTextBox.TextChanged +=new TextChangedEventHandler(configSearchDirTextBox_TextChanged);
            pipeExePathTextBox.DataContext = setting;
          
            setting.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(setting_PropertyChanged);
            configComboBox.SelectionChanged += new SelectionChangedEventHandler(configComboBox_SelectionChanged);            
            configComboBox.Items.SortDescriptions.Add(new SortDescription("Content", ListSortDirection.Ascending));

            lockCheckBox.Checked += new RoutedEventHandler(editCheckBox_Checked);
            lockCheckBox.Unchecked += new RoutedEventHandler(editCheckBox_Unchecked);
            addItemButton.Click += new RoutedEventHandler(addItemButton_Click);
            remItemButton.Click += new RoutedEventHandler(remItemButton_Click);
            editItemButton.Click += new RoutedEventHandler(editItemButton_Click);
            newButton.Click += new RoutedEventHandler(newButton_Click);
            saveButton.Click += new RoutedEventHandler(saveButton_Click);
            saveAsButton.Click += new RoutedEventHandler(saveAsButton_Click);
            pipeSearchDirButton.Click += new RoutedEventHandler(pipeSearchDirButton_Click);
            pipeEditButton.Click += new RoutedEventHandler(pipeEditButton_Click);
            configSearchDirButton.Click += new RoutedEventHandler(configSearchDirButton_Click);
            configEditButton.Click += new RoutedEventHandler(configEditButton_Click);
            pipeExePathButton.Click += new RoutedEventHandler(xmlpipeExePathButton_Click);
            parsePipeButton.Click += new RoutedEventHandler(parsePipeButton_Click);
            reloadButton.Click += new RoutedEventHandler(reloadButton_Click);
            saveAndRunButton.Click += new RoutedEventHandler(saveAndRunButton_Click);

            Width = Properties.Settings.Default.WindowWidth;
            Height = Properties.Settings.Default.WindowHeight;
            Left = Properties.Settings.Default.WindowLeft;
            Top = Properties.Settings.Default.WindowTop;
            lockCheckBox.IsChecked = Properties.Settings.Default.Locked;
            Locked(lockCheckBox.IsChecked.Value);
            configControl.keyColumn.Width = Properties.Settings.Default.KeyColumnWidth;
            configControl.valueColumn.Width = Properties.Settings.Default.ValueColumnWidth;
            configControl.commentColumn.Width = Properties.Settings.Default.CommentColumnWidth;
            configControl.ItemDoubleClick += ConfigControl_ItemDoubleClick;

            string value = null;

            value = inifile.GetValue("exe", "path", "");
            if (value != "" && File.Exists(value))
            {
                setting.PipeExePath = value;
            }
            else
            {
                setting.PipeExePath = Properties.Settings.Default.PipeExePath;
            }

            value = inifile.GetValue("pipe", "path", "");
            if (value != "" && Directory.Exists(value))
            {
                setting.PipeSearchDir = value;
            }
            else
            {
                setting.PipeSearchDir = Properties.Settings.Default.PipeSearchDir;
            }

            value = inifile.GetValue("config", "path", "");
            if (value != "" && Directory.Exists(value))
            {
                setting.ConfigSearchDir = value;
            }
            else
            {
                setting.ConfigSearchDir = Properties.Settings.Default.ConfigSearchDir;
            }

            configComboBox.SelectedItem = Properties.Settings.Default.ConfigFileName;
            pipeComboBox.SelectedItem = Properties.Settings.Default.PipeFileName;
        }

        void saveAndRunButton_Click(object sender, RoutedEventArgs e)
        {
            if (Changed)
            {
                SaveConfig();
            }
            RunPipe();
        }

        void pipeSearchDirTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (pipeSearchDirTextBox.IsFocused && Directory.Exists(pipeSearchDirTextBox.Text))
            {
                setting.PipeSearchDir = pipeSearchDirTextBox.Text;
            }
        }

        void configSearchDirTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (configSearchDirTextBox.IsFocused && Directory.Exists(configSearchDirTextBox.Text))
            {
                setting.ConfigSearchDir = configSearchDirTextBox.Text;
            }
        }

        void Locked(bool flag)
        {
            configControl.IsEnabled = !flag;
            addItemButton.IsEnabled = !flag;
            remItemButton.IsEnabled = !flag;
            editItemButton.IsEnabled = !flag;
            parsePipeButton.IsEnabled = !flag;
            saveAsButton.IsEnabled = !flag;
            newButton.IsEnabled = !flag;            
        }

        void editCheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            Locked(false);
        }

        void editCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            Locked(true);
        }

        void config_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            Changed = true;            
        }

        void MainWindow_Closing(object sender, CancelEventArgs e)
        {
            if (!AskToSave())
            {
                e.Cancel = true;
            }
            else
            {
                Properties.Settings.Default.ConfigSearchDir = setting.ConfigSearchDir;
                Properties.Settings.Default.PipeSearchDir = setting.PipeSearchDir;
                Properties.Settings.Default.PipeExePath = setting.PipeExePath;
                Properties.Settings.Default.ConfigFileName = (string)configComboBox.SelectedItem;
                Properties.Settings.Default.PipeFileName = (string)pipeComboBox.SelectedItem;
                Properties.Settings.Default.WindowWidth = ActualWidth;
                Properties.Settings.Default.WindowHeight = ActualHeight;
                Properties.Settings.Default.WindowLeft = Left;
                Properties.Settings.Default.WindowTop = Top;
                Properties.Settings.Default.Locked = lockCheckBox.IsChecked.Value;
                Properties.Settings.Default.KeyColumnWidth = configControl.keyColumn.Width;
                Properties.Settings.Default.ValueColumnWidth = configControl.valueColumn.Width;
                Properties.Settings.Default.CommentColumnWidth = configControl.commentColumn.Width;
                Properties.Settings.Default.Save();
            }
        }

        void EditItem(Config.Item item)
        {
            if (item != null)
            {
                Config.Item tmp_item = item;
                AddConfigItemWindow dialog = new AddConfigItemWindow(tmp_item)
                {
                    Owner = this
                };
                if (dialog.ShowDialog() == true)
                {
                    item = tmp_item;
                    configControl.Update(config);
                }
            }
        }

        void ConfigControl_ItemDoubleClick(Config.Item item)
        {
            EditItem(item);
        }

        void editItemButton_Click(object sender, RoutedEventArgs e)
        {
            EditItem(SelectedItem());   
        }

        void ReloadConfig()
        {
            Changed = false;
            LoadConfig(CurrentConfigPath());
        }

        void LoadConfig(string path)
        {
            if (!AskToSave())
            {
                return;
            }

            if (File.Exists(path))
            {
                config.Clear();
                config.Load(path);
                configControl.Update(config);

                Changed = false;
            }            
        }

        void reloadButton_Click(object sender, RoutedEventArgs e)
        {
            ReloadConfig();
        }

        void ParsePipe(string path)
        {
            if (File.Exists(path))
            {
                config.ParseItemsFromPipe(path);
                configControl.Update(config);
            }
        }

        void parsePipeButton_Click(object sender, RoutedEventArgs e)
        {
            ParsePipe(CurrentPipePath());  
        }

        void SelectItem(Config.Item item)
        {
            configControl.listView.SelectedItem = item;
        }

        Config.Item SelectedItem()
        {
            return (Config.Item)configControl.listView.SelectedItem;
        }

        void RemItem(Config.Item item)
        {
            if (item != null)
            {
                config.Remove(item);
                configControl.Update(config);
            }
        }

        void remItemButton_Click(object sender, RoutedEventArgs e)
        {
            RemItem (SelectedItem());            
        }

        void AddItem()
        {
            Config.Item tmp_item = new Config.Item();
            AddConfigItemWindow dialog = new AddConfigItemWindow(tmp_item);
            if (dialog.ShowDialog() == true)
            {
                Config.Item item = new Config.Item (tmp_item.ToString());
                config.Add(item);
                configControl.Update(config);
                SelectItem(item);
            }
        }

        void addItemButton_Click(object sender, RoutedEventArgs e)
        {
            AddItem();
        }

        void EditFile (string path)
        {
            if (File.Exists(path)) {

                SHELLEXECUTEINFO info = new SHELLEXECUTEINFO();
                info.cbSize = System.Runtime.InteropServices.Marshal.SizeOf(info);
                info.lpVerb = "open";
                info.lpFile = path;                
                info.fMask = (uint) 1;
                info.lpClass = ".txt";
                ShellExecuteEx(ref info);       
            }
        }

        string CurrentConfigPath()
        {
            return config.Path;            
        }

        string CurrentPipePath()
        {
            return Path.GetFullPath(setting.PipeSearchDir + "\\" + pipeComboBox.SelectedItem + Define.FILEEX_PIPELINE);
        }

        void configEditButton_Click(object sender, RoutedEventArgs e)
        {
            string path = CurrentConfigPath ();            
            {
                EditFile(path);
            }
        }

        void pipeEditButton_Click(object sender, RoutedEventArgs e)
        {
            string path = CurrentPipePath();
            if (File.Exists(path))
            {
                EditFile(path);
            }
        }      

        void configComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            string name = (string)configComboBox.SelectedItem;

            string path = System.IO.Path.GetFullPath(name + Define.FILEEX_CONFIG);
            LoadConfig (path);
        }

        void setting_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case "ConfigSearchDir":
                    {
                        string[] files = Directory.GetFiles(Path.GetFullPath(setting.ConfigSearchDir), "*" + Define.FILEEX_CONFIG);
                        configComboBox.Items.Clear();
                        foreach (string f in files)
                        {
                            configComboBox.Items.Add(Path.GetFileNameWithoutExtension(f));
                        }
                        configComboBox.SelectedIndex = 0;
                        break;
                    }
                case "PipeSearchDir":
                    {
                        string[] files = Directory.GetFiles(Path.GetFullPath(setting.PipeSearchDir), "*" + Define.FILEEX_PIPELINE);
                        pipeComboBox.Items.Clear();
                        foreach (string f in files)
                        {
                            pipeComboBox.Items.Add(Path.GetFileNameWithoutExtension(f));
                        }
                        pipeComboBox.SelectedIndex = 0;
                        break;
                    }
                case "PipeExePath":
                    {                        
                        break;
                    }
            }
        }

        void xmlpipeExePathButton_Click(object sender, RoutedEventArgs e)
        {        
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "Pipeline Exe|" + Define.FILE_PIPEEXE;
            if (Directory.Exists(setting.ConfigSearchDir))
            {
                dialog.InitialDirectory = Path.GetDirectoryName (Path.GetFullPath(setting.PipeExePath));
            }
            if (dialog.ShowDialog() == true)
            {
                setting.PipeExePath = dialog.FileName;
            }
        }

        void configSearchDirButton_Click(object sender, RoutedEventArgs e)
        {
            WPFFolderBrowser.WPFFolderBrowserDialog dialog = new WPFFolderBrowser.WPFFolderBrowserDialog();            
            if (Directory.Exists(setting.ConfigSearchDir))
            {
                dialog.InitialDirectory = Path.GetFullPath(setting.ConfigSearchDir);
            }
            if (dialog.ShowDialog() == true)
            {
                setting.ConfigSearchDir = dialog.FileName;
            }
        }

        void pipeSearchDirButton_Click(object sender, RoutedEventArgs e)
        {
            WPFFolderBrowser.WPFFolderBrowserDialog dialog = new WPFFolderBrowser.WPFFolderBrowserDialog();
            if (Directory.Exists(setting.PipeSearchDir))
            {                
                dialog.InitialDirectory = Path.GetFullPath(setting.PipeSearchDir);
            }
            if (dialog.ShowDialog() == true)
            {
                setting.PipeSearchDir = dialog.FileName;                
            }
        }

        void saveButton_Click(object sender, RoutedEventArgs e)
        {
            SaveConfig();
        }

        void SaveConfig()
        {
            SaveConfig(CurrentConfigPath());
        }

        void SaveConfig(string path)
        {                      
            config.Save(path);
            Changed = false;                            
        }

        void newButton_Click(object sender, RoutedEventArgs e)
        {            
            NewConfig();            
        }

        void NewConfig()
        {
            if (!AskToSave())
            {
                return;
            }

            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "Config File|*" + Define.FILEEX_CONFIG;
            if (Directory.Exists(setting.ConfigSearchDir))
            {
                dialog.InitialDirectory = Path.GetFullPath(setting.ConfigSearchDir);
            }
            if (dialog.ShowDialog() == true)
            {
                config.Clear();                
                SaveConfig (dialog.FileName);
                string s1 = Path.GetDirectoryName(dialog.FileName);
                string s2 = Path.GetFullPath(setting.ConfigSearchDir);
                if (string.Compare(s1, s2) != 0)
                {
                    setting.ConfigSearchDir = s1;
                }
                string file = Path.GetFileNameWithoutExtension(dialog.FileName);
                if (!configComboBox.Items.Contains(file))
                {
                    configComboBox.Items.Add(file);                    
                }
                configComboBox.SelectedItem = file;                
            }
        }

        void saveAsButton_Click(object sender, RoutedEventArgs e)
        {
            SaveConfigAs();
        }

        void SaveConfigAs()
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "Config File|*" + Define.FILEEX_CONFIG;
            if (Directory.Exists(setting.ConfigSearchDir))
            {
                dialog.InitialDirectory = Path.GetDirectoryName(setting.ConfigSearchDir);
            }
            if (dialog.ShowDialog() == true)
            {
                config.Save(dialog.FileName);
                string s1 = Path.GetDirectoryName(dialog.FileName);
                string s2 = Path.GetDirectoryName(setting.ConfigSearchDir);
                if (string.Compare(s1, s2) != 0) 
                {
                    setting.ConfigSearchDir = s1;
                }
                string file = Path.GetFileNameWithoutExtension(dialog.FileName);
                if (!configComboBox.Items.Contains(file))
                {
                    configComboBox.Items.Add(file);
                    configComboBox.SelectedItem = file;
                }
            }
        }

        bool AskToSave()
        {
            if (!Changed)
            {
                return true;
            }

            MessageBoxResult result = MessageBox.Show("Save changes?", "Save", MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
            if (result == MessageBoxResult.Yes)
            {
                SaveConfig();
                return true;
            }
            else if (result == MessageBoxResult.No)
            {
                return true;
            }

            return false;
        }

        void RunPipe()
        {
            string filepath = setting.PipeExePath;
            string pipepath = CurrentPipePath();
            string confpath = CurrentConfigPath();
            if (Path.GetFileName (filepath) != Define.FILE_PIPEEXE) 
            {
                Define.ShowErrorMessage ("Invalid Exe '" + filepath + "'");
                return;
            }
            string arguments = "-save -debug \"" + Path.GetFileName(pipepath) + "\".log";            
            if (File.Exists (confpath))
            {
                arguments += " -config \"" + confpath + "\"";
            }            
            if (File.Exists(pipepath))
            {
                arguments += " \"" + pipepath + "\"";
            } 
            else 
            {
                Define.ShowErrorMessage ("Pipeline '" + pipepath + "' not found");
                return;
            }
        
            Process process = new Process();
            process.StartInfo.FileName = "\"" + filepath + "\"";
            process.StartInfo.Arguments = arguments;
            process.StartInfo.RedirectStandardOutput = false;
            process.StartInfo.RedirectStandardError = false;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = false;
            
            try
            {
                process.Start();
                process.WaitForExit();
            }
            catch (Exception e)
            {
                Define.ShowErrorMessage (e.ToString());                
            }
        }
    }
}
