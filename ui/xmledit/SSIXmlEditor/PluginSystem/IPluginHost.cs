using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.PluginSystem
{
	public interface IPluginHost
	{
		//T RequestInterface<T>() where T : class;		
		Document.ADocument ActiveDocument { get; }
	}
}
