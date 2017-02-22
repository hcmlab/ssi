using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;

namespace SSIXmlEditor.Command
{
	class LoadPluginsCommand : ICommand
	{
		private readonly string m_PluginsPath;
		
		public LoadPluginsCommand(string path)
		{
			m_PluginsPath = path;
		}
	
		#region ICommand Member

		public void Execute()
		{
			if(!Directory.Exists(m_PluginsPath))
				return;
		
			string[] assemblies = Directory.GetFiles(m_PluginsPath);

			foreach (string file in assemblies)
			{
				System.Diagnostics.Debug.WriteLine("try loading plugin: " + file);
				
				var assembly = Assembly.LoadFrom(file);
				
			}
		
			throw new NotImplementedException();
		}

		#endregion
	}
}
