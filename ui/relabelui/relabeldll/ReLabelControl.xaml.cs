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
using System.IO;
using System.Text.RegularExpressions;

namespace ssi
{
    public delegate void HandlerLoaded(ReLabelHandler handler);

    public partial class ReLabelControl : UserControl
    {
        ReLabelHandler relabelh = null;

        public event HandlerLoaded OnHandlerLoaded;

        public ReLabelControl()
        {
            InitializeComponent();
            System.Threading.Thread.CurrentThread.CurrentCulture = new System.Globalization.CultureInfo("en-US");
            this.Dispatcher.ShutdownStarted += ControlUnloaded;
        }

        private void ControlLoaded(object sender, RoutedEventArgs e)
        {
            if (this.ActualWidth > 0 && this.relabelh == null)
            {
                this.relabelh = new ReLabelHandler(this);
                if (OnHandlerLoaded != null)
                {
                    OnHandlerLoaded(this.relabelh);
                }
            }
        }

        private void ControlUnloaded(object sender, EventArgs e)
        {
            if (this.relabelh != null)
            {
                this.relabelh.Unload();
            }
        }
    }
}
