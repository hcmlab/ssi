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
    public delegate void HandlerLoaded (RecordHandler handler);

    public partial class RecordControl : UserControl
    {
        RecordHandler handler = null;
        GridLength previousExpandLength = new GridLength();

        public event HandlerLoaded OnHandlerLoaded;

        public RecordControl()
        {
            InitializeComponent();
            System.Threading.Thread.CurrentThread.CurrentCulture = new System.Globalization.CultureInfo("en-US");
            this.Dispatcher.ShutdownStarted += ControlUnloaded;
        }

        private void ControlLoaded(object sender, RoutedEventArgs e)
        {
            if (this.ActualWidth > 0 && this.handler == null)
            {                
                this.handler = new RecordHandler(this);
                if (OnHandlerLoaded != null)
                {
                    OnHandlerLoaded(this.handler);
                }
            }
        }

        private void ControlUnloaded(object sender, EventArgs e)
        {
            if (this.handler != null)
            {
                this.handler.Unload();
            }
        }

        private void Expander_Expanded(object sender, RoutedEventArgs e)
        {
            myGrid.ColumnDefinitions[2].Width = previousExpandLength;
        }

        private void Expander_Collapsed(object sender, RoutedEventArgs e)
        {
            previousExpandLength = myGrid.ColumnDefinitions[2].Width;
            myGrid.ColumnDefinitions[2].Width = GridLength.Auto;
        }

        private void stimuliComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {            
            if (stimuliComboBox.SelectedIndex >= 0)
            {
                RecordHandler.LoadStimuli(handler, stimuliComboBox.SelectedValue.ToString());
            }
        }
    }
}
