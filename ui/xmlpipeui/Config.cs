using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Text.RegularExpressions;
using System.ComponentModel;
using System.Collections;
using System.Windows.Controls;

namespace ssi
{
    public class Config : IEnumerable, INotifyPropertyChanged
    {
        string path = null;
        public string Path
        {
            get
            {
                return path;
            }
            set
            {                
                path = System.IO.Path.GetDirectoryName(value) + "\\" + System.IO.Path.GetFileNameWithoutExtension(value) + Define.FILEEX_CONFIG;                
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        List<Config.Item> items;

        public class Item : INotifyPropertyChanged
        {
            public event PropertyChangedEventHandler PropertyChanged;

            public Item(string line) : this ()
            {                
                Parse(line);
            }

            public Item(Item item)
            {
                Line = item.Line;
                Value = item.Value;
                ValueType = item.ValueType;
                ValueOptions = item.ValueOptions;
                Comment = item.Comment;                
                Key = item.Key;
            }

            public Item()
            {
                Line = "";                
                ValueType = Type.NONE;
                ValueOptions = "";
                Comment = "";
                Value = "";
                Key = "";
            }

            public enum Type
            {           
                NONE,
                TEXT,
                BOOLEAN,
                SELECTION
            }

            string line;
            public string Line
            {
                get { return line; }
                set
                {
                    line = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Line"));
                    }
                }
            }

            string key;
            public string Key
            {
                get { return key; }
                set
                {
                    key = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Key"));
                    }
                }
            }

            string value;
            public string Value
            {
                get { return value; }
                set
                {
                    this.value = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("ConfigValueSearchDir"));
                    }
                }
            }

            Type valueType;
            public Type ValueType
            {
                get { return valueType; }
                set
                {
                    valueType = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("ValueType"));
                    }
                }
            }

            string valueOptions;
            public string ValueOptions
            {
                get { return valueOptions; }
                set
                {
                    this.valueOptions = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("ValueOptions"));
                    }
                }
            }

            string comment;
            public string Comment
            {
                get { return comment; }
                set
                {
                    comment = value;
                    if (PropertyChanged != null)
                    {
                        PropertyChanged(this, new PropertyChangedEventArgs("Comment"));
                    }
                }
            }

            public void Parse(string line)
            {
                line.Trim();
                
                // too short
                if (line.Length <= 2)
                {                    
                    return;
                }

                // is a comment
                if (line[0] == Define.CHAR_COMMENT)
                {                    
                    return;
                }
            
                // split key/value from comment
                string[] tokens = line.Split(Define.CHAR_COMMENT);
                string keyvalue = tokens[0];            
                if (tokens.Length > 1)
                {
                    Comment = String.Join ("", tokens, 1, tokens.Length-1).Trim ();                
                }

                // parse key and value
                tokens = keyvalue.Split(Define.CHAR_EQUAL);
                if (tokens.Length != 2)
                {
                    return;
                }
                Key = tokens[0].Trim();
                Value = tokens[1].Trim();
                ValueType = Item.Type.TEXT; 
                
                // parse meta from comment                
                Regex regex = new Regex(Define.REGEX_META);
                Match match = regex.Match(Comment.Trim());
                if (match.Success && match.Groups.Count == 3)
                {                                
                    string meta = match.Groups[1].Value.Trim ();
                    ParseMeta(meta);  
                    Comment = match.Groups[2].Value.Trim ();
                }   
            }

            void ParseMeta(string meta)
            {               
                string command = meta;                
                Regex regex = new Regex(Define.REGEX_META_OPTIONS);
                Match match = regex.Match(meta);
                if (match.Success && match.Groups.Count == 3)
                {
                    command = match.Groups[1].Value.Trim();
                    ValueOptions = match.Groups[2].Value.Trim();
                }                
               
                switch (command.ToLower())
                {
                    case "boolean":
                    case "bool":
                        {
                            ValueType = Item.Type.BOOLEAN;
                            break;
                        }
                    case "select":                        
                        {
                            ValueType = Item.Type.SELECTION;
                            break;
                        }
                }

            }

            public override string ToString()
            {
                StringBuilder sb = new StringBuilder();
 
                if (ValueType != Item.Type.NONE)
                {
                    sb.Append(Key);
                    sb.Append(" " + Define.CHAR_EQUAL + " ");
                    sb.Append(Value + " ");
                }


                if (ValueType != Item.Type.NONE || Comment != "")
                {
                    sb.Append(Define.CHAR_COMMENT);
                    switch (ValueType)
                    {
                        case Type.BOOLEAN:
                            sb.Append(" " + Define.CHAR_META_START + "bool" + Define.CHAR_META_END);
                            break;
                        case Type.SELECTION:
                            sb.Append(" " + Define.CHAR_META_START + "select{" + ValueOptions + "}" + Define.CHAR_META_END);
                            break;
                    }
                    if (Comment != "")
                    {
                        sb.Append(" " + Comment);
                    }
                }

                return sb.ToString();
            }

            public static implicit operator ListViewItem(Item v)
            {
                throw new NotImplementedException();
            }
        }

        public Config()
        {
            items = new List<Item>();
        }

        public IEnumerator GetEnumerator()
        {
            foreach (object o in items)
            {
                if (o == null)
                {
                    break;
                }               
                yield return o;
            }
        }

        public void Clear()
        {
            items.Clear();
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs("Config"));
            }
        }

        public void Remove(Item item)
        {
            items.Remove(item);
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs("Config"));
            }
        }

        public void Add(Item item)
        {
            item.PropertyChanged += new PropertyChangedEventHandler(item_PropertyChanged);
            items.Add(item);
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs("Config"));
            }
        }

        void item_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs("Config"));
            }
        }

        public void ParseItemsFromPipe(string pathWithOrWithoutExtension)
        {
            Path = pathWithOrWithoutExtension;

            string[] lines = System.IO.File.ReadAllLines(path);
            Regex regex = new Regex(Define.REGEX_VARIABLE);
            foreach (string line in lines)
            {                
                MatchCollection matches = regex.Matches (line);
                foreach (Match match in matches)
                {                    
                    string key = match.Groups["var"].Value;
                    InsertKey(key);                                            
                }                                      
            }
        }

        private void InsertKey(string key)
        {
            foreach (Item i in items)
            {
                if (i.Key == key)
                {
                    return;
                }
            }

            Item item = new Item();
            item.Key = key;
            item.ValueType = Item.Type.TEXT;
            Add(item);
        }

        public void Load(string pathWithOrWithoutExtension)
        {
            Path = pathWithOrWithoutExtension;

            try
            {
                string[] lines = System.IO.File.ReadAllLines(Path);
                foreach (string line in lines)
                {
                    Item item = new Item (line);                    
                    Add(item);                    
                }
            }
            catch (Exception ex)
            {
                Define.ShowErrorMessage(ex.ToString());
            }
        }

        public void Save(string pathWithOrWithoutExtension)
        {
            Path = pathWithOrWithoutExtension;

            try
            {
                using (System.IO.StreamWriter writer = new System.IO.StreamWriter(path))
                {
                    foreach (Item i in items)
                    {
                        writer.WriteLine(i.ToString ());                        
                    }
                }
            }
            catch (Exception ex)
            {
                Define.ShowErrorMessage(ex.ToString());
            }
        }

    }
}
