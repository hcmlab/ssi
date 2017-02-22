using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.View
{
    public class SSIModelChangedEventArgs : EventArgs
    {
		public Object Model { get; private set; }
        public string Category { get; private set; }
        public string Property { get; private set; }
        public object NewValue { get; private set; }
        public object OldValue { get; private set; }

        public SSIModelChangedEventArgs(object model, string category, string property, object newValue, object oldValue)
        {
            if (null == model)
                throw new ArgumentNullException("model");
            if (String.IsNullOrEmpty(property)) 
                throw new ArgumentNullException("property");
            if(null == newValue)
                throw new ArgumentNullException("newValue");
            
            Model = model;
            Category = category;
            Property = property;
            NewValue = newValue;
            OldValue = oldValue;
        }
    }

    public interface ISSIModelView : IView
    {
        event EventHandler<SSIModelChangedEventArgs> ModelChanged;

        Object Selected { set; }
    }
}
