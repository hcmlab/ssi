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
    public partial class Window1 : Window
    {
        GridLength previousExpandLength = new GridLength();

        AnnoHandler annoh;
        SelectHandler selecth;

        public Window1()
        {
            InitializeComponent();

            this.select.OnHandlerLoaded += selectHandlerLoaded;
            this.anno.OnHandlerLoaded += annoHandlerLoaded;
        }

        void annoHandlerLoaded(AnnoHandler handler)
        {
            annoh = handler;
            if (selecth != null)
            {
                handlerLoaded();
            }
        }

        void selectHandlerLoaded(SelectHandler handler)
        {
            selecth = handler;
            selecth.DisplayAnnotations(true);
            selecth.DisplaySignals(true);
            selecth.DisplaySamples(false);
            if (annoh != null)
            {
                handlerLoaded();
            }
        }

        void handlerLoaded()
        {
            selecth.OnProjectSelectionChanged += annoh.OnProjectSelectionChanged;
            selecth.OnSignalSelectionChanged += annoh.OnSignalSelectionChanged;
            selecth.OnAnnotationSelectionChanged += annoh.OnAnnotationSelectionChanged;
        }

        private void Expander_Expanded(object sender, RoutedEventArgs e)
        {
            myGrid.RowDefinitions[0].Height = previousExpandLength;
        }

        private void Expander_Collapsed(object sender, RoutedEventArgs e)
        {
            previousExpandLength = myGrid.RowDefinitions[0].Height;
            myGrid.RowDefinitions[0].Height = GridLength.Auto;
        }
    }
}
