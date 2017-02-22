using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace ssi
{
    public class Setting : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private string configSearchDir = "";
        public string ConfigSearchDir { 
            get { return configSearchDir; }
            set { configSearchDir = value; 
                  if (PropertyChanged != null) {
                     PropertyChanged(this, new PropertyChangedEventArgs("ConfigSearchDir"));
                  }
                }
        }

        private string pipeSearchDir = "";
        public string PipeSearchDir
        {
            get { return pipeSearchDir; }
            set
            {
                pipeSearchDir = value;
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("PipeSearchDir"));
                }
            }
        }

        private string pipeExePath = "";
        public string PipeExePath
        {
            get { return pipeExePath; }
            set
            {
                pipeExePath = value;
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs("PipeExePath"));
                }
            }
        }


    }
}
