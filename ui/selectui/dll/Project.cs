using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Windows;
using System.Threading;
using System.Text.RegularExpressions;

namespace ssi
{

    public class Project
    {        
        public static string PROJECT_FILE_NAME = "modelui.project.xml";
        public static string PROJECT_DATA_DIR = "data";
        public static string PROJECT_STIMULI_DIR = "stimuli";
        public static string PROJECT_BIN_DIR = "bin";
        public static string PROJECT_OPTS_DIR = "opts";
        public static string PROJECT_TRAIN_DIR = "train";
        public static string PROJECT_LOG_DIR = "log";
        public static string PROJECT_EVAL_DIR = "eval";

        public enum SignalType
        {
            STREAM = 0,
            AUDIO = 1,
            VIDEO = 2
        }

        #region vars

        string name;
        public string Name
        {
            get { return name; }
        }

        string dir;
        public string Dir
        {
            get { return dir; }
        }

        string dataDir;
        public string DataDir
        {
            get { return dataDir; }
        }

        string trainDir;
        public string TrainDir
        {
            get { return trainDir; }
        }

        string evalDir;
        public string EvalDir
        {
            get { return evalDir; }
        }

        string stimuliDir;
        public string StimuliDir
        {
            get { return stimuliDir; }
        }

        string logDir;
        public string LogDir
        {
            get { return logDir; }
        }

        string optsDir;
        public string OptsDir
        {
            get { return optsDir; }
        }

        string stimuli;
        public string Stimuli
        {
            get { return stimuli; }
        }

        string signal;
        public string Signal
        {
            get { return signal; }
        }

        Project.SignalType type = Project.SignalType.STREAM;
        public Project.SignalType Type
        {
            get { return type; }
        }

        string anno;
        public string Anno
        {
            get { return anno; }
        }

        string[] defNames;
        public string[] DefNames
        {
            get { return defNames; }
        }

        #endregion

        #region create
        public Project(string dir, string name, string stimuli, string pipe, string def, string signal, SignalType type, string anno)
        {            
            this.name = name;
            if (!Path.IsPathRooted(dir))
            {
                dir = Directory.GetCurrentDirectory() + "\\" + dir;
            }
            this.dir = dir;
           // this.dir = dir;
            if (stimuli != "")
            {
                this.stimuli = stimuli.Substring(stimuli.LastIndexOf('\\') + 1);
            }
            else
            {
                this.stimuli = createEmptyStimuli(dir + "\\" + PROJECT_STIMULI_DIR);
            }
            this.dataDir = dir + "\\" + PROJECT_DATA_DIR;
            this.trainDir = dir + "\\" + PROJECT_TRAIN_DIR;
            this.evalDir = dir + "\\" + PROJECT_EVAL_DIR;
            this.logDir = dir + "\\" + PROJECT_LOG_DIR;
            this.optsDir = dir + "\\" + PROJECT_OPTS_DIR;
            this.stimuliDir = dir + "\\" + PROJECT_STIMULI_DIR;
            this.signal = signal;
            this.type = type;
            this.anno = anno;

            if (!Directory.Exists(this.dir))
            {
                Directory.CreateDirectory(this.dir);
            }            
            if (!Directory.Exists(this.dataDir))
            {
                Directory.CreateDirectory(this.dataDir);
            }
            if (!Directory.Exists(this.trainDir))
            {
                Directory.CreateDirectory(this.trainDir);
            }
            if (!Directory.Exists(this.evalDir))
            {
                Directory.CreateDirectory(this.evalDir);
            }
            if (!Directory.Exists(this.logDir))
            {
                Directory.CreateDirectory(this.logDir);
            }
            if (!Directory.Exists(this.optsDir))
            {
                Directory.CreateDirectory(this.optsDir);
            }
            if (!Directory.Exists(this.stimuliDir))
            {
                Directory.CreateDirectory(this.stimuliDir);
            }
            if (!File.Exists(stimuliDir + "\\" + stimuli))
            {
                string stimuliFileName = new FileInfo(stimuli).Name;
                File.Copy(stimuli, stimuliDir + "\\" + stimuliFileName);
            }
            if (!File.Exists(dir + "\\" + name + ".pipeline"))
            {
                File.Copy(pipe, dir + "\\" + name + ".pipeline");
            }
            if (!File.Exists(dir + "\\" + name + ".traindef"))
            {
                File.Copy(def, dir + "\\" + name + ".traindef");
            }

            if (!File.Exists(dir + "\\" + PROJECT_FILE_NAME))
            {               
                save();    
            }            
           
            ssi.MlpXmlRun.xmlTrainFilepath = @"mlpxml.exe";
            ssi.MlpXmlTrain.xmlTrainFilepath = @"mlpxml.exe";

            readDefNames(dir + "\\" + name + ".traindef");
        }

        public void Close()
        {
        }

        private void readDefNames(string filepath)
        {
            List<string> defs = new List<string> ();
            try {                                
                XmlTextReader xml = new XmlTextReader(filepath);
                xml.ReadToFollowing("traindef");
                bool found = false;
                do {
                    found = xml.ReadToFollowing("item"); 
                    if (found) 
                    {                        
                        defs.Add (xml.GetAttribute ("name"));
                    }
                } while (found);
            } catch (Exception e) 
            {
                Error.Show (e.ToString ());
            }
            defNames = defs.ToArray ();
        }

        private string createEmptyStimuli(string dir)
        {
            string filename = "stimuli.xml";
            string filepath = dir + '\\' + filename;
            string source = "<?xml version=\"1.0\"?>\n<!--SSI MLP Stimuli File-->\n<stimuli>\n\t<list size=\"0\">\t</list>\n</stimuli>\n";
            Directory.CreateDirectory(dir);
            TextWriter writer = new StreamWriter(filepath);
            writer.Write(source);
            writer.Close();            
            return filename;
        }

        public override string ToString()
        {
            return name;
        }

        public void AddUser(string name)
        {
            Directory.CreateDirectory(this.dataDir + '\\' + name);
        }
        #endregion

        #region xml

        void save()
        {
            Directory.CreateDirectory(dir);
            string filename = dir + "\\" + PROJECT_FILE_NAME;
            XmlTextWriter xml = new XmlTextWriter(filename, null);
            xml.Formatting = Formatting.Indented;
            xml.WriteStartDocument();
            xml.WriteComment("SSI MLP Project File");
            xml.WriteStartElement("project");
            xml.WriteStartElement("name");
            xml.WriteString(name);
            xml.WriteEndElement();
            xml.WriteStartElement("stimuli");
            xml.WriteString(stimuli);                        
            xml.WriteEndElement();
            xml.WriteStartElement("signal");
            xml.WriteString(signal);
            xml.WriteEndElement();
            xml.WriteStartElement("type");
            xml.WriteString(type.ToString ());
            xml.WriteEndElement();  
            xml.WriteStartElement("anno");
            xml.WriteString(anno);
            xml.WriteEndElement();
            xml.WriteEndDocument();
            xml.Close();
        }

        public static Project Load (string path)
        {
            string filename = path + "\\" + PROJECT_FILE_NAME;
            string name = null;
            string stimuli = null;
            string signal = null;
            SignalType type = SignalType.STREAM;
            string anno = null;
            Project project = null;
            if (File.Exists(filename))
            {                
                XmlTextReader xml = new XmlTextReader(filename);                
                try {                    
                    xml.ReadToFollowing("project");        
                    xml.ReadToFollowing("name");                    
                    name = xml.ReadString();
                    xml.ReadToFollowing("stimuli");
                    stimuli = xml.ReadString();
                    xml.ReadToFollowing("signal");
                    signal = xml.ReadString();
                    xml.ReadToFollowing("type");
                    type = (SignalType) Enum.Parse (typeof (SignalType), xml.ReadString(), true);                    
                    xml.ReadToFollowing("anno");
                    anno = xml.ReadString();                  
                    project = new Project(path, name, stimuli, name, name, signal, type, anno);
                } catch (Exception e) {
                    project = null;
                    MessageBox.Show (e.ToString ());
                }                               
                xml.Close();
            }
            return project;
        }

        #endregion        
    }
}
