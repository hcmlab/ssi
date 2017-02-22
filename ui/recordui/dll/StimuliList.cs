using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Windows.Controls.Primitives;
using System.Windows;
using System.Windows.Controls;

namespace ssi
{
    public delegate void NextStimuliSlide(StimuliSlide slide);
    public delegate void WaitForNextStimuli();
    public delegate void WaitForLastStimuli();

    public class StimuliList : ObservableCollection<StimuliSlide>
    {
        public NextStimuliSlide OnNextStimuliSlide;
        public event WaitForLastStimuli OnWaitForLastStimuli;
        bool onSlideShow;
        IEnumerator<StimuliSlide> slideShow = null;

        public string getCurrentLabel()
        {
            if (slideShow != null && slideShow.Current != null)
            {
                return slideShow.Current.Label;
            }
            return "";
        }

        public StimuliList () 
        : base () {

            onSlideShow = false;
        }

        public void show(StimuliSlide slide)
        {
            if (OnNextStimuliSlide != null)
            {
                OnNextStimuliSlide(slide);
            }            
        }

        public void interruptSlideShow()
        {
            if (onSlideShow)
            {
                onSlideShow = false;
                slideShow.Current.Trigger.interrupt ();
                OnWaitForLastStimuli();
            }
        }

        public void startSlideShow()
        {
            if (!onSlideShow)
            {
                onSlideShow = true;
                slideShow = this.GetEnumerator();
                nextSlideShow();
            }
        }

        void stopSlideShow()
        {
            if (onSlideShow)
            {
                onSlideShow = false;
                OnWaitForLastStimuli();
            }
        }
    
        void nextSlideShow () 
        {
            if (slideShow.MoveNext())
            {
                StimuliSlide slide = slideShow.Current;
                show(slide);
                slide.Trigger.wait(SlideShowWait);
            }
            else
            {
                stopSlideShow();
            }            
        }

        public void SlideShowWait()
        {
            if (onSlideShow)
            {
                nextSlideShow();
            }
        }        
    }
}
