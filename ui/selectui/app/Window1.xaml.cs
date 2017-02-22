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

        public Window1()
        {
            InitializeComponent();

            this.select.OnHandlerLoaded += selectHandlerLoaded;
        }

        private void selectHandlerLoaded(SelectHandler handler)
        {
            selecth = handler;
            this.useMultiSelect.SetBinding(CheckBox.IsCheckedProperty, this.selecth.useMultiSelectBinding);
            this.useMultiFilter.SetBinding(CheckBox.IsCheckedProperty, this.selecth.useMultiFilterBinding);
            this.useRecycleBin.SetBinding(CheckBox.IsCheckedProperty, this.selecth.useRecycleBinBinding);
            this.useSimpleInterface.SetBinding(CheckBox.IsCheckedProperty, this.selecth.useSimpleInterfaceBinding);
        }

    }
}
