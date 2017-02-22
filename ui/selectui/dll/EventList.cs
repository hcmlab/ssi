using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;

namespace ssi
{
    public class EventItem
    {

        public EventItem(string label, double start, double duration, bool store, bool change)
        {
            this.label = label;
            this.start = start;
            this.duration = duration;
            this.store = store;
            this.change = change;
        }

        bool store;
        public bool Store
        {
            get { return store; }
            set { store = value; }
        }

        bool change;
        public bool Change
        {
            get { return change; }
            set { change = value; }
        }

        double start;
        public double Start
        {
            get { return start; }
            set { start = value; }
        }

        double duration;
        public double Duration
        {
            get { return duration; }
            set { duration = value; }
        }

        String label;
        public String Label
        {
            get { return label; }
            set { label = value; }
        }
    }

    public class EventList : ObservableCollection<EventItem>
    {

    }
}
