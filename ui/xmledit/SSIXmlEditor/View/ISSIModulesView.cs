using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.View
{
    public class SSIModuleEventArgs : EventArgs
    {
        public MetaData SSIModule { get; private set; }

        public SSIModuleEventArgs(MetaData ssimodule)
        {
            SSIModule = ssimodule;
        }
    }

    public interface ISSIModulesView : IView 
    {
        event EventHandler<SSIModuleEventArgs> SSIModuleSelected;
        event EventHandler<SSIModuleEventArgs> SSIModuleActivated;
        
        IEnumerable<MetaData> SSIModules { set; }
    }
}
