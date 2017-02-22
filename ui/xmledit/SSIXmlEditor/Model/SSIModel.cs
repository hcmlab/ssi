using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.Model
{
    public class SSIModel : IModel 
    {
        public event EventHandler<View.SSIModelChangedEventArgs> ModelChanged;
        public event EventHandler SelectionChanged;

        public ISSIModules SSIModules { set; private get; }
		public Object Selected { get; private set; }
		public MetaData GetMetaData(string value)
        {
            return SSIModules.GetMetaData(value);
        }

		public void Select(Object model)
		{
			if (null != Selected && Selected.Equals(model))
				return;

			Selected = model;

			if (null != SelectionChanged)
				SelectionChanged(this, EventArgs.Empty);
		}

        public void Change(View.SSIModelChangedEventArgs e)
        {
            if (null != ModelChanged)
                ModelChanged(this, e);
        }
    }
}
