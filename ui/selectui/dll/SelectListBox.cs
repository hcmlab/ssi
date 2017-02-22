using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Text.RegularExpressions;

namespace ssi
{
    public delegate void SourceItemsChanged(Collection<SelectItem> items);

    public enum SelectListBoxType
    {
        FILES = 0,
        DIRECTORIES
    }

    public class SelectListBox : ListBox
    {        
        ObservableCollection<string> listItems = new ObservableCollection<string>();
        Collection<SelectItem> realItems = new Collection<SelectItem>();

        public static readonly DependencyProperty TypeProperty = DependencyProperty.Register("Type", typeof(SelectListBoxType), typeof(SelectListBox), new PropertyMetadata(SelectListBoxType.FILES));
        public SelectListBoxType Type
        {
            get { return (SelectListBoxType)this.GetValue(TypeProperty); }
            set { this.SetValue(TypeProperty, value); }
        }

        public static readonly DependencyProperty MultipleFilterProperty = DependencyProperty.Register("MultipleFilter", typeof(Boolean), typeof(SelectListBox), new PropertyMetadata(false));
        public Boolean MultipleFilter
        {
            get { return (Boolean)this.GetValue(MultipleFilterProperty); }
            set { this.SetValue(MultipleFilterProperty, value); }
        }

        public static readonly DependencyProperty RegexFilterProperty = DependencyProperty.Register("RegexFilter", typeof(String), typeof(SelectListBox), new PropertyMetadata(""));
        public String RegexFilter
        {
            get { return (String)this.GetValue(RegexFilterProperty); }
            set { this.SetValue(RegexFilterProperty, value); }
        }

        public SelectListBox()
        {
            this.ItemsSource = listItems;
        }

        public void OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            SelectListBox from = (SelectListBox)sender;
            Collection<SelectItem> selected = new Collection<SelectItem> ();
            from.AddSelectedItems(ref selected);
            this.SetItems(selected);
        }

        public void SetItems(Collection<SelectItem> items)
        {            
            this.realItems.Clear();

            if (items.Count > 0)
            {

                Dictionary<string, uint> dict = null;                

                if (MultipleFilter)
                {
                    dict = new Dictionary<string, uint>();
                    string[] candidates = null;
                    switch (Type)
                    {
                        case SelectListBoxType.FILES:
                            candidates = Directory.GetFiles(items[0].Path);
                            break;
                        case SelectListBoxType.DIRECTORIES:
                            candidates = Directory.GetDirectories(items[0].Path);
                            break;
                    }
                    foreach (string candidate in candidates)
                    {
                        SelectItem item = new SelectItem(candidate, Type == SelectListBoxType.DIRECTORIES);
                        dict.Add(item.FullName, 0);
                    }
                    foreach (SelectItem item in items)
                    {
                        string[] files = null;

                        switch (Type)
                        {
                            case SelectListBoxType.FILES:
                                files = Directory.GetFiles(item.Path);
                                break;
                            case SelectListBoxType.DIRECTORIES:
                                files = Directory.GetDirectories(item.Path);
                                break;
                        }

                        foreach (string file in files)
                        {
                            SelectItem candidate = new SelectItem(file, Type == SelectListBoxType.DIRECTORIES);
                            string key = candidate.FullName;
                            if (dict.ContainsKey(key))
                            {
                                dict[key]++;
                            }
                        }
                    }
                }

                Regex regex = null;

                if (RegexFilter != "") 
                {
                    regex = new Regex (RegexFilter);
                }

                uint count = (uint)items.Count;
                foreach (SelectItem item in items)
                {
                    string[] files = null;

                    switch (Type)
                    {
                        case SelectListBoxType.FILES:
                            files = Directory.GetFiles(item.Path);
                            break;
                        case SelectListBoxType.DIRECTORIES:
                            files = Directory.GetDirectories(item.Path);
                            break;
                    }

                    foreach (string file in files)
                    {
                        SelectItem candidate = new SelectItem(file, Type == SelectListBoxType.DIRECTORIES);
                        if (!MultipleFilter || (dict.ContainsKey (candidate.FullName) && dict[candidate.FullName] == count))
                        {
                            if (regex == null || regex.IsMatch (candidate.FullName)) {
                                this.realItems.Add(candidate);
                            }
                        }
                    }
                }
            }

            displayItems();
        }

        void displayItems()
        {
            this.listItems.Clear();

            foreach (SelectItem item in this.realItems)
            {
                if (!MultipleFilter || !this.listItems.Contains(item.FullName))
                {
                    this.listItems.Add(item.FullName);
                }
            }

        }

        public void AddSelectedItems(ref Collection<SelectItem> list)
        {
            foreach (SelectItem item in realItems)
            {
                if (SelectedItems.Contains(item.FullName))
                {
                    list.Add(item);
                }
            }            
        }
    }
}
