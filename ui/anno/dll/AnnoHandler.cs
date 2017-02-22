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

namespace ssi
{
    public class AnnoHandler
    {
        AnnoControl control;

        MediaList media_list = new MediaList();
        SelectItem anno;
        Collection<SelectItem> video = new Collection<SelectItem> ();
        Collection<SelectItem> audio = new Collection<SelectItem> ();
        string newAnnoName;
        string newAnnoPath;
        AnnoList newAnno;
        int currentItem;
        bool isPausing;
        bool isRunning;

        Project project;
        public Project Project
        {
            get { return project; }
            set
            {
                project = value;
            }
        }

        public const string EVAL_TMP_FILEPATH = "~eval.tmp";

        public AnnoHandler(AnnoControl control)
        {
            this.control = control;

            this.control.load.annoOutName.TextChanged += annoOutName_TextChanged;
            this.control.load.startButton.Click += startButton_Click;
            this.control.load.stopButton.Click += stopButton_Click;
            this.control.load.pauseButton.Click += pauseButton_Click;
            this.control.load.addLabelButton.Click += addLabel_Click;
            this.control.load.clearLabelButton.Click += clearLabel_Click;
            this.control.play.label.OnLabelSelected += OnLabelSelected;
            this.control.play.list.annoDataGrid.SelectionChanged += annoDataGrid_SelectionChanged;

            checkSelection();
        }

        public void Unload ()
        {
        }

        #region SelectionChanged

        public void OnProjectSelectionChanged(Project project)
        {
            Project = project;
            checkSelection();
        }

        string itemsToString(Collection<SelectItem> items)
        {
            StringBuilder sb = new StringBuilder();
            foreach (SelectItem item in items)
            {
                sb.Append(item.Name + " ");
            }

            return sb.ToString ();
        }

        bool checkSelection()
        {
            bool result = false;

            if (isRunning)
            {
                control.load.startButton.IsEnabled = false;
                control.load.stopButton.IsEnabled = true;
                control.load.pauseButton.IsEnabled = true;                

                if (isPausing)
                {
                    control.load.pauseButton.Content = "Continue";
                    control.play.label.IsEnabled = false;
                    control.play.list.IsEnabled = false;
                }
                else
                {
                    control.load.pauseButton.Content = "Pause";
                    control.play.label.IsEnabled = true;
                    control.play.list.IsEnabled = true;
                }
            }
            else
            {

                if (video.Count > 0)
                {
                    control.load.videoName.Foreground = Brushes.Green;
                    control.load.videoName.Content = itemsToString(video);
                }
                else
                {
                    control.load.videoName.Foreground = Brushes.Red;
                    control.load.videoName.Content = "please select";
                }

                if (audio.Count > 0)
                {
                    control.load.audioName.Foreground = Brushes.Green;
                    control.load.audioName.Content = itemsToString(audio);
                }
                else
                {
                    control.load.audioName.Foreground = Brushes.Red;
                    control.load.audioName.Content = "please select";
                }

                if (anno != null)
                {
                    control.load.annoInName.Foreground = Brushes.Green;
                    control.load.annoInName.Content = anno.Name;
                }
                else
                {
                    control.load.annoInName.Foreground = Brushes.Red;
                    control.load.annoInName.Content = "please select";
                }

                newAnnoName = control.load.annoOutName.Text;

                result = newAnnoName != "" && anno != null && (audio.Count > 0 || video.Count > 0);

                control.load.startButton.IsEnabled = result;                
                control.load.stopButton.IsEnabled = false;                
                control.load.pauseButton.IsEnabled = false;
                control.play.label.IsEnabled = false;
                control.play.list.IsEnabled = false;

            }

            return result;
        }

        public void OnAnnotationSelectionChanged(Collection<SelectItem> items)
        {
            if (items.Count > 0)
            {
                anno = items[0];
            }
            else
            {
                anno = null;
            }
            checkSelection();
        }

        public void OnSignalSelectionChanged(Collection<SelectItem> items) {

            video.Clear();
            audio.Clear();

            if (items.Count > 0)
            {

                foreach (SelectItem item in items) {
                    string type = item.FullName.Substring (item.FullName.LastIndexOf ('.') + 1);
                    if (type == "avi")
                    {
                        video.Add (item);
                    }
                    else if (type == "wav") 
                    {
                        audio.Add (item);
                    }                                        
                }
            }
           
            checkSelection();
        }

        private void annoOutName_TextChanged(object sender, TextChangedEventArgs e)
        {
            checkSelection();
        }

        #endregion

        #region Button

        void addLabel_Click(object sender, RoutedEventArgs e)
        {
            LabelInputBox inputBox = new LabelInputBox("Input", "Enter a new label name", "");
            inputBox.ShowDialog();
            inputBox.Close();
            if (inputBox.DialogResult == true)
            {
                this.control.play.label.addLabel(inputBox.Result());
            }
        }

        void clearLabel_Click(object sender, RoutedEventArgs e)
        {
            this.control.play.label.clear();
        }

        void startButton_Click(object sender, RoutedEventArgs e)
        {
            start();
        }

        void pauseButton_Click(object sender, RoutedEventArgs e)
        {
            pause();
            isPausing = !isPausing;
            checkSelection();
        }

        void stopButton_Click(object sender, RoutedEventArgs e)
        {
            stop();
            isRunning = false;
            isPausing = false;
            checkSelection();
        }

        #endregion

        #region Play

        void start()
        {
            this.control.play.media.clear();
            this.media_list.clear();

            foreach (SelectItem item in video)
            {
                IMedia media = media_list.addMedia(item.Path, 0);
                this.control.play.media.addMedia(media, true);
            }
            foreach (SelectItem item in audio)
            {
                IMedia media = media_list.addMedia(item.Path, 0);
                this.control.play.media.addMedia(media, false);
            }

            newAnno = AnnoList.LoadfromFile(anno.Path);
            foreach (AnnoListItem item in newAnno)
            {
                item.Label = "";
            }

            string dir = anno.Path.Substring(0, anno.Path.LastIndexOf('\\') + 1);
            newAnnoPath = dir + newAnnoName + ".anno";
            newAnno.saveToFile(newAnnoPath);

            control.play.list.annoDataGrid.ItemsSource = newAnno;

            control.load.pauseButton.IsEnabled = true;
            control.load.stopButton.IsEnabled = true;
            control.load.startButton.IsEnabled = false;

            isPausing = false;
            isRunning = true;
            currentItem = 0;                        

            checkSelection();

            play();
        }

        void play()
        {
            media_list.play(newAnno[currentItem], true);
        }

        void pause()
        {
            if (!isPausing)
            {              
                stop();
            }
            else
            {
                play();
            }            
        }

        void stop()
        {
            media_list.stop();
        }

        public void OnLabelSelected(string label)
        {
            newAnno[currentItem].Label = label;
            newAnno.saveToFile(newAnnoPath);
            currentItem++;
            if (currentItem == newAnno.Count)
            {
                currentItem = 0;
            }
            this.control.play.list.annoDataGrid.SelectedIndex = currentItem;            
        }

        void annoDataGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                currentItem = newAnno.IndexOf((AnnoListItem)e.AddedItems[0]);
                this.control.play.list.annoDataGrid.ScrollIntoView(currentItem);
                play();
            }
        }

        #endregion

    }

}
