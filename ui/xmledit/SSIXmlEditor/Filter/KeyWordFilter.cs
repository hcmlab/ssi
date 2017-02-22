using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Filter
{
    public class KeyWordFilterEventArgs : EventArgs
    {
        public string Filtered { get; private set; }

        public KeyWordFilterEventArgs(string value)
        {
            if (string.IsNullOrEmpty(value))
                throw new ArgumentNullException("value");

            Filtered = value;
        }
    }

    public class KeyWordFilter
    {
        public event EventHandler<KeyWordFilterEventArgs> Found;

        private IEnumerable<string> KeyWords { get; set; }

        public IFilterStrategy FilterStrategy { get; set; }

        public KeyWordFilter(IEnumerable<string> keywords)
        {
            if (null == keywords)
                throw new ArgumentNullException("keywords");

            KeyWords = keywords;
        }

        public void Filter(string text)
        {
            string found;
            if (FilterStrategy.Matches(text, out found))
            {
                if (string.IsNullOrEmpty(found))
                    return;

                var result = KeyWords.Where(c => c.Equals(found));

                if (result.Count() > 0 && null != Found)
                    Found(this, new KeyWordFilterEventArgs(found));
            }
        }
    }
}
