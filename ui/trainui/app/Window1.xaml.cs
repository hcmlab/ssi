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
        SelectHandler selecth;
        TrainHandler trainh;

        GridLength previousExpandLength = new GridLength();

        public Window1()
        {
            InitializeComponent();

            this.select.OnHandlerLoaded += selectHandlerLoaded;
            this.train.OnHandlerLoaded += trainHandlerLoaded;
        }

        void selectHandlerLoaded(SelectHandler handler)
        {
            selecth = handler;
            if (trainh != null)
            {
                handlerLoaded();
            }
        }

        void trainHandlerLoaded(TrainHandler handler)
        {
            trainh = handler;
            if (selecth != null)
            {
                handlerLoaded();
            }
        }

        void handlerLoaded()
        {
            //selecth.OnSignalSelectionChanged += trainh.OnSignalSelectionChanged;
            selecth.OnAnnotationSelectionChanged += trainh.OnAnnotationSelectionChanged;
            selecth.OnDateSelectionChanged += trainh.OnDateSelectionChanged;
            selecth.OnProjectSelectionChanged += trainh.OnProjectSelectionChanged;
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
