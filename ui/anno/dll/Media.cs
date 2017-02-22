using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Windows.Media;
using System.Windows;

namespace ssi
{
   
    public class Media : MediaElement, IMedia
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
                
        public Media(string filepath, double pos_in_seconds)
        {
            this.LoadedBehavior = MediaState.Manual;
            this.UnloadedBehavior = MediaState.Manual;
            this.Source = new Uri(filepath);
            //this.Open (new Uri (filename));
            // if ScrubbingEnabled is true move correctly shows selected frame, but cursor won't work any more...
            //this.ScrubbingEnabled = true; 
            this.Pause();
            this.filepath = filepath;
        }

        public void Move(double to_in_seconds)
        {            
            this.Position = TimeSpan.FromSeconds(to_in_seconds);                     
        }

        public double GetPosition()
        {
            return Position.TotalMilliseconds / 1000.0;
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
