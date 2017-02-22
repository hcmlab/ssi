using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ssi
{
 
    class Project : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;        

        public void OnPropertyChanged(string name)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(name));
            }
        }

        public Project() 
        {
            Reset();
        }

        public void Reset()
        {
            Source = @"plugins\_template";
            Target = "plugins";
            Name = "";
            GUID = Guid.NewGuid().ToString();
            Author = "";
            Email = "";
            Date = DateTime.Now.ToString("d/M/yyyy");
        }

        public Dictionary<string, string> Map
        {
            get
            {
                Dictionary<string, string> map = new Dictionary<string, string>();
                map.Add("name", Name);
                map.Add("NAME", Name.ToUpper());
                map.Add("Name", char.ToUpper(Name[0]) + Name.Substring (1));
                map.Add("guid", GUID);
                map.Add("author", Author);
                map.Add("email", Email);
                map.Add("date", Date);
                return map;
            }
        }

        string source;
        public string Source
        {
            get
            {
                return source;
            }
            set
            {
                source = value;
                OnPropertyChanged("Source");
            }
        }

        string target;
        public string Target
        {
            get
            {
                return target;
            }
            set
            {
                target = value;
                OnPropertyChanged("Target");
            }
        }

        string name;
        public string Name
        {
            get
            {
                return name;
            }
            set
            {
                name = value;
                OnPropertyChanged("Name");
            }
        }

        string guid;
        public string GUID
        {
            get
            {
                return guid;
            }
            set
            {
                guid = value;
                OnPropertyChanged("GUID");
            }
        }

        string author;
        public string Author
        {
            get
            {
                return author;
            }
            set
            {
                author = value;
                OnPropertyChanged("Author");
            }
        }

        string email;
        public string Email
        {
            get
            {
                return email;
            }
            set
            {
                email = value;
                OnPropertyChanged("Email");
            }
        }

        string date;
        public string Date
        {
            get
            {
                return date;
            }
            set
            {
                date = value;
                OnPropertyChanged("Date");
            }
        }

        public bool Check()
        {
            return name != "" && email != "" && author != "" && guid != "" && date != "";
        }
    }
}
