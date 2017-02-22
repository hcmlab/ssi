using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using SSIXmlEditor.App;
using System.Windows.Input;

namespace SSIXmlEditor
{
    public partial class DllSelectForm : Form
    {
        public SingleInstanceApp m_app;
        private List<string> m_lastSelected;

        public DllSelectForm(App.SingleInstanceApp app)
        {
            InitializeComponent();

            m_app = app;
            LoadSelection();

            List<string> dlls = getDlls();
            foreach (string s in dlls)
            {
                if (m_lastSelected.Contains(s))
                {
                    SelectedListBox.Items.Add(s);
                }
                else
                {
                    AvailableListBox.Items.Add(s);
                }
            }

            AvailableListBox.DoubleClick += SelectButton_Click;
            AvailableListBox.KeyPress += (sender, args) =>
            {
                if (args.KeyChar == (char)Keys.Space || args.KeyChar == (char)Keys.Enter) {
                    MoveListBoxItems(AvailableListBox, SelectedListBox);
                }  
            };

            SelectedListBox.DoubleClick += DeselectButton_Click;
            SelectedListBox.KeyPress += (sender, args) =>
            {
                if (args.KeyChar == (char)Keys.Space || args.KeyChar == (char)Keys.Enter) {
                    MoveListBoxItems(SelectedListBox, AvailableListBox);
                }
            };


        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            foreach (string s in SelectedListBox.Items)
            {
                m_app.DllNames.Add(s);
            }
            SaveSelection();

            DialogResult = DialogResult.OK;

            Close();
        }

        private List<string> getDlls()
        {
            List<string> dlls = new List<string>();
            string search_path = Properties.Settings.Default.Path;
            string[] files = System.IO.Directory.GetFiles(search_path, "ssi*.dll");
            foreach (var fi in files)
            {
#if DEBUG
                if (!fi.EndsWith("d.dll"))
                    continue;
#else
                if (fi.EndsWith("d.dll"))
					continue;
#endif                
                dlls.Add(fi);
            }

            return dlls;
        }

        private void SelectButton_Click(object sender, EventArgs e)
        {            
            MoveListBoxItems(AvailableListBox, SelectedListBox);
        }

        private void DeselectButton_Click(object sender, EventArgs e)
        {
            MoveListBoxItems(SelectedListBox, AvailableListBox);
        }

        private void MoveListBoxItems(ListBox source, ListBox destination)
        {
            ListBox.SelectedObjectCollection sourceItems = source.SelectedItems;
            foreach (var item in sourceItems)
            {
                destination.Items.Add(item);
            }
            while (source.SelectedItems.Count > 0)
            {
                source.Items.Remove(source.SelectedItems[0]);
            }
        }

        private void LoadSelection()
        {
            m_lastSelected = new List<string>();
            
            try
            {
                List<string> lines = new List<string>();
#if DEBUG
                using (StreamReader file = new StreamReader(Properties.Settings.Default.Path + "\\xmleditd_dlls.txt"))
#else
                using (StreamReader file = new StreamReader(Properties.Settings.Default.Path + "\\xmledit_dlls.txt"))
#endif
                {
                    string line;
                    while ((line = file.ReadLine()) != null)
                    {
                        m_lastSelected.Add(line);
                    }
                }
            }
            catch (Exception e)
            {
            }
        }

        private void SaveSelection()
        {
            try
            {
#if DEBUG
               using (StreamWriter file = new StreamWriter(Properties.Settings.Default.Path + "\\xmleditd_dlls.txt"))
#else
               using (StreamWriter file = new StreamWriter(Properties.Settings.Default.Path + "\\xmledit_dlls.txt"))
#endif
                {
                    foreach (var item in SelectedListBox.Items)
                    {
                        file.WriteLine(item.ToString());
                    }
                }                
            }
            catch (Exception e)
            {
            }
        }

        private void CancelButton_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}
