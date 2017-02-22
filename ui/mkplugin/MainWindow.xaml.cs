using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;

namespace ssi
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Project project;       

        public MainWindow()
        {
            Thread.CurrentThread.CurrentCulture = CultureInfo.CreateSpecificCulture("en-us");

            InitializeComponent();

            Closing += OnClosing;

            project = new Project();
            DataContext = project;
        }

        void OnClosing(object sender, CancelEventArgs e)
        {
            MessageBoxResult result = MessageBox.Show("Quit application?", "Question", MessageBoxButton.OKCancel, MessageBoxImage.Question);
            if (result == MessageBoxResult.Cancel)
            {
                e.Cancel = true;
            }
        }

        private void CancelClick(object sender, RoutedEventArgs e)
        {
            Close();               
        }

        private void CreateClick(object sender, RoutedEventArgs e)
        {
            if (!project.Check())
            {
                MessageBox.Show("Invalid project settings!", "Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            
            Worker worker = new Worker (project);
            worker.Run();
            File.WriteAllLines("mkplugin.log", worker.Log.Cast<string>());

            MessageBox.Show("Created project " + project.Name + " in " + project.Target + Path.DirectorySeparatorChar, "Message", MessageBoxButton.OK, MessageBoxImage.Information);
            
            project.Reset();
        }


    }

}
