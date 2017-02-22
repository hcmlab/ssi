using System;
using System.Linq;
using System.Text;
using System.Collections.Generic;
using System.Xml;
using System.IO;

namespace ssi
{    

    public class Projects : List<Project>
    {
        public static string PROJECTS_FILE_NAME = "modelui.projects.xml";

        public Projects(string path)
        {
            if (!File.Exists(path))
            {
                Save(path);
            }

            try
            {
                XmlTextReader xml = new XmlTextReader(path);
                xml.ReadToFollowing("projects");
                while (xml.ReadToFollowing("item"))
                {
                    try
                    {
                        string dir = xml.ReadString();
                        Project project = Project.Load(dir);
                        Add(project);
                    }
                    catch (Exception e)
                    {
                        Error.Show(e.ToString());
                    }
                }
                xml.Close();
            }
            catch (Exception e)
            {
                Error.Show(e.ToString());
            }
        }

        public void Save (string path) 
        {
            try
            {
                XmlTextWriter xml = new XmlTextWriter(path, Encoding.Default);
                xml.Formatting = Formatting.Indented;
                xml.WriteStartDocument();
                xml.WriteComment("SSI MLP Project Collection");
                xml.WriteStartElement("projects");
                foreach (Project item in this)
                {                                       
                    xml.WriteStartElement("item");
                    xml.WriteString(item.Dir);
                    xml.WriteEndElement();                    
                }
                xml.WriteEndElement();
                xml.WriteEndDocument();
                xml.Close();
            }
            catch (Exception e)
            {
                Error.Show(e.ToString());
            }
        }
    }
}
