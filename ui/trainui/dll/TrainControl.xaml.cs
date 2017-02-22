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
    public delegate void HandlerLoaded(TrainHandler handler);

    public partial class TrainControl : UserControl
    {
        TrainHandler trainh = null;
        GridLength previousExpandLength = new GridLength();

        public event HandlerLoaded OnHandlerLoaded;

        public TrainControl()
        {
            InitializeComponent();
            System.Threading.Thread.CurrentThread.CurrentCulture = new System.Globalization.CultureInfo("en-US");
            this.Dispatcher.ShutdownStarted += ControlUnloaded;

        }

        private void ControlLoaded(object sender, RoutedEventArgs e)
        {
            if (this.ActualWidth > 0 && this.trainh == null)
            {
                this.trainh = new TrainHandler(this);
                if (OnHandlerLoaded != null)
                {
                    OnHandlerLoaded(this.trainh);
                }
            }
        }

        private void ControlUnloaded(object sender, EventArgs e)
        {
            if (this.trainh != null)
            {
                this.trainh.Unload();
            }
        }
    }
}
