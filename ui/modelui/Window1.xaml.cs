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
using System.Globalization;
using System.Threading;

namespace ssi
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        GridLength previousExpandLength = new GridLength();

        SelectHandler selecth;
        RecordHandler recordh;
        ViewHandler viewh;
        TrainHandler trainh;
        ReLabelHandler relabelh;

        public Window1()
        {
            InitializeComponent();

            CultureInfo ci = new CultureInfo("en-GB");
            Thread.CurrentThread.CurrentCulture = ci;
            Thread.CurrentThread.CurrentUICulture = ci;

            record.OnHandlerLoaded += recordHandlerLoaded;
            select.OnHandlerLoaded += selectHandlerLoaded;            
            view.OnHandlerLoaded += viewHandlerLoaded;
            train.OnHandlerLoaded += trainHandlerLoaded;
            relabel.OnHandlerLoaded += relabelHandlerLoaded;

            tab.SelectedIndex = 0;
            projectQuickHelp.Text = "The control on the left allows you to manage your projects. You can add a new or existing project by pressing '+' or remove a project by pressing '-'. Likewise you can add new users. A new recording is always assigned to the user that is currently selected and according recordings are listed and automatically updated when a new recording is finished. During model evaluation and training you have the possibilty to select several users using the 'ctrl' key and choose from their sessions (again using the 'ctrl' key).";
        }

        void selectHandlerLoaded(SelectHandler handler)
        {
            selecth = handler;

            useRecycleBin.SetBinding(CheckBox.IsCheckedProperty, selecth.useRecycleBinBinding);
            selecth.UseMultiFilterProperty = true;
            selecth.UseMultiSelectProperty = true;
            useMultiSelect.SetBinding(CheckBox.IsCheckedProperty, selecth.useMultiSelectBinding);
            selecth.UseSimpleInterfaceProperty = true;
            useSimpleInterface.SetBinding(CheckBox.IsCheckedProperty, this.selecth.useSimpleInterfaceBinding);
            selecth_OnUseSimpleInterfaceChanged(this.selecth.UseSimpleInterfaceProperty);
            selecth.OnUseSimpleInterfaceChanged += new UseSimpleInterfaceChanged(selecth_OnUseSimpleInterfaceChanged);

            recordAndSelectLoaded();
            viewAndSelectLoaded();
            trainAndSelectLoaded();
            relabelAndSelectLoaded();
        }

        void selecth_OnUseSimpleInterfaceChanged(bool flag)
        {
            this.tabReLabel.Visibility = flag ? Visibility.Collapsed : Visibility.Visible;
        }

        void recordHandlerLoaded(RecordHandler handler)
        {
            recordh = handler;            
            recordAndSelectLoaded();            
        }

        void viewHandlerLoaded(ViewHandler handler)
        {
            viewh = handler;
            this.KeyDown += handler.OnKeyDown;
            viewAndSelectLoaded();
        }

        void trainHandlerLoaded(TrainHandler handler)
        {
            trainh = handler;
            useRecycleBin.SetBinding(CheckBox.IsCheckedProperty, trainh.useRecycleBinBinding);
            trainAndSelectLoaded();
        }

        void relabelHandlerLoaded(ReLabelHandler handler)
        {
            relabelh = handler;
            relabelAndSelectLoaded();
        }

        void relabelAndSelectLoaded()
        {
            if (relabelh != null && selecth != null)
            {
                selecth.OnProjectSelectionChanged += relabelh.OnProjectSelectionChanged;
                relabelh.OnProjectSelectionChanged(selecth.Project);
                selecth.OnDateSelectionChanged += relabelh.OnDateSelectionChanged;
                relabelh.OnDateSelectionChanged(selecth.Dates);
                selecth.OnAnnotationSelectionChanged += relabelh.OnAnnotationSelectionChanged;
                relabelh.OnAnnotationSelectionChanged(selecth.Annotations);      
            }
        }

        void recordAndSelectLoaded()
        {
            if (recordh != null && selecth != null)
            {
                selecth.OnProjectSelectionChanged += recordh.OnProjectSelectionChanged;
                recordh.OnUserSelectionChanged(selecth.Users);
                selecth.OnUserSelectionChanged += recordh.OnUserSelectionChanged;
                recordh.OnProjectSelectionChanged(selecth.Project);
                recordh.OnRecordingStopped += onRecordingStopped;
                recordh.OnRecordingStarted += onRecordingStarted;
            }
        }

        void viewAndSelectLoaded()
        {
            if (viewh != null && selecth != null)
            {
                viewh.LoadButton.Visibility = Visibility.Collapsed;
               // selecth.OnDataMouseDoubleClick += viewh.loadFromFile;
                selecth.OnDateMouseDoubleClick += new DateMouseDoubleClick(selecth_OnDateMouseDoubleClick);
            }
        }

        bool selecth_OnDateMouseDoubleClick(string filename)
        {
            bool result = true;

            this.viewh.clear();

            string[] files = Directory.GetFiles(filename);
            foreach (string file in files)
            {
                switch (new FileInfo (file).Extension) 
                {
                    case ".avi":
                    case ".wav":
                    case ".anno":
                    case ".stream":
                    result = this.viewh.loadFromFile(file) && result;
                    break;
                }
            }

            return result;
        }

        void trainAndSelectLoaded()
        {
            if (trainh != null && selecth != null)
            {
                trainh.OnUseSimpleInterfaceChanged(selecth.UseSimpleInterfaceProperty);
                selecth.OnUseSimpleInterfaceChanged += trainh.OnUseSimpleInterfaceChanged;
                selecth.OnProjectSelectionChanged += trainh.OnProjectSelectionChanged;
                trainh.OnProjectSelectionChanged(selecth.Project);
                selecth.OnDateSelectionChanged += trainh.OnDateSelectionChanged;
                trainh.OnDateSelectionChanged(selecth.Dates);
                selecth.OnSignalSelectionChanged += trainh.OnSignalSelectionChanged;
                trainh.OnSignalSelectionChanged(selecth.Signals);
                selecth.OnAnnotationSelectionChanged += trainh.OnAnnotationSelectionChanged;
                trainh.OnAnnotationSelectionChanged(selecth.Annotations);                                                     
            }
        }

        private void Expander_Expanded(object sender, RoutedEventArgs e)
        {
            myGrid.RowDefinitions[0].Height = previousExpandLength;
        }

        private void Expander_Collapsed(object sender, RoutedEventArgs e)
        {
            previousExpandLength = myGrid.RowDefinitions[0].Height;
            myGrid.RowDefinitions[0].Height = GridLength.Auto;
        }

        private void reloadButton_Click(object sender, RoutedEventArgs e)
        {
            if (selecth != null)
            {
                selecth.Reload();
            }
        }

        void onRecordingStarted()
        {
            this.tabTrain.IsEnabled = false;
            this.tabView.IsEnabled = false;
            this.select.IsEnabled = false;
            this.tabReLabel.IsEnabled = false;
        }

        void onRecordingStopped()
        {
            this.tabTrain.IsEnabled = true;
            this.tabView.IsEnabled = true;
            this.select.IsEnabled = true;
            this.tabReLabel.IsEnabled = true;
            selecth.Update();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            if (selecth != null)
            {
                selecth.Unload();
            }
        }

        private void tab_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            try
            {
                TabItem tab = (TabItem)e.AddedItems[0];

                if (tab == tabRecord)
                {
                    quickHelp.Text = "To start a recording select a project and a user first, then press the 'Start Recording' button. You are now presented a slideshow with instructions, images, videos etc. Usually, you will be automatically taken to the next task when an event has been detected. You can monitor new events in the lower of the two tables on the right. Sometimes, however, you have to manually switch to the next slide. In this case the 'Next' button becomes available. Press it to display the next slide. The recording automatically stops after the last slide. However, you can interrupt it at anytime by pressing the 'Stop' (all data that has been recorded so far will be stored). When no recording is under progress you can use upper list on the right to browse the slides off-line.";
                }
                else if (tab == tabView)
                {
                    quickHelp.Text = "To view a recorded session select a user and double click the desired session name. Videos (when available), volume controls for each media and a table listing segments of the current annotation track will be shown on the left side. On the right, signal and annotation tracks will be displayed. Recordings containing at least one media track (audio or video) can be replayed in real-time from the current cursor position by pressing the 'Play' button or hitting the space key. Other buttons allow you to add and save new annotations. To add a new segment to an annotation track press the right mouse button. You can adjust segment length and position by moving the cursor over of the segment. To select a segment click on it with the left mouse button. Hit the 'enter' key to assign a new label or 'Del' to delete it. Same operations can also be applied to multiple segments from the segment table (press the 'ctrl' key to make multiple selections). Note that changes to an annotation will not be permanent until pressing the 'Save' button.";
                }
                else if (tab == tabReLabel)
                {
                    quickHelp.Text = "Allows you to quickly rename labels or map different labels to a single one. Just select an annotation and type a new name for the annotation and press 'Import'. For each label category you can now choose a new name. By assigning the same label name to different categeories you can merge them. Press 'Ok' to save the changes to a new annotation.";
                }
                else if (tab == tabTrain)
                {
                    quickHelp.Text = "Allows you to evaluate, train and run a model. The extraction bar is used by both, the evaluation and the training panel to extract the samples from the selected signal. The method bar on top let you choose the type of features and classifier used to train the model. When at least one recording session and a training method are selected the 'Start Evaluation' button becomes available to invoke evaluation on the selected recording sessions. The result of an evaluation is displayed as a confusion matrix and you can copy it to the clipboard or store it in a text file. While the training panel is visible you can press 'Start Training' to use all selected recording sessions for training a new model. When training is finished the model is added to the list of models. By selecting a model from the list and pressing 'Run Model' you can immidiatelly test your model in real-time. Detected events are classified and the classification result will be displayed.";
                }
            }
            catch (Exception ex)
            {
            }
       }

    }
}
