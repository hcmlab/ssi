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
using System.Diagnostics;
using System.Windows.Media;
using System.Windows.Input;
using System.Windows.Threading;
using Microsoft.VisualBasic.FileIO;
using System.Threading;
using System.Text.RegularExpressions;

namespace ssi
{
    public class TrainHandler
    {
        TrainControl control;

        ModelList modelList = new ModelList();
        ModelItem model;        
        Dispatcher dispatcher;
        EventList eventList = new EventList();
        bool useSimpleInterface;

        Project project;
        public Project Project
        {
            get { return project; }
            set
            {
                updateMethodList(value);
                updateModelList(value);
                project = value;
            }
        }

        TrainInfo info = new TrainInfo();
        public TrainInfo Info
        {
            get { return info; }
            set { info = value; }
        }

        WaitDialog dialog = null;

        public const string EVAL_TMP_FILEPATH = "~eval.tmp";

        public TrainHandler(TrainControl control)
        {
            this.control = control;           

            control.extract.methods.SelectionChanged += OnMethodSelectionChanged;
            control.extract.startButton.Click += startButton_Click;
            control.extract.reextract.Checked += new RoutedEventHandler(reextract_Checked);
            control.extract.reextract.Unchecked += new RoutedEventHandler(reextract_Unchecked);

            //control.eval.evalFullRadioButton.Checked += evalFullRadioButton_Checked;
            //control.eval.evalCrossRadioButton.Checked += evalCrossRadioButton_Checked;
            //control.eval.evalLOORadioButton.Checked += evalLOORadioButton_Checked;
            //control.eval.evalLOUORadioButton.Checked += evalLOUORadioButton_Checked;            

            control.eval.evalMethod.SelectionChanged += evalMethod_selection;
            control.eval.evalContinuous.Checked += evalContinuous_Checked;
            control.eval.evalContinuous.Unchecked += evalContinuous_Unchecked;

            control.eval.crossFolds_text.TextChanged += foldsTextBox_TextChanged;
            control.eval.contFPS_text.TextChanged += fpsTextBox_TextChanged;
            control.eval.contReps_text.TextChanged += repsTextBox_TextChanged;

            control.evalTab.GotFocus += evalTab_GotFocus;
            control.trainTab.GotFocus += trainTab_GotFocus;            

            dispatcher = Dispatcher.CurrentDispatcher;

            control.run.events.ItemsSource = eventList;

            control.run.models.ItemsSource = modelList;
            control.run.models.SelectionChanged += OnModelSelectionChanged;
            control.run.exportButton.Click += OnExportButtonClick;
            control.run.startButton.Click += OnStartButtonClick;
            control.run.stopButton.Click += OnStopButtonClick;
            control.run.infoButton.Click += OnInfoButtonClick;
            control.run.remModelButton.Click += OnRemModelButtonClick;

            useRecycleBinBinding = new Binding("UseRecycleBinProperty");
            useRecycleBinBinding.Source = this;

            checkOnSelectionChanged();
            control.run.stopButton.IsEnabled = false;

            checkSelection();
        }

        public void Unload ()
        {
        }

        #region Evaluation

        private void foldsTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            try
            {
                info.EvalKFolds = int.Parse(control.eval.crossFolds_text.Text);
            } 
            catch (Exception ex) 
            {
                MessageBox.Show(e.ToString ());
                control.eval.crossFolds_text.Text = info.EvalKFolds.ToString();
            }
        }

        private void fpsTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            try
            {
                info.ContFPS = int.Parse(control.eval.contFPS_text.Text);
            }
            catch (Exception ex)
            {
                MessageBox.Show(e.ToString());
                control.eval.contFPS_text.Text = info.ContFPS.ToString();
            }
        }

        private void repsTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            try
            {
                info.ContReps = int.Parse(control.eval.contReps_text.Text);
            }
            catch (Exception ex)
            {
                MessageBox.Show(e.ToString());
                control.eval.contReps_text.Text = info.ContReps.ToString();
            }
        }

        private void evalTab_GotFocus (object sender, RoutedEventArgs e)
        {
            control.extract.startButton.Content = "Evaluate Model";
            info.EvalOn = true;
        }

        private void trainTab_GotFocus(object sender, RoutedEventArgs e)
        {
            control.extract.startButton.Content = "Train Model";
            info.EvalOn = false;            
        }

        private void evalMethod_selection(object sender, SelectionChangedEventArgs e)
        {
            //Cross Fold
            if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalCross)
            {
                info.EvalType = 0;

                //Folds text box
                control.eval.crossFolds_text.Visibility = System.Windows.Visibility.Visible;
                control.eval.corssFolds_label.Visibility = System.Windows.Visibility.Visible;

                //Continuous mode parameters
                control.eval.evalContinuous.Visibility = System.Windows.Visibility.Collapsed;
                control.eval.evalContinuous.IsChecked = false;
            }
            else
            {
                control.eval.crossFolds_text.Visibility = System.Windows.Visibility.Collapsed;
                control.eval.corssFolds_label.Visibility = System.Windows.Visibility.Collapsed;            
            }

            //Leave one sample out
            if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalLOO)
            {
                info.EvalType = 1;

                //Continuous mode parameters
                control.eval.evalContinuous.Visibility = System.Windows.Visibility.Collapsed;
                control.eval.evalContinuous.IsChecked = false;
            }

            //Leave one user out
            if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalLOUO)
            {
                info.EvalType = 2;
                control.eval.evalContinuous.Visibility = System.Windows.Visibility.Visible;
            }       
            
            //Full
            if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_Full)
            {
                info.EvalType = 3;
                control.eval.evalContinuous.Visibility = System.Windows.Visibility.Visible;
            }
        }

        private void evalContinuous_Checked(object sender, RoutedEventArgs e)
        {
            //change eval type to continuous variant
            if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalCross)
            {
                info.EvalType = 0; //4
            }
            else if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalLOO)
            {
                info.EvalType = 1; //5
            }
            else if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalLOUO)
            {
                info.EvalType = 6;
            }
            else if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_Full)
            {
                info.EvalType = 7;
            }

            control.eval.contFPS_label.Visibility = System.Windows.Visibility.Visible;
            control.eval.contFPS_text.Visibility = System.Windows.Visibility.Visible;

            control.eval.contReps_label.Visibility = System.Windows.Visibility.Visible;
            control.eval.contReps_text.Visibility = System.Windows.Visibility.Visible;
        }

        private void evalContinuous_Unchecked(object sender, RoutedEventArgs e)
        {
            //change eval type to continuous variant
            if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalCross)
            {
                info.EvalType = 0;
            }
            else if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalLOO)
            {
                info.EvalType = 1;
            }
            else if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_evalLOUO)
            {
                info.EvalType = 2;
            }
            else if (control.eval.evalMethod.SelectedItem == control.eval.evalMethod_Full)
            {
                info.EvalType = 3;
            }

            control.eval.contFPS_label.Visibility = System.Windows.Visibility.Collapsed;
            control.eval.contFPS_text.Visibility = System.Windows.Visibility.Collapsed;

            control.eval.contReps_label.Visibility = System.Windows.Visibility.Collapsed;
            control.eval.contReps_text.Visibility = System.Windows.Visibility.Collapsed;
        }

        void showEval(string s)
        {
            control.eval.evalInfo.SetInfo(project.Dir + "\\" + project.Name, s);
        }   

        #endregion

        #region Extract

        void reextract_Unchecked(object sender, RoutedEventArgs e)
        {
            info.ReExtract = false;
        }

        void reextract_Checked(object sender, RoutedEventArgs e)
        {
            info.ReExtract = true;
        }

        void startButton_Click(object sender, RoutedEventArgs e)
        {
            if (checkSelection ())
            {                
                control.extract.startButton.IsEnabled = false;
                
                string outdir;
                if (info.EvalOn)
                {
                    outdir = project.EvalDir + "\\" + DateTime.Now.ToString("yyyy-MM-dd_hh-mm-ss");
                }
                else
                {
                    outdir = project.TrainDir + "\\" + DateTime.Now.ToString("yyyy-MM-dd_hh-mm-ss");
                }
                Directory.CreateDirectory(outdir);
                info.Save(outdir + "\\" + project.Name + ".training");
          
                ssi.MlpXmlTrain.pipelineFilepath = project.Dir + "\\" + project.Name + ".pipeline";
                ssi.MlpXmlTrain.traindef = project.Dir + "\\" + project.Name + ".traindef";
                ssi.MlpXmlTrain.training = outdir + "\\" + project.Name;
                ssi.MlpXmlTrain.reextract = info.ReExtract;
                if (info.EvalOn)
                {
                    ssi.MlpXmlTrain.eval = info.EvalType;
                    ssi.MlpXmlTrain.kfolds = info.EvalKFolds;
                    ssi.MlpXmlTrain.trainer = outdir + "\\" + project.Name + ".eval";
                    ssi.MlpXmlTrain.fps = info.ContFPS;
                    ssi.MlpXmlTrain.reps = info.ContReps;
                }
                else
                {
                    ssi.MlpXmlTrain.eval = -1;
                    ssi.MlpXmlTrain.trainer = outdir + "\\" + project.Name;
                }
                ssi.MlpXmlTrain.log = project.LogDir + "\\" + DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss") + ".log";

                ssi.MlpXmlTrain.processStartEvent += processTrainStartEventSafe;
                ssi.MlpXmlTrain.processStopEvent += processTrainStopEventSafe;

                ThreadStart threadStart = new ThreadStart(ssi.MlpXmlTrain.Start);
                Thread thread = new Thread(threadStart);
                thread.Start();

                dialog = new WaitDialog();
                try
                {
                    dialog.ShowDialog();
                }
                catch (Exception ex)
                {
                    Error.Show(ex.ToString());
                }

                if (dialog.Canceled)
                {
                    ssi.MlpXmlTrain.Cancel();                    
                }
            }
        }

        void processTrainStartEvent()
        {
            control.extract.startButton.IsEnabled = false;

        }

        void processTrainStartEventSafe()
        {
            ssi.ProcessStartDelegate d = new ssi.ProcessStartDelegate(processTrainStartEvent);
            dispatcher.Invoke(d, new object[] { });
                        
        }

        void processTrainStopEvent()
        {
            if (dialog.IsVisible)
            {
                

                dialog.Canceled = false;
                dialog.Close();

                if (info.EvalOn)
                {
                    if (!File.Exists(ssi.MlpXmlTrain.trainer) && !File.Exists(ssi.MlpXmlTrain.trainer + ".eval"))
                        MessageBox.Show("An error might have occured during evaluation (eval file not found)");
                    else
                    {
                        try {
                            string evalString = File.ReadAllText(ssi.MlpXmlTrain.trainer);
                            showEval(evalString);
                        } catch {
                            MessageBox.Show("ERROR: while parsing results");
                        }
                    }
                }
                else
                {
                    if (!File.Exists(ssi.MlpXmlTrain.trainer) && !File.Exists(ssi.MlpXmlTrain.trainer + ".trainer"))
                        MessageBox.Show("An error might have occured during training (model file not found)");
                    else
                    {
                        try {
                            string trainer_dir = new FileInfo(ssi.MlpXmlTrain.trainer).Directory.FullName;
                            OnModelCreated(trainer_dir);
                        } catch {
                            MessageBox.Show("ERROR: while parsing results");
                        }
                    }
                }
            }

            ssi.MlpXmlTrain.processStartEvent -= processTrainStartEventSafe;
            ssi.MlpXmlTrain.processStopEvent -= processTrainStopEventSafe;
            ssi.MlpXmlTrain.OutputLog();

            control.extract.startButton.IsEnabled = true;


        }

        void processTrainStopEventSafe()
        {
            ssi.ProcessStartDelegate d = new ssi.ProcessStartDelegate(processTrainStopEvent);
            dispatcher.Invoke(d, new object[] { });

        }

        void cancelButton_Click(object sender, RoutedEventArgs e)
        {
           
            ssi.MlpXmlTrain.Cancel();
        }

        #endregion

        #region SelectionChanged

        public void OnProjectSelectionChanged(Project project)
        {
            Project = project;
            if (project != null)
            {
                info.Type = project.Type;
                info.Signal = project.Signal;
            }
            checkSelection();
        }

        bool checkSelection()
        {
            bool result = false;

            if (project != null)
            {

                if (useSimpleInterface)
                {
                    if (info.Dates != null)
                    {
                        info.Annotation = project.Anno;
                        info.Signal = project.Signal;
                        info.Type = project.Type;
                        result = true;
                    }
                }
                else
                {
                    result = info.Annotation != null && info.Model != null && info.Signal != null;

                    if (info.Signal != null)
                    {
                        control.extract.signalLabel.Foreground = Brushes.Green;
                        control.extract.signalLabel.Content = info.Signal;
                    }
                    else
                    {
                        control.extract.signalLabel.Foreground = Brushes.Red;
                        control.extract.signalLabel.Content = "select a signal";
                    }

                    if (info.Annotation != null)
                    {
                        control.extract.annoLabel.Foreground = Brushes.Green;
                        control.extract.annoLabel.Content = info.Annotation;
                    }
                    else
                    {
                        control.extract.annoLabel.Foreground = Brushes.Red;
                        control.extract.annoLabel.Content = "select an annotation";
                    }                   
                }

                control.run.startButton.IsEnabled = control.run.infoButton.IsEnabled = model != null;
                control.extract.startButton.IsEnabled = result;
                control.extract.methods.IsEnabled = control.extract.methods.Items.Count > 0;
            }
            
            return result;
        }

        public void OnUseSimpleInterfaceChanged(bool flag)
        {
            useSimpleInterface = flag;
            control.extract.signalLabel.Visibility = flag ? Visibility.Collapsed : Visibility.Visible;
            control.extract.annoLabel.Visibility = flag ? Visibility.Collapsed : Visibility.Visible;
            checkSelection();
        }

        public void OnDateSelectionChanged(Collection<SelectItem> items)
        {
            if (items.Count > 0)
            {
                info.Dates = items;
            }
            else
            {
                info.Dates = null;
            }
            checkSelection();
        }

        public void OnAnnotationSelectionChanged(Collection<SelectItem> items)
        {
            if (items.Count > 0)
            {
                info.Annotation = items[0].Name;
            }
            else
            {
                info.Annotation = null;
            }
            checkSelection();
        }

        public void OnSignalSelectionChanged(Collection<SelectItem> items) {

            if (items.Count > 0)
            {
                info.Signal = items[0].Name;                           
            }
            else
            {
                info.Signal = null;
            }
            checkSelection();
        }

        private void OnMethodSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                info.Model = e.AddedItems[0].ToString();             
            }
            else
            {
                info.Model = null;
            }
            checkSelection();
        }

        #endregion

        #region Run

        void updateMethodList(Project project)
        {
            control.extract.methods.Items.Clear();

            if (project != null)
            {
                foreach (string def in project.DefNames)
                {
                    control.extract.methods.Items.Add(def);
                }
            }

            if (control.extract.methods.Items.Count > 0)
            {
                control.extract.methods.SelectedIndex = 0;
            }
        }

        void updateModelList(Project project)
        {
            modelList.Clear();

            if (project != null)
            {

                string[] dirs = Directory.GetDirectories(project.TrainDir);
                foreach (string d in dirs)
                {
                    ModelItem item = ModelItem.Load(d);
                    if (item != null)
                    {
                        modelList.Add(item);
                    }
                }
            }
        }

        public void OnRemModelButtonClick(object sender, RoutedEventArgs e)
        {
            remHelper(control.run.models.SelectedItems, "models");
        }

        void remHelper(IList items, string name)
        {
            if (items.Count > 0)
            {

                StringBuilder sb = new StringBuilder();
                foreach (ModelItem item in items)
                {
                    sb.Append("'" + item.Name + "' ");
                }
                if (showRemDialog(name + " " + sb + "from the current project (all related files will be " + (useRecycleBin ? "moved to the recycle bin)" : "permanently deleted)")))
                {
                    try
                    {
                        foreach (ModelItem item in items)
                        {
                            string dir = item.Path.Substring(0, item.Path.LastIndexOf('\\'));
                            FileSystem.DeleteDirectory(dir, UIOption.OnlyErrorDialogs, useRecycleBin ? RecycleOption.SendToRecycleBin : RecycleOption.DeletePermanently);
                        }
                    }
                    catch (Exception e)
                    {
                        MessageBox.Show(e.ToString());
                    }
                    updateModelList(project);
                }
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

        public void OnModelSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                model = (ModelItem)e.AddedItems[0];
            }
            else
            {
                model = null;
            }
            checkOnSelectionChanged();
        }

        bool checkOnSelectionChanged()
        {
            bool ready = model != null && project != null;
            control.run.startButton.IsEnabled = control.run.infoButton.IsEnabled = ready;            
            return ready;
        }

        void processUpdateEvent(double start, double duration, string label, bool store, bool change)
        {
            EventItem e = new EventItem(label, start, duration, store, change);
            eventList.Add(e);
            this.control.run.events.ScrollIntoView(e);
        }

        void processUpdateEventSafe(double start, double duration, string name, bool store, bool change)
        {
            if (store)
            {
                ProcessUpdateDelegate d = new ProcessUpdateDelegate(processUpdateEvent);
                this.control.Dispatcher.Invoke(d, new object[] { start, duration, name, store, change });
            }
        }

        private void processRunStart()
        {
            this.control.run.startButton.IsEnabled = false;
            this.control.run.stopButton.IsEnabled = true;
            this.control.run.infoButton.IsEnabled = false;
        }

        private void processRunStartSafe()
        {
            ssi.ProcessStartDelegate d = new ssi.ProcessStartDelegate(processRunStart);
            this.control.Dispatcher.Invoke(d, new object[] {});
        }

        private void processRunStop()
        {
            ssi.MlpXmlRun.processStartEvent -= processRunStartSafe;
            ssi.MlpXmlRun.processStopEvent -= processRunStopSafe;
            ssi.MlpXmlRun.processUpdateEvent -= processUpdateEventSafe;
            ssi.MlpXmlRun.OutputLog();

            this.control.run.startButton.IsEnabled = true;
            this.control.run.exportButton.IsEnabled = true;
            this.control.run.stopButton.IsEnabled = false;
            this.control.run.infoButton.IsEnabled = true;
        }

        private void ReadAllText(string p)
        {
            throw new NotImplementedException();
        }

        private void processRunStopSafe()
        {
            ssi.ProcessStartDelegate d = new ssi.ProcessStartDelegate(processRunStop);
            this.control.Dispatcher.Invoke(d, new object[] {});
        }

        private void OnExportButtonClick(object sender, RoutedEventArgs e)
        {
            try
            {
                Microsoft.Win32.SaveFileDialog saveFileDialog = new Microsoft.Win32.SaveFileDialog();
                saveFileDialog.Filter = "trainer files (*.trainer)|*.trainer|All files (*.*)|*.*";
                saveFileDialog.FilterIndex = 1;
                saveFileDialog.RestoreDirectory = true;

                Nullable<bool> result = saveFileDialog.ShowDialog();

                if (result == true)
                {
                    string targetPath = saveFileDialog.FileName;
                    string sourcePath = model.Path;

                    string targetName = Path.GetFileNameWithoutExtension(targetPath);
                    string sourceName = Path.GetFileNameWithoutExtension(sourcePath);

                    string sourceDir = Path.GetDirectoryName(sourcePath);
                    string targetDir = Path.GetDirectoryName(targetPath);

                    string[] sourceFiles = Directory.GetFiles(sourceDir);
                    for (int i = 0; i < sourceFiles.Length; i++) 
                    {
                        sourceFiles[i] = Path.GetFileName(sourceFiles[i]);
                    }
                    for (int i = 0; i < sourceFiles.Length; i++)
                    {
                        string from = sourceDir + "\\" + sourceFiles[i];
                        string to = targetDir + "\\" + sourceFiles[i].Replace(sourceName, targetName);
                        File.Copy(from, to, true);                       
                    }
                    
                    File.WriteAllText(targetPath, File.ReadAllText(targetPath).Replace(sourceName, targetName));
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void OnStartButtonClick(object sender, RoutedEventArgs e)
        {
            control.run.startButton.IsEnabled = false;
            control.run.exportButton.IsEnabled = false;

            this.eventList.Clear();                 

            ssi.MlpXmlRun.processStartEvent += processRunStartSafe;
            ssi.MlpXmlRun.processStopEvent += processRunStopSafe;
            ssi.MlpXmlRun.processUpdateEvent += processUpdateEventSafe;

            ssi.MlpXmlRun.pipelineFilepath = project.Dir + "\\" + project.Name + ".pipeline";
            ssi.MlpXmlRun.trainer = model.Path;
            ssi.MlpXmlRun.signal = null;
            ssi.MlpXmlRun.anno = null;
            ssi.MlpXmlRun.user = null;
            ssi.MlpXmlRun.log = project.LogDir + "\\" + DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss") + ".log";

            ThreadStart threadStart = new ThreadStart(ssi.MlpXmlRun.Start);
            Thread thread = new Thread(threadStart);
            thread.Start();

        }

        private void OnStopButtonClick(object sender, RoutedEventArgs e)
        {
            control.run.stopButton.IsEnabled = false;

            ssi.MlpXmlRun.Stop();
        }

        private void OnInfoButtonClick(object sender, RoutedEventArgs e)
        {
            string filename = model.Dir + "\\" + model.Name + ".training";
            System.Diagnostics.Process.Start(filename);
        }

        public void OnModelCreated(string path)
        {
            ModelItem item = ModelItem.Load(path);
            if (item != null)
            {
                modelList.Add(item);
            }
        }

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
            }
        }
        #endregion

    }

}
