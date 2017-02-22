using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
//using System.Windows;

namespace ssi
{
    public class Error
    {
        public static void Show(string msg)
        {
            //MessageBox.Show(msg, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            MessageBox.Show(msg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }
}
