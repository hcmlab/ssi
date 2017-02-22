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
using System.Windows.Shapes;

namespace ssi
{
    /// <summary>
    /// Interaction logic for LabelInputBox.xaml
    /// </summary>
    public partial class LabelInputBox : Window
    {
        public LabelInputBox(String header, String info, String text)
        {
            InitializeComponent();
            this.Title = header;
            this.ib_label.Content = info;
            this.ib_labelText.Text = text;
        }

        private void ib_ok_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();            
        }

        private void ib_cancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        public string Result ()
        {
            return this.ib_labelText.Text;
        }
    }
}
