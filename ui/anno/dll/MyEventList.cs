using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;

namespace ssi
{
    public class MyEventList : MyList<AnnoListItem>, INotifyPropertyChanged
    {
        public MyEventList()
        {
            foreach (AnnoListItem item in Items)
            {
                item.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(item_PropertyChanged);
            }
        }

        ~MyEventList()
        {
            foreach (AnnoListItem item in Items)
            {
                item.PropertyChanged -= new System.ComponentModel.PropertyChangedEventHandler(item_PropertyChanged);
            }
        }

        void item_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            OnChildPropertyChanged(e.PropertyName);
        }

        override protected void itemRemoved(AnnoListItem removedItem)
        {
            removedItem.PropertyChanged -= new System.ComponentModel.PropertyChangedEventHandler(item_PropertyChanged);
        }

        override protected void itemAdded(AnnoListItem addedItem)
        {
            addedItem.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(item_PropertyChanged);
        }

        override protected void itemReplaced(AnnoListItem replacedItem)
        {
            //overwrite and do anything
        }

        public event PropertyChangedEventHandler ChildPropertyChanged;
        protected void OnChildPropertyChanged(string propertyName)
        {
            if (this.ChildPropertyChanged != null)
                ChildPropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
