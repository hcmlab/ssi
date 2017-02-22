using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ssi
{
    public class AnnoListItem : MyListItem
    {
        private double start;
        private double duration;
        private String label;

        public double Start
        {
            get { return start; }
            set {
                duration += start - value;
                start = value;                
                OnPropertyChanged("Start");
                OnPropertyChanged("Duration");
            }
        }

        public double Stop
        {
            get { return start + duration; }
            set {
                duration = value - start;
                OnPropertyChanged("Stop");
                OnPropertyChanged("Duration");
            }
        }

        public double Duration
        {
            get { return duration; }
            set {
                duration = value;
                OnPropertyChanged("Stop");
                OnPropertyChanged("Duration");
            }
        }

        public String Label
        {
            get { return label; }
            set
            {
                label = value;
                OnPropertyChanged("Label");
            }
        }

        public AnnoListItem(double _start, double _duration, String _label)
        {
            start = _start;
            duration = _duration;
            label = _label;
        }
/*
        public AnnoListItem()
        {
            start = 0.0;
            duration = 0.0;
            label = "";
        }*/
    }
}
