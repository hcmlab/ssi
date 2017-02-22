using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;

namespace ssi
{
    public interface IMedia
    {
        UIElement GetView();
        void SetVolume(double volume);
        void Play();
        void Stop();
        void Pause();
        void Move(double to_in_seconds);
        double GetPosition();
        bool IsVideo();
        string GetFilepath();
    }
}
