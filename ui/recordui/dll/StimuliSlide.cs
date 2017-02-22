using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.ComponentModel;
using System.Windows.Data;
using System.Windows.Threading;
using System.Xml;

namespace ssi
{
    public class StimuliSlide
    {
        IStimuliSource source;
        public IStimuliSource Source
        {
            get { return source; }
        }

        IStimuliTrigger trigger;
        public IStimuliTrigger Trigger
        {
            get { return trigger; }
        }

        string label;
        public string Label
        {
            get { return label; }
        }

        public string SourceType
        {
            get { return source.Type.ToString (); }
        }

        public string SourceInfo
        {
            get { return source.Info; }
        }

        public string SlideType
        {
            get { return trigger.getType ().ToString (); }
        }

        public StimuliSlide(IStimuliSource source, IStimuliTrigger trigger, string label)
        {
            this.source = source;
            this.trigger = trigger;
            this.label = label;            
        }

    }
}
