using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.ComponentModel;
using System.Windows;
using System.Windows.Threading;
using System.Xml;
using System.Windows.Controls.Primitives;

namespace ssi
{
    public enum StimuliTriggerType
    {
        Button = 0,
        ButtonX,
        Timer,
        Event
    }

    public abstract class IStimuliTrigger : INotifyPropertyChanged
    {
        public IStimuliTrigger()
        {
            TriggerInfoProperty = getInfo();
            TriggerInfoBinding = new Binding("TriggerInfoProperty");
            TriggerInfoBinding.Source = this;
        }

        public abstract string getInfo();
        public abstract void wait(WaitForNextStimuli func);
        public abstract void interrupt();
        public abstract StimuliTriggerType getType();
        public abstract void save(XmlWriter xml);

        #region PropertyChanged

        public Binding TriggerInfoBinding;
        string triggerInfoProperty;
        public string TriggerInfoProperty
        {
            get { return triggerInfoProperty; }
            set
            {
                triggerInfoProperty = value;
                OnPropertyChanged("TriggerInfoProperty");
            }
        }

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
    }

    public class StimuliTriggerButton : IStimuliTrigger
    {
        ButtonBase button;
        event WaitForNextStimuli OnWaitForStimuliSlide;

        public StimuliTriggerButton(ButtonBase button)
        {
            this.button = button;
        }

        public override string getInfo()
        {
            return "Waiting for user to press button..";
        }

        public override void wait(WaitForNextStimuli func)
        {
            button.IsEnabled = true;
            button.Click += OnEvent;
            OnWaitForStimuliSlide += func;
        }

        public override void interrupt()
        {
            OnEvent(this, null);
        }

        void OnEvent(object sender, RoutedEventArgs e)
        {
            button.Click -= OnEvent;
            OnWaitForStimuliSlide();
            OnWaitForStimuliSlide = null;
        }

        public override StimuliTriggerType getType()
        {
            return StimuliTriggerType.Button;
        }

        public override void save(XmlWriter xml)
        {
        }

        public static IStimuliTrigger Load(XmlReader xml, ButtonBase button)
        {
            return new StimuliTriggerButton(button);
        }
    }

    public class StimuliTriggerButtonX : IStimuliTrigger
    {

        ButtonBase button;
        event WaitForNextStimuli OnWaitForStimuliSlide;
        uint number, counter;
        uint Counter
        {
            get { return counter; }
            set
            {
                counter = value;
                TriggerInfoProperty = getInfo();
            }
        }

        public StimuliTriggerButtonX(ButtonBase button, uint x)
        {
            this.button = button;
            this.number = x;
            Counter = x;
        }

        public override string getInfo()
        {
            return "Waiting for user to press button " + counter + " more times..";
        }

        public override void wait(WaitForNextStimuli func)
        {
            button.IsEnabled = true;
            button.Click += OnEvent;
            OnWaitForStimuliSlide += func;
        }

        public override void interrupt()
        {
            Counter = 1;
            OnEvent(this, null);
        }

        void OnEvent(object sender, RoutedEventArgs e)
        {
            if (--Counter == 0)
            {
                button.Click -= OnEvent;
                OnWaitForStimuliSlide();
                OnWaitForStimuliSlide = null;
                Counter = number;
            }
        }

        public override StimuliTriggerType getType()
        {
            return StimuliTriggerType.ButtonX;
        }

        public override void save(XmlWriter xml)
        {
            xml.WriteStartAttribute("number");
            xml.WriteString(number.ToString());
            xml.WriteEndAttribute();
        }

        public static IStimuliTrigger Load(XmlReader xml, ButtonBase button)
        {
            uint x = uint.Parse(xml.GetAttribute("number"));
            return new StimuliTriggerButtonX( button, x);
        }
    }

    public class StimuliTriggerTimer : IStimuliTrigger
    {
        event WaitForNextStimuli OnWaitForStimuliSlide;
        DispatcherTimer timer;
        uint seconds, counter;
        uint Counter
        {
            get { return counter; }
            set
            {
                counter = value;
                TriggerInfoProperty = getInfo();
            }
        }

        public StimuliTriggerTimer( uint seconds)
        {
            this.seconds = seconds;
            timer = new DispatcherTimer();
            timer.Interval = new TimeSpan(0, 0, 1);
            timer.Tick += OnTimerTick;
            Counter = seconds;
        }

        public override string getInfo()
        {
            return "Time to elsapse: " + counter + " seconds..";
        }

        public override void wait(WaitForNextStimuli func)
        {
            OnWaitForStimuliSlide += func;
            timer.Start();
        }

        public override void interrupt()
        {
            Counter = 1;
        }

        private void OnTimerTick(object sender, EventArgs e)
        {
            if (--Counter == 0)
            {
                timer.Stop();
                OnWaitForStimuliSlide();
                OnWaitForStimuliSlide = null;
                Counter = seconds;
            }
        }

        public override StimuliTriggerType getType()
        {
            return StimuliTriggerType.Timer;
        }

        public override void save(XmlWriter xml)
        {
            xml.WriteStartAttribute("seconds");
            xml.WriteString(seconds.ToString());
            xml.WriteEndAttribute();
        }

        public static IStimuliTrigger Load(XmlReader xml)
        {
            uint seconds = uint.Parse(xml.GetAttribute("seconds"));
            return new StimuliTriggerTimer(seconds);
        }
    }

    public class StimuliTriggerEvent : IStimuliTrigger
    {

        EventList list;
        event WaitForNextStimuli OnWaitForStimuliSlide;
        uint number, counter;
        uint Counter
        {
            get { return counter; }
            set
            {
                counter = value;
                TriggerInfoProperty = getInfo();
            }
        }

        public StimuliTriggerEvent(EventList list, uint x)
        {
            this.list = list;
            this.number = x;
            Counter = x;
        }

        public override string getInfo()
        {
            return "Waiting for " + counter + " more events..";
        }

        public override void wait(WaitForNextStimuli func)
        {
            list.CollectionChanged += OnEvent;
            OnWaitForStimuliSlide += func;
        }

        public override void interrupt()
        {
            list.CollectionChanged -= OnEvent;
            Counter = 1;
            OnEvent(null, null);
        }

        void OnEvent(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            if (e == null || e.NewItems == null)
            {
                return;
            }

            if (((EventItem)e.NewItems[0]).Change == false)
            {
                // ignore events not meant to change the slides
                return;
            }

            if (--Counter == 0)
            {
                list.CollectionChanged -= OnEvent;
                OnWaitForStimuliSlide();
                OnWaitForStimuliSlide = null;
                Counter = number;
            }
        }

        public override StimuliTriggerType getType()
        {
            return StimuliTriggerType.Event;
        }

        public override void save(XmlWriter xml)
        {
            xml.WriteStartAttribute("number");
            xml.WriteString(number.ToString());
            xml.WriteEndAttribute();
        }

        public static IStimuliTrigger Load(XmlReader xml, EventList list)
        {
            uint x = uint.Parse(xml.GetAttribute("number"));
            return new StimuliTriggerEvent(list, x);
        }
    }
}
