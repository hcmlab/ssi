using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

namespace SSIXmlEditor.PluginSystem
{
	internal static class PluginManager
	{
		private static List<PluginSystem.IPlugin> m_Plugins;
	
		internal static void Initialize(IPluginHost host)
		{
			var plugins = System.IO.Directory.GetFiles("./plugins", "*.dll");
			if(null != m_Plugins)
				return;
				
			m_Plugins = new List<IPlugin>();
			
			foreach(var plugin in plugins)
			{
				var asmPlugin = Assembly.Load(plugin);
				var tmpPlugin = LoadPlugin(asmPlugin, "IPlugin");
				if (null != tmpPlugin)
				{
					tmpPlugin.Init(host);
					m_Plugins.Add(tmpPlugin);
				}
			}
		}		
		
		private static IPlugin LoadPlugin(Assembly asm, string interfaceName)
		{
			foreach(var type in asm.GetTypes())
			{
				if(!type.IsPublic)
					continue;
					
				var inft = type.GetInterface(interfaceName, true);
				if(null != inft)
				{
					var plugin = asm.CreateInstance(inft.FullName) as IPlugin;
					if(null != plugin)
					{
						return plugin;
					}					
				}
			}	
			
			return null;		
		}
	}
}
