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
    /// Interaction logic for WaitDialog.xaml
    /// </summary>
    public partial class WaitDialog : Window
    {
        bool canceled = true;
        public bool Canceled
        {
            get { return canceled; }
            set { canceled = value; }
        }

        public WaitDialog()
        {
            InitializeComponent();
        }

        private void ButtonCanel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
