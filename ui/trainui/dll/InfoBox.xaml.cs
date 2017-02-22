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
using Microsoft.Win32;

namespace ssi
{
    public partial class InfoBox : UserControl
    {
        private String data = null;
        private ConfMatrix cm = null;
        private String root = null;

        public InfoBox ()
        {
            InitializeComponent();                       
        }

        public void SetInfo(String root, String data)
        {
            this.root = root;
            this.info_textbox.Text = data;
            this.data = data;

            try
            {
                this.cm = ConfMatrix.FromString(data);
            }
            catch
            {
                MessageBox.Show("ERROR: while parsing evaluation results");
            }
        }

        private void info_saveButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                String path = saveFileDialog(root, ".txt", "Save evaluation results");
                if (path != null)
                {
                    System.IO.File.WriteAllText(path, cm.ToString());
                }
            }
            catch 
            {
                MessageBox.Show("ERROR: while saving evaluation results");
            }
        }

        private void info_copyButton_Click(object sender, RoutedEventArgs e)
        {
            if (cm != null)
            {
                Clipboard.SetText(cm.ToString());
            }
        }

        static String saveFileDialog(String rootfolder, String filetype, String description)
        {
            String path;
            SaveFileDialog save = new SaveFileDialog();

            save.InitialDirectory = rootfolder;
            save.Title = description;
            save.Filter = "Textfile|*.txt";

            if (save.ShowDialog().Value)
            {
                path = save.FileName;
                return path;
            }
            else
            {
                return null;
            }
        }
    }

    public class ConfMatrix
    {
        public uint n_classes = 0;
        public string[] class_name = null;
        public uint[,] class_conf = null;
        public double[] class_prob = null;
        public double overall_prob = 0;

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();

            for (int i = 0; i < n_classes; i++) 
            {
                sb.Append(class_name[i] + ":\t");
                for (int j = 0; j < n_classes; j++)
                {
                    sb.Append(class_conf[i, j] + "\t");
                }
                sb.Append("->\t" + class_prob[i] + "%\n");
            }

            for (int i = 0; i < n_classes + 1; i++) 
            {
                sb.Append ("\t");
            }
            sb.Append("=>" + "\t" + overall_prob + "%\n");

            return sb.ToString();
        }

        public static ConfMatrix FromString (string data) 
        {
            ConfMatrix cm = new ConfMatrix();

            data = data.Replace("\r", "");
            string[] lines = data.Split ('\n');            

            string line_classes = lines[0];
            uint n_classes = uint.Parse(line_classes.Substring(line_classes.IndexOf("#classes:") + 9));
            cm.n_classes = n_classes;

            cm.class_name = new string[n_classes];
            cm.class_conf = new uint[n_classes, n_classes];
            cm.class_prob = new double[n_classes];

            string[] delims = { ":", "=>", "->", " ", "\t", "%", "=" };
            for (int i = 0; i < n_classes; i++)
            {
                string line = lines[i+5];
                string[] tokens = line.Split(delims, StringSplitOptions.RemoveEmptyEntries);
                cm.class_name[i] = tokens[0].Trim ();                
                for (int j = 0; j < n_classes; j++)
                {
                    cm.class_conf[i, j] = uint.Parse(tokens[1 + j]);
                }
                cm.class_prob[i] = double.Parse(tokens[tokens.Length - 1]);
            }
            string[] token = lines[n_classes+5].Split(delims, StringSplitOptions.RemoveEmptyEntries);
            cm.overall_prob = double.Parse(token[0]);

            return cm;
        }
    }
}
