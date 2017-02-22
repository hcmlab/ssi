using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Windows.Controls;
using System.Windows;
using System.Windows.Threading;
using System.Collections.ObjectModel;
using System.Xml;
using System.Windows.Controls.Primitives;
using System.IO;
using System.Threading;
using System.Text.RegularExpressions;

namespace ssi
{
    public delegate void RecordingStarted (); 
    public delegate void RecordingStopped ();

    public class RecordHandler
    {
        RecordControl control;
        Dispatcher dispatcher;

        public Project project;
        StimuliList stimuliList = new StimuliList();
        EventList eventList = new EventList();

        public event RecordingStarted OnRecordingStarted;
        public event RecordingStopped OnRecordingStopped;

        public EventList List
        {
            get { return eventList; }
        }

        string user;
        public string User
        {
            get { return user; }
            set {
                this.control.navigator.startButton.IsEnabled = value != "";
                user = value; 
            }
        }

        public ButtonBase NextButton
        {
            get { return this.control.navigator.nextButton; }
        }

        public RecordHandler(RecordControl recordControl)
        {
            this.control = recordControl;
            this.dispatcher = Dispatcher.CurrentDispatcher;            

            this.control.events.list.ItemsSource = eventList;
                       
            this.control.navigator.startButton.Click += startButtonClick;
            this.control.navigator.stopButton.Click += stopButtonClick;

            this.stimuliList.OnNextStimuliSlide += this.control.browser.OnUpdateStimuliSlide;
            this.stimuliList.OnNextStimuliSlide += OnNextStimuliSlide;
            this.stimuliList.OnWaitForLastStimuli += this.control.browser.OnUpdateLastStimuli;           
            this.stimuliList.OnWaitForLastStimuli += OnLastStimuliSlide;

            this.control.stimuli.list.ItemsSource = stimuliList;
            this.control.stimuli.list.SelectionChanged += listSelectionChanged;               

            //loadList();            
            //SaveStimuli(stimuliList, "stimuli.xml");
            //stimuliList.Clear();
            //LoadStimuli(stimuliList, this, "stimuli.xml");

            checkSelection();
            this.control.navigator.stopButton.IsEnabled = false;
            this.control.navigator.nextButton.IsEnabled = false;
        }

        public void Unload()
        {
        }

        #region SelectionChanged

        public void OnProjectSelectionChanged(Project project)
        {
            this.project = project;
            if (project != null)
            {
                LoadStimuli(this, project.Stimuli);

                // peek other stimuli files
                PeekStimuliFiles(project.StimuliDir, this);

            }
            checkSelection();
        }

        public void OnUserSelectionChanged(Collection<SelectItem> users)
        {
            if (users.Count > 0) 
            {
                this.user = users[0].Name;
            }
            else
            {
                this.user = null;
            }
            checkSelection();
        }

        bool checkSelection()
        {
            bool check = stimuliList != null && project != null && user != null && stimuliList.Count > 0;
            control.navigator.startButton.IsEnabled = check;
            control.stimuli.IsEnabled = project != null && stimuliList != null;

            return check;
        }

        #endregion

        #region StimuliList

        void listSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                this.stimuliList.show((StimuliSlide)e.AddedItems[0]);
            }
        }

        void loadList ()
        {
            stimuliList.Add(new StimuliSlide(new StimuliSourceText("hallo event!", "000000", "12"), new StimuliTriggerEvent(eventList, 3), "labelA"));
            stimuliList.Add(new StimuliSlide(new StimuliSourceText("hallo welt!", "000000", "12"), new StimuliTriggerButton(this.control.navigator.nextButton), "labelB"));
            stimuliList.Add(new StimuliSlide(new StimuliSourceText("timer", "000000", "12"), new StimuliTriggerTimer(5), "labelA"));
            stimuliList.Add(new StimuliSlide(new StimuliSourceText("hallo welt x 3!", "000000", "12"), new StimuliTriggerButtonX(this.control.navigator.nextButton, 3), "labelC"));                        
            stimuliList.Add(new StimuliSlide (new StimuliSourceCode("<html><head></head><body><h1>Bitte zeichnen Sie... </h1></body></html>"), new StimuliTriggerButton(this.control.navigator.nextButton), "labelA"));
            stimuliList.Add(new StimuliSlide(new StimuliSourceURL(new Uri(@"D:\wagner\openssi\ui\test\gesture\stimuli\dreieck.html")), new StimuliTriggerButton(this.control.navigator.nextButton), "labelB"));                               
        }

        public static void SaveStimuli(StimuliList list, string filepath)
        {
            XmlTextWriter xml = new XmlTextWriter(filepath, null);
            xml.Formatting = Formatting.Indented;
            xml.WriteStartDocument();
            xml.WriteComment("SSI MLP Stimuli File");
            xml.WriteStartElement("stimuli");

            StimuliXmlWriter sxml = new StimuliXmlWriter(xml);
            sxml.writeList(list);

            xml.WriteEndElement();
            xml.WriteEndDocument();
            xml.Close();
        }

        public static void PeekStimuliFiles(string stimuliDir, RecordHandler handler)
        {
            string[] files = Directory.GetFiles(stimuliDir, "*.xml");
            ObservableCollection<string> list = new ObservableCollection<string>();
            
            foreach (string file in files) {
                list.Add(Path.GetFileName (file));
            }
            handler.control.stimuliComboBox.SelectedIndex = -1;
            handler.control.stimuliComboBox.ItemsSource = list;
        }

        public static void LoadStimuli(RecordHandler handler, string filename)
        {
            handler.stimuliList.Clear();

            XmlReader xml = XmlReader.Create(new StreamReader(handler.project.StimuliDir + "\\" + filename, Encoding.GetEncoding("ISO-8859-9")));
            xml.ReadToFollowing("stimuli");

            string x = xml.BaseURI;

            StimuliXmlReader sxml = new StimuliXmlReader(handler, xml);
            sxml.readList(handler.stimuliList);
          
            xml.Close();
        }

        #endregion

        #region Record

        public void OnLastStimuliSlide()
        {
            ssi.MlpXmlRun.Stop();
            this.control.navigator.stopButton.IsEnabled = false;           
        }

        void ProcessUpdateEvent(double start, double duration, string label, bool store, bool change)
        {
            dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(
                delegate()
                {
                    EventItem item = new EventItem(stimuliList.getCurrentLabel(), start, duration, store, change);
                    eventList.Add(item);
                    this.control.events.list.ScrollIntoView(item);
                }
            ));
        }

        public void OnNextStimuliSlide(StimuliSlide slide)
        {            
            this.control.navigator.nextButton.IsEnabled = false; 
            this.control.stimuli.list.SelectedItem = slide;
            this.control.stimuli.list.ScrollIntoView(slide);
        }

        private void ProcessRunStart()
        {
            this.control.navigator.startButton.IsEnabled = false;
            this.control.navigator.stopButton.IsEnabled = true;
            this.control.navigator.nextButton.IsEnabled = true;
            this.control.stimuli.list.IsEnabled = false;
            this.control.stimuliComboBox.IsEnabled = false;
            this.eventList.Clear();

            if (OnRecordingStarted != null)
            {
                OnRecordingStarted();
            }
        }

        private void processRunStart()
        {
            ssi.ProcessStartDelegate d = new ProcessStartDelegate(ProcessRunStart);
            this.control.Dispatcher.Invoke(d, new object[] {});            
        }

        private void ProcessRunStop()
        {
            ssi.MlpXmlRun.processStartEvent -= processRunStart;
            ssi.MlpXmlRun.processStopEvent -= processRunStop;
            ssi.MlpXmlRun.processUpdateEvent -= processUpdateEvent;
            ssi.MlpXmlRun.OutputLog();

            this.control.navigator.startButton.IsEnabled = true;
            this.control.navigator.stopButton.IsEnabled = false;
            this.control.navigator.nextButton.IsEnabled = false;
            this.control.stimuli.list.IsEnabled = true;
            this.control.stimuliComboBox.IsEnabled = true;
            this.saveAnno();

            if (OnRecordingStopped != null)
            {
                OnRecordingStopped();
            }
        }

        private void processRunStop()
        {
            ssi.ProcessStartDelegate d = new ProcessStartDelegate(ProcessRunStop);
            this.control.Dispatcher.Invoke(d, new object[] {});
        }

        void processUpdateEvent(double start, double duration, string name, bool store, bool change)
        {
            ProcessUpdateDelegate d = new ProcessUpdateDelegate(ProcessUpdateEvent);
            this.control.Dispatcher.Invoke(d, new object[] { start, duration, name, store, change });
        }

        void startButtonClick(object sender, RoutedEventArgs e)
        {
            if (stimuliList != null && project != null && user != null)
            {
                this.control.navigator.startButton.IsEnabled = false;

                ssi.MlpXmlRun.processStartEvent += processRunStart;
                ssi.MlpXmlRun.processStopEvent += processRunStop;
                ssi.MlpXmlRun.processUpdateEvent += processUpdateEvent;

                ssi.MlpXmlRun.pipelineFilepath = project.Dir + "\\" + project.Name + ".pipeline";
                ssi.MlpXmlRun.trainer = null;
                ssi.MlpXmlRun.signal = project.Signal;
                ssi.MlpXmlRun.anno = null;
                ssi.MlpXmlRun.user = user;
                ssi.MlpXmlRun.log = project.LogDir + "\\" + DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss") + ".log";

                ThreadStart threadStart = new ThreadStart(ssi.MlpXmlRun.Start);
                Thread thread = new Thread(threadStart);
                thread.Start();

                this.stimuliList.startSlideShow();
            }
        }

        void stopButtonClick(object sender, RoutedEventArgs e)
        {
            if (stimuliList != null)
            {
                stimuliList.interruptSlideShow();
            }
        }

        void saveAnno()
        {
            DirectoryInfo di = new DirectoryInfo(project.DataDir + "\\" + user);
            DirectoryInfo[] dirs = di.GetDirectories("????-??-??_??-??-??");            
            
            DirectoryInfo newest = dirs[0];
            DateTime now = DateTime.Now;
            foreach (DirectoryInfo dir in dirs)
            {
                if (now - dir.CreationTime  < now - newest.CreationTime) {
                    newest = dir;
                }
            }

            try 
            {
                StreamWriter sw = new StreamWriter (newest.FullName + "\\" + project.Anno + ".anno");
                foreach (EventItem item in eventList)
                {
                    if (item.Store)
                    {
                        sw.WriteLine(item.Start + " " + (item.Start + item.Duration) + " " + item.Label);
                    }
                }
                sw.Close();
            }
            catch (Exception e)
            {
                Error.Show(e.ToString ());
            }
        }

        #endregion       
    }
}
