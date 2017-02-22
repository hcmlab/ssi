using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.Command;

namespace SSIXmlEditor.View
{
    public interface IMainView : IView 
    {
        string Caption { set; }
        
        void RegisterCommand(string cmd, bool active);
        void ActivateCommand(string cmd, bool activate);
    }
}
