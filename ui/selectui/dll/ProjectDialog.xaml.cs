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
using System.Windows.Shapes;
using Microsoft.Win32;
using System.IO;
using System.Collections.ObjectModel;

namespace ssi
{    
    public partial class ProjectDialog : Window
    {

        public enum RESULT
        {
            CANCELED = 0,
            CREATE,
            IMPORT
        }

        RESULT result;
        public RESULT Result
        {
            get { return result; }
        }

        string name;
        public string Name
        {
            get { return name; }
        }

        string dir;
        public string Dir
        {
            get { return dir; }
        }

        string stimuli;
        public string Stimuli
        {
            get { return stimuli; }            
        }

        string pipe;
        public string Pipe
        {
            get { return pipe; }
        }

        string def;
        public string Def
        {
            get { return def; }
        }

        string signal;
        public string Signal
        {
            get { return signal; }
        }

        Project.SignalType type;
        public Project.SignalType Type
        {
            get { return type; }
        }

        string anno;
        public string Anno
        {
            get { return anno; }
        }

        public ProjectDialog()
        {
            InitializeComponent();

            result = RESULT.CANCELED;            
            folder.getTreeView().SelectedItemChanged += ProjectDialog_SelectedItemChanged;
            string[] names = Enum.GetNames(typeof(Project.SignalType));          
            foreach (string s in names) {
                typeCombo.Items.Add(s);
            }
            typeCombo.SelectedIndex = 0;

            checkDialog();

            quickHelp.Text = "To create a new project enter a project name first and select a root folder. All recorded signal, annotation, sample and model files, as well as, stimuli slides will be stored inside this folder. Now load a pipeline, definition and stimuli file, and choose a type and name for the recorded signal and annotation. By pressing 'Create' your new project will be loaded and added to the list of projects. When selecting a root folder that already contains a project use the 'Import' button to import the project.";
                 
        }

        void ProjectDialog_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            if (e.NewValue != null)
            {
                dir = ((TreeViewItem)e.NewValue).Tag.ToString();                
            }

            checkDialog();
        }

        private void LoadStimuliClick(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Load Stimuli File"; // Default file name
            dlg.DefaultExt = ".xmal"; // Default file extension
            dlg.Filter = "Stimuli file (.xml)|*.xml"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            if (result.Value)
            {
                this.stimuliText.Text = dlg.FileName; 
            }
        }

        private void LoadPipeClick(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Load Pipeline File"; // Default file name
            dlg.DefaultExt = ".pipeline"; // Default file extension
            dlg.Filter = "Pipeline file (.pipeline)|*.pipeline"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            if (result.Value)
            {
                this.pipeText.Text = dlg.FileName;
            }
        }

        private void LoadDefClick(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();
            dlg.FileName = "Load Definition File"; // Default file name
            dlg.DefaultExt = ".traindef"; // Default file extension
            dlg.Filter = "Definition file (.traindef)|*.traindef"; // Filter files by extension

            // Show open file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            if (result.Value)
            {
                this.defText.Text = dlg.FileName;
            }
        }
       
        void checkDialog()
        {                        
            name = nameText.Text;
            stimuli = stimuliText.Text;
            pipe = pipeText.Text;
            def = defText.Text;
            signal = signalText.Text;
            type = (Project.SignalType) typeCombo.SelectedIndex;
            anno = annoText.Text;

            this.importButton.IsEnabled = false;
            this.createButton.IsEnabled = false;
            if (File.Exists (dir + "\\" + Project.PROJECT_FILE_NAME))
            {
                this.importButton.IsEnabled = true;
            }
            else if (name != "" && dir != null && pipe != "" && def != "" && signal != "" && anno != "")
            {
                this.createButton.IsEnabled = true;
            }           
        }

        private void CreateClick(object sender, RoutedEventArgs e)
        {            
            result = RESULT.CREATE;
            Close();            
        }

        private void ImportClick(object sender, RoutedEventArgs e)
        {
            result = RESULT.IMPORT;
            Close();
        }

        private void CancelClick(object sender, RoutedEventArgs e)
        {
            result = RESULT.CANCELED;
            Close();
        }

        private void nameText_TextChanged(object sender, TextChangedEventArgs e)
        {
            checkDialog();
        }

        private void stimuliText_TextChanged(object sender, TextChangedEventArgs e)
        {
            checkDialog();
        }

        private void pipeText_TextChanged(object sender, TextChangedEventArgs e)
        {
            checkDialog();
        }

        private void defText_TextChanged(object sender, TextChangedEventArgs e)
        {
            checkDialog();
        }

        private void signalText_TextChanged(object sender, TextChangedEventArgs e)
        {
            checkDialog();
        }

        private void typeCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            checkDialog();
        }

        private void annoText_TextChanged(object sender, TextChangedEventArgs e)
        {
            checkDialog();
        }
    }
}
