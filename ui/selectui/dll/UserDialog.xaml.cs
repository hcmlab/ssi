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
    /// Interaction logic for UserDialog.xaml
    /// </summary>
    public partial class UserDialog : Window
    {

        bool success;
        public bool Success
        {
            get { return success; }
        }

        string name;
        public string Name
        {
            get { return name; }
        }

        public UserDialog()
        {
            InitializeComponent();

            success = false;                        
        }

        private void CreateClick(object sender, RoutedEventArgs e)
        {
            name = nameText.Text;

            if (name != "")
            {
                success = true;
                Close();
            }
            else
            {
                MessageBox.Show("ERROR: cannot create user");
            }
        }

        private void CancelClick(object sender, RoutedEventArgs e)
        {            
            Close();
        }
    }
}
