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
using System.Text.RegularExpressions;
using System.Globalization;

namespace ssi
{    
    [ValueConversion(typeof(String), typeof(Boolean))]
    public class ValueCheckBoxConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            String from = (String)value;
            Boolean to = true;

            try
            {
                to = Boolean.Parse(from);
            } catch (Exception ex) {
                System.Console.Out.WriteLine(ex.ToString());
            }

            return to;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {            
            return ((Boolean) value).ToString ();
        }
    }

    [ValueConversion(typeof(String), typeof(string[]))]
    public class ValueComboBoxConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            String from = (String)value;
            return from.Split(Define.CHAR_META_DELIM);                
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }
    }

    public class ValueTemplateSelector : DataTemplateSelector
    {
        public DataTemplate TextBoxTemplate { get; set; }
        public DataTemplate ComboBoxTemplate { get; set; }
        public DataTemplate CheckBoxTemplate { get; set; }

        public override DataTemplate SelectTemplate(object item,
          DependencyObject container)
        {
            Config.Item i = (Config.Item)item;            

            switch (i.ValueType)
            {
                case Config.Item.Type.TEXT:
                    return TextBoxTemplate;
                case Config.Item.Type.BOOLEAN:
                    return CheckBoxTemplate;
                case Config.Item.Type.SELECTION:
                    return ComboBoxTemplate;
            }

            return null;
        }
    }

    public partial class ConfigControl : UserControl
    {
    
        public ConfigControl()
        {
            InitializeComponent();   
        }

        public void Update(Config config)
        {
            listView.Items.Clear();
            foreach (Config.Item item in config)
            {
                if (item.ValueType != Config.Item.Type.NONE)
                {                    
                    listView.Items.Add(item);
                }
            }
        }

        public delegate void ItemDoubleClickHandler(Config.Item item);
        public event ItemDoubleClickHandler ItemDoubleClick;

        private void ListView_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {          
            DependencyObject dep = (DependencyObject)e.OriginalSource;
            while ((dep != null) && !(dep is ListViewItem))
            {
                dep = VisualTreeHelper.GetParent(dep);
            }
            if (dep == null)
            {
                return;
            }
            Config.Item item = (Config.Item)listView.ItemContainerGenerator.ItemFromContainer(dep);

            if (ItemDoubleClick != null)
            {
                ItemDoubleClick(item);
            }
        }

        private void ListViewItem_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            DependencyObject dep = (DependencyObject)e.OriginalSource;
            while ((dep != null) && !(dep is ListViewItem))
            {
                dep = VisualTreeHelper.GetParent(dep);
            }
            if (dep == null)
            {
                return;
            }
            Config.Item item = (Config.Item)listView.ItemContainerGenerator.ItemFromContainer(dep);

            int index = listView.Items.IndexOf(item);
            listView.SelectedIndex = index;
            System.Console.WriteLine(index);
        }
    }
}
