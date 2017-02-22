using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Text.RegularExpressions;

namespace ssi
{
    public class AnnoList : MyEventList
    {
        private bool loaded = false;
        private String name = null;
        private String filename = null;
        private String filepath = null;

        public bool Loaded
        {
            get { return loaded; }
            set { loaded = value; }
        }

        public string Filename
        {
            get { return filename; }
        }

        public string Filepath
        {
            get { return filepath; }
        }

        public String Name
        {
            get { return name; }
        }

        public AnnoList()
        {
        }

        public AnnoList(String filepath)
        {
            this.filepath = filepath;
            string[] tmp = filepath.Split('\\');
            this.filename = tmp[tmp.Length - 1];
            this.name = this.filename.Split('.')[0];                      
        }

        public static AnnoList LoadfromFile(String filepath)
        {
            AnnoList list = new AnnoList(filepath);

            try
            {
                StreamReader sr = new StreamReader (filepath, System.Text.Encoding.Default);
                //list.events = new MyEventList();
                string pattern = "^([0-9]+.[0-9]+|[0-9]+) ([0-9]+.[0-9]+|[0-9]+) .+";
                Regex reg = new Regex(pattern);
                string line = null;
                while ((line = sr.ReadLine()) != null)
                {
                    if (reg.IsMatch(line))
                    {
                        string[] data = line.Split(' ');                        
                        double start = Convert.ToDouble(data[0]);
                        double duration = Convert.ToDouble(data[1]) - Convert.ToDouble(data[0]);
                        string label = data[2];
                        for (int i = 3; i < data.Length; i++)
                        {
                            label += " " + data[i];
                        }
                        AnnoListItem e = new AnnoListItem(start, duration, label);
                        list.Add(e);
                    }
                }
                sr.Close(); ;
                list.loaded = true;
            }
            catch (Exception e)
            {
                Console.WriteLine("Can't read annotation file: " + filepath, e);
            }

            return list;
        }

        public AnnoList saveToFile(String _filename)
        {
            try
            {
                StreamWriter sw = new StreamWriter(_filename, false, System.Text.Encoding.Default); 
                
                foreach (AnnoListItem e in this)
                {
                    sw.WriteLine(e.Start.ToString() + " " + e.Stop.ToString() + " " + e.Label);
                }
                sw.Close();

                AnnoList newAnno = new AnnoList(_filename);
                return newAnno;

            }
            catch
            {
                return null;
            }
        }
    }
}
