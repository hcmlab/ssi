using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows;
using System.IO;
using System.Windows.Data;
using System.Collections.ObjectModel;
using System.Collections;
using System.ComponentModel;
using System.Collections.Specialized;
using Microsoft.VisualBasic.FileIO;
using System.Windows.Input;
using System.Globalization;
using System.Threading;

namespace ssi
{
    public delegate void ProjectSelectionChanged (Project project);
    public delegate void UserSelectionChanged (Collection<SelectItem> items);
    public delegate void DateSelectionChanged(Collection<SelectItem> items);
    public delegate void AnnotationSelectionChanged(Collection<SelectItem> items);
    public delegate void SignalSelectionChanged(Collection<SelectItem> items);
    public delegate void SampleSelectionChanged(Collection<SelectItem> items);     
    public delegate bool DataMouseDoubleClick (string filename);
    public delegate bool DateMouseDoubleClick (string filename);
    public delegate void UseSimpleInterfaceChanged (bool flag);

    public class SelectHandler : INotifyPropertyChanged
    {
        SelectControl control;

        Projects projects;
        Project project;
        public event ProjectSelectionChanged OnProjectSelectionChanged;
        public Project Project
        {
            get { return project; }
            set {
                if (OnProjectSelectionChanged != null)
                {
                    OnProjectSelectionChanged(value);
                }
                if (value != null)
                {
                    this.control.session.Path = value.DataDir;
                }
                else
                {
                    this.control.session.Path = "";
                }
                project = value;
            }
        }
        
        public void Unload()
        {
            if (project != null)
            {                
                project.Close();
            }
        }

        public SelectHandler (SelectControl selectControl) {

            this.control = selectControl;

            projects = new Projects(Projects.PROJECTS_FILE_NAME);

            OnProjectSelectionChanged += onProjectSelectionChanged;
            control.project.list.SelectionChanged += projectSelectionChanged;
            control.project.addProjectButton.Click += addProjectClick;
            control.project.remProjectButton.Click += remProjectClick;
            control.session.addUserButton.Click += addUserClick;
            control.session.remUserButton.Click += remUserClick;
            control.session.remDateButton.Click += remDateClick;
            control.session.remDataButton.Click += remDataClick;
            control.session.remAnnoButton.Click += remAnnoClick;
            control.session.remSampButton.Click += remSampClick;

            control.session.data.MouseDoubleClick += onDataMouseDoubleClick;
            control.session.anno.MouseDoubleClick += onAnnoMouseDoubleClick;
            control.session.date.MouseDoubleClick += onDateMouseDoubleClick;
            
            control.session.user.SelectionChanged += onUserSelectionChanged;
            control.session.date.SelectionChanged += onDateSelectionChanged;
            control.session.data.SelectionChanged += onSignalSelectionChanged;
            control.session.anno.SelectionChanged += onAnnotationSelectionChanged;
            control.session.samp.SelectionChanged += onSampleSelectionChanged;

            useMultiSelectBinding = new Binding("UseMultiSelectProperty");
            useMultiSelectBinding.Source = this;
            useMultiFilterBinding = new Binding("UseMultiFilterProperty");
            useMultiFilterBinding.Source = this;
            useRecycleBinBinding = new Binding("UseRecycleBinProperty");
            useRecycleBinBinding.Source = this;
            useSimpleInterfaceBinding = new Binding("UseSimpleInterfaceProperty");
            useSimpleInterfaceBinding.Source = this;

            if (projects != null)
            {
                StringCollection valid = new StringCollection();
                foreach (Project project in projects)
                {                    
                    control.project.list.Items.Add(project);                                        
                }                                
            }
        }

        #region Project

        public void DisplayAnnotations(bool toggle)
        {
            this.control.session.annoPanel.Visibility = toggle ? Visibility.Visible : Visibility.Collapsed;
        }
        
        public void DisplaySignals(bool toggle)
        {
            this.control.session.dataPanel.Visibility = toggle ? Visibility.Visible : Visibility.Collapsed;
        }

        public void DisplaySamples(bool toggle)
        {
            this.control.session.sampPanel.Visibility = toggle ? Visibility.Visible : Visibility.Collapsed;
        }

        public void New()
        {
            ProjectDialog dlg = new ProjectDialog();            
            dlg.ShowDialog();

            Project new_project = null;

            switch (dlg.Result)
            {
                case ProjectDialog.RESULT.CANCELED:
                    break;

                case ProjectDialog.RESULT.CREATE:                    
                    new_project = new Project(dlg.Dir + '\\' + dlg.Name, dlg.Name, dlg.Stimuli, dlg.Pipe, dlg.Def, dlg.Signal, dlg.Type, dlg.Anno);                    
                    break;

                case ProjectDialog.RESULT.IMPORT:
                    new_project = Project.Load(dlg.Dir);
                    break;
            }

            if (new_project != null)
            {                
                control.project.list.Items.Add(new_project);
                control.project.list.SelectedItem = new_project;
                projects.Add(new_project);
                projects.Save(Projects.PROJECTS_FILE_NAME);
            }
        }

        private string createEmptyStimuli(string p)
        {
            throw new NotImplementedException();
        }

        void onProjectSelectionChanged(Project new_project)
        {           
            if (new_project != project)
            {
                if (project != null)
                {
                    project.Close();
                }                
            }            
        }

        void projectSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                Project = (Project)e.AddedItems[0];                
            }
            else
            {
                Project = null;
            }
        }

        #endregion

        #region RemAdd       
 
        public void Reload()
        {
            Project tmp = project;
            Project = null;
            Project = tmp;
        }

        public void Update()
        {
            if (project != null)
            {
                Project = project;
            }
        }

        void addProjectClick(object sender, RoutedEventArgs e)
        {
            New();
        }

        void remSampClick(object sender, RoutedEventArgs e)
        {
            remHelper(Samples, "sample(s)");
        }

        void remAnnoClick(object sender, RoutedEventArgs e)
        {
            remHelper(Annotations, "annotation(s)");
        }

        void remDataClick(object sender, RoutedEventArgs e)
        {
            remHelper(Signals, "signal(s)");        
        }
       
        void remDateClick(object sender, RoutedEventArgs e)
        {
            remHelper(Dates, "date(s)");
        }

        void remUserClick(object sender, RoutedEventArgs e)
        {
            remHelper(Users, "user(s)");
        }

        void remHelper(Collection<SelectItem> items, string name)
        {
            if (items.Count > 0)
            {

                StringBuilder sb = new StringBuilder();
                foreach (SelectItem item in items)
                {
                    sb.Append("'" + item.Name + "' ");
                }
                if (showRemDialog(name + " " + sb + "from the current project (all related files will be " + (useRecycleBin ? "moved to the recycle bin)" : "permanently deleted)")))
                {
                    try
                    {
                        foreach (SelectItem item in items)
                        {
                            if (item.Type == SelectItemType.DIRECTORY)
                            {
                                FileSystem.DeleteDirectory(item.Path, UIOption.OnlyErrorDialogs, useRecycleBin ? RecycleOption.SendToRecycleBin : RecycleOption.DeletePermanently);
                            }
                            else
                            {
                                FileSystem.DeleteFile(item.Path, UIOption.OnlyErrorDialogs, useRecycleBin ? RecycleOption.SendToRecycleBin : RecycleOption.DeletePermanently);
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        MessageBox.Show(e.ToString ());
                    }
                    Update();
                }
            }
        }

        void remProjectClick(object sender, RoutedEventArgs e)
        {
            if (project != null && showRemDialog("project '" + Project.Name + "' from the list of projects (no files will be deleted)"))
            {
                projects.Remove(project);
                projects.Save(Projects.PROJECTS_FILE_NAME);
                project.Close();
                control.project.list.Items.Remove(project);                
            }
        }

        bool showRemDialog(string name)
        {
            string messageBoxText = "Do you want to remove " + name + "?";
            string caption = "Remove";
            MessageBoxButton button = MessageBoxButton.YesNo;
            MessageBoxImage icon = MessageBoxImage.Warning;
            MessageBoxResult result = MessageBox.Show(messageBoxText, caption, button, icon);
            return result == MessageBoxResult.Yes;
        }

        void addUserClick(object sender, RoutedEventArgs e)
        {
            if (project != null)
            {
                UserDialog dlg = new UserDialog();
                dlg.ShowDialog();
                if (dlg.Success)
                {
                    project.AddUser(dlg.Name);
                    control.session.Path = project.DataDir;                    
                }
            }            
        }

        #endregion

        #region Session

        Collection<SelectItem> signals = new Collection<SelectItem> ();
        public Collection<SelectItem> Signals
        {
            get { 
                signals.Clear ();
                control.session.data.AddSelectedItems(ref signals);
                return signals;
            }            
        }
        public event SignalSelectionChanged OnSignalSelectionChanged;
        void onSignalSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (OnSignalSelectionChanged != null)
            {
                OnSignalSelectionChanged(Signals);
            }
        }

        Collection<SelectItem> annotations = new Collection<SelectItem> ();
        public Collection<SelectItem> Annotations
        {
            get { 
                annotations.Clear ();
                control.session.anno.AddSelectedItems(ref annotations);
                return annotations;
            }            
        }
        public event AnnotationSelectionChanged OnAnnotationSelectionChanged;
        void onAnnotationSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (OnAnnotationSelectionChanged != null)
            {
                OnAnnotationSelectionChanged(Annotations);
            }
        }

        Collection<SelectItem> samples = new Collection<SelectItem> ();
        public Collection<SelectItem> Samples
        {
            get {
                samples.Clear();
                control.session.samp.AddSelectedItems(ref samples);
                return samples;
            }            
        }
        public event SampleSelectionChanged OnSampleSelectionChanged;
        void onSampleSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (OnSampleSelectionChanged != null)
            {
                OnSampleSelectionChanged(Samples);
            }
        }

        Collection<SelectItem> dates = new Collection<SelectItem>();
        public Collection<SelectItem> Dates
        {
            get
            {
                dates.Clear();
                control.session.date.AddSelectedItems(ref dates);
                return dates;
            }
        }
        public event DateSelectionChanged OnDateSelectionChanged;
        void onDateSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (OnDateSelectionChanged != null)
            {
                OnDateSelectionChanged(Dates);
            }
        }
        
        Collection<SelectItem> users = new Collection<SelectItem>();
        public Collection<SelectItem> Users
        {
            get
            {
                users.Clear();
                control.session.user.AddSelectedItems(ref users);                
                return users;
            }
        }
        public event UserSelectionChanged OnUserSelectionChanged;
        void onUserSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (OnUserSelectionChanged != null) {
                OnUserSelectionChanged (Users);
            }
        }
        
        #endregion

        #region PropertyChanged
        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string info)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(info));
            }
        }
        #endregion
        
        #region UseMultiFilter
        public Binding useMultiFilterBinding;
        public bool UseMultiFilterProperty
        {
            get { return this.control.session.UseMultiFilter; }
            set
            {
                control.session.UseMultiFilter = value;
                OnPropertyChanged("UseMultiFilterProperty");
            }
        }
        #endregion

        #region UseMultiSelect
        public Binding useMultiSelectBinding;        
        public bool UseMultiSelectProperty
        {
            get 
            {
                return control.session.UseMultiSelection; 
            }
            set
            {
                control.session.UseMultiSelection = value;                
                OnPropertyChanged("UseMultiSelectProperty");
            }
        }        
        #endregion

        #region UseRecycleBin
        bool useRecycleBin = true;
        public Binding useRecycleBinBinding;
        public bool UseRecycleBinProperty        
        {
            get
            {
                return useRecycleBin;
            }
            set
            {
                useRecycleBin = value;
                OnPropertyChanged("UseRecycleBinProperty");
            }
        }
        #endregion

        #region UseSimpleInterface
        public event UseSimpleInterfaceChanged OnUseSimpleInterfaceChanged;
        bool useSimpleInterface = true;
        public Binding useSimpleInterfaceBinding;
        public bool UseSimpleInterfaceProperty
        {
            get
            {
                return useSimpleInterface;
            }
            set
            {
                useSimpleInterface = value;
                OnPropertyChanged("UseSimpleInterfaceProperty");
                control.session.files.Visibility = useSimpleInterface ? Visibility.Collapsed : Visibility.Visible;
                if (OnUseSimpleInterfaceChanged != null)
                {
                    OnUseSimpleInterfaceChanged(useSimpleInterface);
                }
            }
        }
        #endregion

#region DoubleClick

        public event DataMouseDoubleClick OnDataMouseDoubleClick;
        private void onDataMouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (OnDataMouseDoubleClick != null && Signals != null) 
            {
                OnDataMouseDoubleClick(Signals[0].Path);
            }
        }
        private void onAnnoMouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (OnDataMouseDoubleClick != null && Annotations != null)
            {
                OnDataMouseDoubleClick(Annotations[0].Path);
            }
        }

        public event DateMouseDoubleClick OnDateMouseDoubleClick;
        private void onDateMouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (OnDateMouseDoubleClick != null && Dates != null)
            {
                OnDateMouseDoubleClick(Dates[0].Path);
            }
        }        

#endregion
    }
        
}
