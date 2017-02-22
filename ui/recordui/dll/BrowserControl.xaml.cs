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

namespace ssi
{
    /// <summary>
    /// Interaction logic for BrowserControl.xaml
    /// </summary>
    public partial class BrowserControl : UserControl
    {
        IStimuliSource defaultSource;
        public IStimuliSource DefaultSource
        {
            get { return defaultSource; }
            set { defaultSource = value; }
        }

        public BrowserControl()
        {
            InitializeComponent();      
      
            //defaultSource = new StimuliSourceBlank();
            defaultSource = new StimuliSourceBlank ();
            OnUpdateLastStimuli();
        }

        public void OnUpdateStimuliSlide(StimuliSlide slide)
        {
            slide.Source.show(browser);
            statusName.Text = slide.SourceType;
            statusType.Text = slide.SlideType;
            statusInfo.SetBinding(TextBlock.TextProperty, slide.Trigger.TriggerInfoBinding);
        }


        public void OnUpdateLastStimuli()
        {
            defaultSource.show(browser);
        }
    }
}
