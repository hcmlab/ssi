using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SSIXmlEditor.PluginSystem
{
	public interface IPlugin
	{
		string Name { get; }
		IPluginHost Host { get; set; }
	
		void Init(IPluginHost host);
		void Dispose();
	}
}
