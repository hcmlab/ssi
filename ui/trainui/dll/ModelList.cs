using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.IO;
using System.Xml;
using System.Text.RegularExpressions;
using System.Windows;

namespace ssi
{
    public class ModelItem
    {
        int version;

        string[] classes;
        public string Classes
        {
            get {
                StringBuilder sb = new StringBuilder();
                bool first = true;
                foreach (string c in classes)
                {
                    sb.Append((first ? "" : "\n") + c);                    
                    first = false;
                }
                return sb.ToString (); 
            }
        }

        string[] users;
        public string Users
        {
            get
            {
                StringBuilder sb = new StringBuilder();
                bool first = true;
                foreach (string c in users)
                {
                    sb.Append((first ? "" : "\n") + c);
                    first = false;
                }
                return sb.ToString();
            }
        }

        string path;
        public string Path
        {
            get { return path; }
        }

        string dir;
        public string Dir
        {
            get { return dir; }
        }

        string name;
        public string Name
        {
            get { return name; }
        }

        string date;
        public string Date
        {
            get { return date; }            
        }

        static public ModelItem Load(string dir)
        {
            ModelItem model = null;

            string[] files = Directory.GetFiles(dir, "*.trainer");           
            if (files.Length > 0)
            {                                
                model = new ModelItem();
                model.dir = dir;
                model.path = files[0].Substring(0, files[0].LastIndexOf('.'));
                model.name = model.path.Substring(model.path.LastIndexOf('\\') + 1);                               
                model.date = dir.Substring (dir.LastIndexOf ('\\') + 1);                                 
                try
                {
                    XmlDocument xml = new XmlDocument();
                    XmlNodeList nodes = null;
                    int i = 0;

                    xml.Load(files[0]);
                   
                    nodes = xml.DocumentElement.SelectNodes("/trainer/classes/item");
                    model.classes = new string[nodes.Count];
                    i = 0;
                    foreach (XmlNode node in nodes)
                    {
                        model.classes[i++] = node.Attributes["name"].Value;
                    }

                    nodes = xml.DocumentElement.SelectNodes("/trainer/users/item");
                    model.users = new string[nodes.Count];
                    i = 0;
                    foreach (XmlNode node in nodes)
                    {
                        model.users[i++] = node.Attributes["name"].Value;
                    }                                        
                }
                catch (Exception e)
                {
                    MessageBox.Show(e.ToString());
                }
            }
            return model;
        }
        
    }

    public class ModelList : ObservableCollection<ModelItem>
    {
    }

}
