using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;

namespace ssi
{
    public class ReLabelItem
    {

        public ReLabelItem(string label)
        {
            this.old_label = label;
            this.new_label = label;            
        }
        
        String old_label;
        public String OldLabel
        {
            get { return old_label; }
            set { old_label = value; }
        }

        String new_label;
        public String NewLabel
        {
            get { return new_label; }
            set { new_label = value; }
        }
    }

    public class ReLabelList : ObservableCollection<ReLabelItem>
    {
    }

    public class ReLabelListEqualityComparer : EqualityComparer<ReLabelItem>
    {
        public override bool Equals(ReLabelItem x, ReLabelItem y)
        {
            return x.OldLabel.Equals(y.OldLabel);
        }

        public override int GetHashCode(ReLabelItem item)
        {
            return item.OldLabel.GetHashCode();
        }
    }
}
