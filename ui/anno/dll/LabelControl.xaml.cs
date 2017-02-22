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
    public delegate void LabelSelected (string label);

    public partial class LabelControl : UserControl
    {
        public event LabelSelected OnLabelSelected;

        public LabelControl()
        {
            InitializeComponent();

            addLabel("negative-high");
            addLabel("negative-low");
            //addLabel("neutral");
            addLabel("positive-low");
            addLabel("positive-high");
        }

        public void clear()
        {
            this.labelGrid.RowDefinitions.Clear();
            this.labelGrid.Children.Clear();
        }

        public void addLabel(string label)
        {
            // video
            RowDefinition row = new RowDefinition();
            row.Height = new GridLength(1, GridUnitType.Star);            
            labelGrid.RowDefinitions.Add(row);

            Button button = new Button();
            button.Margin = new Thickness(5);
            button.Content = label;
            button.Click += onClick;
            Grid.SetColumn(button, 0);
            Grid.SetRow(button, labelGrid.RowDefinitions.Count - 1);
            labelGrid.Children.Add(button);        
    
            
        }

        void onClick(object sender, RoutedEventArgs e)
        {
            if (OnLabelSelected != null) 
            {
                OnLabelSelected (((Button) sender).Content.ToString ());
            }

        }
        
    }
}
