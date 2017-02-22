using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ssi
{
    public enum SelectItemType
    {
        DIRECTORY = 0,
        SIGNAL,
        ANNOTATION,
        SAMPLES,
        UNKOWN
    }

    public class SelectItem
    {
        string fullname;
        public string FullName
        {
            get { return fullname; }
        }

        string path;
        public string Path
        {
            get { return path; }
        }

        string name;
        public string Name
        {
            get { return name; }
        }

        SelectItemType type;
        public SelectItemType Type
        {
            get { return type; }
        }

        public SelectItem(string path, bool isDirectory)
        {
            this.fullname = path.Substring(path.LastIndexOf('\\') + 1);
            this.name = this.fullname;
            this.type = SelectItemType.UNKOWN;
            if (isDirectory)
            {                
                this.type = SelectItemType.DIRECTORY;
            }
            else
            {
                int endindex = fullname.LastIndexOf('.');
                if (endindex != -1)
                {
                    string ending = fullname.Substring(endindex);
                    this.name = fullname.Substring(0, endindex);
                    switch (ending)
                    {
                        case ".stream":                                                
                        case ".wav":
                        case ".avi":
                            this.type = SelectItemType.SIGNAL;
                            break;
                        case ".anno":
                            this.type = SelectItemType.ANNOTATION;
                            break;
                        case ".samples":
                            this.type = SelectItemType.SAMPLES;
                            break;
                    }
                }
            }
            this.path = path;
        }

        public override string ToString()
        {
            return fullname;
        }
    }
}
