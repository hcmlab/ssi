using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SSIXmlEditor.PluginSystem;

namespace XmlMacroPlugin
{
	public class PluginMain : IPlugin
	{
		#region IPlugin Member

		public string Name
		{
			get { return "Macro Plugin"; }
		}

		public IPluginHost Host { get; set; }

		public void Init(IPluginHost host)
		{
			Host = host;
		}

		public void Dispose()
		{
			throw new NotImplementedException();
		}

		#endregion
	}
}
