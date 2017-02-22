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
using System.IO;

namespace ssi
{
    public partial class SessionControl : UserControl
    {
        string path = "";
        Collection<SelectItem> pathItems = new Collection<SelectItem>();
        public string Path
        {
            get { return path; }
            set {
                pathItems.Clear();
                if (value != "")
                {
                    pathItems.Add(new SelectItem(value, true));                    
                }
                user.SetItems(pathItems);
                path = value;
            }
        }        
       
        public SessionControl()
        {
            InitializeComponent();

            user.SelectionChanged += date.OnSelectionChanged;
            date.SelectionChanged += data.OnSelectionChanged;
            date.SelectionChanged += anno.OnSelectionChanged;
            date.SelectionChanged += samp.OnSelectionChanged;

            UseMultiSelection = true;
            UseMultiFilter = true;
        }

        bool useMultiSelection;
        public bool UseMultiSelection
        {
            get { return useMultiSelection; }
            set
            {
                user.SelectionMode = date.SelectionMode = anno.SelectionMode = data.SelectionMode = samp.SelectionMode = value ? SelectionMode.Extended : SelectionMode.Single;
                useMultiSelection = value; 
            }
        }

        bool useMultiFilter;
        public bool UseMultiFilter
        {
            get { return useMultiFilter; }
            set
            {
                anno.MultipleFilter = data.MultipleFilter = samp.MultipleFilter = value;
                useMultiFilter = value;
            }
        }                     
    }
}
