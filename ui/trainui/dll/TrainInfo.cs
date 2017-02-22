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
    public class TrainInfo
    {
        Collection<SelectItem> dates;
        public Collection<SelectItem> Dates
        {
            get { return dates; }
            set { dates = value; }
        }

        string annotation;
        public string Annotation
        {
            get { return annotation; }
            set { annotation = value; }
        }

        string signal;
        public string Signal
        {
            get { return signal; }
            set { signal = value; }
        }

        Project.SignalType type;
        public Project.SignalType Type
        {
            get { return type; }
            set { type = value; }
        }

        string model;
        public string Model
        {
            get { return model; }
            set { model = value; }
        }

        bool evalOn = true;
        public bool EvalOn
        {
            get { return evalOn; }
            set { evalOn = value; }
        }

        int evalType = 0;
        public int EvalType
        {
            get { return evalType; }
            set { evalType = value; }
        }

        int evalKFolds = 5;
        public int EvalKFolds
        {
            get { return evalKFolds; }
            set { evalKFolds = value; }
        }

        bool reExtract = false;
        public bool ReExtract
        {
            get { return reExtract; }
            set { reExtract = value; }
        }

        int contReps = 1;
        public int ContReps
        {
            get { return contReps; }
            set { contReps = value; }
        }

        int contFPS = 30;
        public int ContFPS
        {
            get { return contFPS; }
            set { contFPS = value; }
        }

        public void Save(string filepath)
        {
            XmlTextWriter xml = new XmlTextWriter(filepath, null);
            xml.Formatting = Formatting.Indented;
            xml.WriteStartDocument();
            xml.WriteStartElement("training");
            xml.WriteAttributeString("ssi-v", "1");

            xml.WriteStartElement("def");
            xml.WriteString(model);
            xml.WriteEndElement();

            xml.WriteStartElement("signal");
            xml.WriteString(signal);
            xml.WriteEndElement();

            xml.WriteStartElement("type");
            xml.WriteString("" + (int) type);
            xml.WriteEndElement();

            xml.WriteStartElement("anno");
            xml.WriteString(annotation);
            xml.WriteEndElement();

            xml.WriteStartElement("paths");           
            foreach (SelectItem date in dates)
            {                
                xml.WriteStartElement("item");
                xml.WriteString(date.Path);
                xml.WriteEndElement ();
            }
            xml.WriteEndElement();
            
            xml.WriteEndElement();
            xml.WriteEndDocument();
            xml.Close();
        }        

        static public TrainInfo Load(string filepath)
        {
            TrainInfo info = new TrainInfo();

            try
            {
                XmlTextReader xml = new XmlTextReader(filepath);

                xml.ReadStartElement("training");

                xml.ReadStartElement("def");
                info.model = xml.ReadString();
                xml.ReadEndElement();

                xml.ReadStartElement("signal");
                info.signal = xml.ReadString(); // new SelectItem(xml.ReadString(), false);
                xml.ReadEndElement();

                xml.ReadStartElement("anno");
                info.annotation = xml.ReadString();
                xml.ReadEndElement();

                xml.ReadStartElement("paths");                               
                while (xml.ReadToFollowing ("item"))
                {                                        
                    info.dates = new Collection<SelectItem>();                      
                    info.dates.Add(new SelectItem(xml.ReadString(), true));                 
                }               
                xml.ReadEndElement();

                xml.Close();
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
            }

            return info;
        }
    }
}
