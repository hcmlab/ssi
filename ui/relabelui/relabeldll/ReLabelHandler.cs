using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows;
using System.IO;
using System.Windows.Data;
using System.Collections.ObjectModel;
using System.Collections;
using System.ComponentModel;
using System.Diagnostics;
using System.Windows.Media;
using System.Windows.Input;
using System.Windows.Threading;
using System.Text.RegularExpressions;

namespace ssi
{
    public class ReLabelHandler
    {
        ReLabelControl control;

        SelectItem annotation = null;
        Collection<SelectItem> dates = null;
        ReLabelList labelSet = new ReLabelList ();
        string newName = "";

        Project project;
        public Project Project
        {
            get { return project; }
            set
            {                
                project = value;
            }
        }

        public ReLabelHandler(ReLabelControl control)
        {
            this.control = control;

            this.control.newNameTextBox.TextChanged += newNameTextBox_TextChanged;
            this.control.importButton.Click += importButton_Click;
            this.control.okButton.Click += okButton_Click;
            
            this.control.table.list.ItemsSource = labelSet;

            checkSelection();
        }

        void newNameTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            this.newName = ((TextBox)sender).Text;
            checkSelection();
        }

        public void Unload ()
        {
        }

        #region SelectionChanged

        public void OnProjectSelectionChanged(Project project)
        {
            Project = project;
            checkSelection();
        }

        bool checkSelection()
        {
            bool result = annotation != null && dates != null && newName != "" && annotation.Name != newName && labelSet.Count > 0;

            if (annotation != null)
            {
                control.annoLabel.Foreground = Brushes.Green;
                control.annoLabel.Content = annotation.Name;
            }
            else
            {
                control.annoLabel.Foreground = Brushes.Red;
                control.annoLabel.Content = "please select";
            }

            control.importButton.IsEnabled = annotation != null && dates != null;
            control.okButton.IsEnabled = result;            
            
            return result;
        }

        public void OnDateSelectionChanged(Collection<SelectItem> items)
        {
            if (items.Count > 0)
            {
                dates = items;
            }
            else
            {
                dates = null;
            }
            checkSelection();
        }

        public void OnAnnotationSelectionChanged(Collection<SelectItem> items)
        {
            if (items.Count > 0)
            {
                annotation = items[0];
            }
            else
            {
                annotation = null;
            }
            checkSelection();
        }

        #endregion

        #region Buttons

        private void importButton_Click(object sender, RoutedEventArgs e)
        {
            labelSet.Clear();

            HashSet<ReLabelItem> set = new HashSet<ReLabelItem>(new ReLabelListEqualityComparer());
            foreach (SelectItem date in dates)
            {
                string annopath = date.Path + "\\" + annotation.FullName;
                importLabel(annopath, set);               
            }

            foreach (ReLabelItem item in set)
            {
                labelSet.Add(item);
            }

            checkSelection();
        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            Hashtable table = new Hashtable();

            foreach (ReLabelItem item in labelSet)
            {
                System.Console.WriteLine(item.OldLabel + "->" + item.NewLabel);
                table.Add(item.OldLabel, item.NewLabel);               
            }

            foreach (SelectItem date in dates)
            {
                string annopath_in = date.Path + "\\" + annotation.FullName;
                string annopath_out = date.Path + "\\" + newName + ".anno";
                reLabel(annopath_in, annopath_out, table);
            }

            labelSet.Clear();
            checkSelection();
        }

        #endregion

        #region Label

        public void importLabel(String filepath, HashSet<ReLabelItem> set)
        {           
            try
            {
                StreamReader sr = new StreamReader(filepath, System.Text.Encoding.Default);
                string pattern = "^([0-9]+.[0-9]+|[0-9]+) ([0-9]+.[0-9]+|[0-9]+) .+";
                Regex reg = new Regex(pattern);
                string line = null;
                while ((line = sr.ReadLine()) != null)
                {
                    if (reg.IsMatch(line))
                    {
                        string[] data = line.Split(' ');
                        string label = data[2];
                        for (int i = 3; i < data.Length; i++)
                        {
                            label += " " + data[i];
                        }

                        set.Add(new ReLabelItem (label));
                    }
                }
                sr.Close();
            }
            catch (Exception e)
            {
                MessageBox.Show("ERROR: Can't read annotation file: " + filepath);
            }
        }

        public void reLabel(String filepath_in, String filepath_out, Hashtable table)
        {
            try
            {
                StreamReader sr = new StreamReader(filepath_in, System.Text.Encoding.Default);
                StreamWriter sw = new StreamWriter(filepath_out, false, System.Text.Encoding.Default);

                string pattern = "^([0-9]+.[0-9]+|[0-9]+) ([0-9]+.[0-9]+|[0-9]+) .+";
                Regex reg = new Regex(pattern);
                string line = null;
                while ((line = sr.ReadLine()) != null)
                {
                    if (reg.IsMatch(line))
                    {
                        string[] data = line.Split(' ');
                        double from = Convert.ToDouble(data[0]);
                        double to = Convert.ToDouble(data[1]);
                        string label = data[2];
                                                
                        string new_label = (string) table[label];
                        sw.WriteLine (from + " " + to + " " + new_label);
                    }
                }
                sr.Close();
                sw.Close();
            }
            catch (Exception e)
            {
                MessageBox.Show ("ERROR: Can't read annotation file: " + filepath_in);
            }
        }

        #endregion

    }


}
