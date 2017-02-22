using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Windows.Media;
using WPFMediaKit.DirectShow.Controls;
using System.Windows;

namespace ssi
{
    public class MediaKit : MediaUriElement, IMedia
    {
        private string filepath;

        public string GetFilepath ()
        {
            return filepath; 
        }

        public void SetVolume(double volume)
        {
            this.Volume = volume;
        }
                
        public MediaKit(string filepath, double pos_in_seconds)
        {
            this.LoadedBehavior = WPFMediaKit.DirectShow.MediaPlayers.MediaState.Manual;
            this.UnloadedBehavior = WPFMediaKit.DirectShow.MediaPlayers.MediaState.Manual;

            this.BeginInit();
            this.Source = new Uri(filepath);
            this.EndInit();
            // if ScrubbingEnabled is true move correctly shows selected frame, but cursor won't work any more...
            //this.ScrubbingEnabled = true; 
            this.Volume = 1.0;
            this.Pause();         
            this.filepath = filepath;
        }

        public void Move(double to_in_seconds)
        {
            this.MediaPosition = (long)(to_in_seconds * 10000000.0);            
        }

        public double GetPosition()
        {
            return this.MediaPosition / 10000000.0;
        }

        public bool IsVideo()
        {
            return this.HasVideo;
        }

        public UIElement GetView()
        {
            return this;
        }


    }
}
