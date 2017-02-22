using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.Text.RegularExpressions;

namespace ssi
{
    interface ISelectItemFilter
    {
        void select(Collection<SelectItem> source, ref Collection<SelectItem> target);
    }

    public class SelectItemFilterRegex : ISelectItemFilter
    {
        Regex regex;

        public SelectItemFilterRegex(Regex regex)
        {
            this.regex = regex;
        }
        
        #region ISelectItemFilter Members

        public void select(Collection<SelectItem> source, ref Collection<SelectItem> target)
        {
            foreach (SelectItem item in source) {
                if (regex.IsMatch(item.ToString ()))
                {
                    target.Add(item);
                }
            }
        }

        #endregion
    }
 
}
