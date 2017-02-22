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
using System.Collections.ObjectModel;

namespace ssi
{
    public partial class Window1 : Window
    {
        RecordHandler recordh;
        SelectHandler selecth;

        GridLength previousExpandLength = new GridLength();

        public Window1()
        {
            InitializeComponent();            

            this.record.OnHandlerLoaded += recordHandlerLoaded;
            this.select.OnHandlerLoaded += selectHandlerLoaded;              
        }

        void recordHandlerLoaded(RecordHandler handler)
        {
            this.recordh = handler;
            if (this.selecth != null)
            {
                handlerLoaded();
            }
        }

        void selectHandlerLoaded(SelectHandler handler)
        {
            this.selecth = handler;
            this.selecth.UseMultiFilterProperty = true;
            this.selecth.UseMultiSelectProperty = false;

            if (this.recordh != null)
            {
                handlerLoaded();
            }
        }

        void handlerLoaded()
        {
            selecth.OnProjectSelectionChanged += recordh.OnProjectSelectionChanged;
            selecth.OnUserSelectionChanged += recordh.OnUserSelectionChanged;
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
