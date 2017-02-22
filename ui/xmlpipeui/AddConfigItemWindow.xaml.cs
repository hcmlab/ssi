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
    /// Interaction logic for AddConfigItemWindow.xaml
    /// </summary>
    public partial class AddConfigItemWindow : Window
    {
        public AddConfigItemWindow(Config.Item item)
        {
            InitializeComponent();

            Array types = Enum.GetValues(typeof(Config.Item.Type));
            for (int i = 1; i < types.Length; i++)
            {
                valueTypeTextBox.Items.Add(types.GetValue (i));
            }
            if (item.ValueType == Config.Item.Type.NONE)
            {
                item.ValueType = Config.Item.Type.TEXT;
            }

            keyTextBox.DataContext = item;
            valueTextBox.DataContext = item;
            valueTypeTextBox.DataContext = item;
            valueOptionsTextBox.DataContext = item;
            commentTextBox.DataContext = item;

            keyTextBox.GotFocus += new RoutedEventHandler(keyTextBox_GotFocus);

            doneButton.Click += new RoutedEventHandler(doneButton_Click);
            cancelButton.Click += new RoutedEventHandler(cancelButton_Click);
        }

        void keyTextBox_GotFocus(object sender, RoutedEventArgs e)
        {
            keyTextBox.Background = SystemColors.ControlBrush;
        }

        void cancelButton_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        void doneButton_Click(object sender, RoutedEventArgs e)
        {
            if (keyTextBox.Text == "")
            {
                keyTextBox.Background = Brushes.Red;
                return;
            }
            
            DialogResult = true;
            Close();
        }
    }
}
