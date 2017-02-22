using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace SSIXmlEditor
{
	class Win32
	{
		[DllImport("kernel32")]
		public static extern long WritePrivateProfileString(string section,
			string key, string val, string filePath);
		[DllImport("kernel32")]
		public static extern int GetPrivateProfileString(string section,
				 string key, string def, StringBuilder retVal,
			int size, string filePath);
	}

	public class IniFile
	{
		public string path;

		public IniFile(string INIPath)
		{
			path = INIPath;
		}

		public void IniWriteValue(string Section, string Key, string Value)
		{
			Win32.WritePrivateProfileString(Section, Key, Value, this.path);
		}

		public string IniReadValue(string Section, string Key)
		{
			StringBuilder temp = new StringBuilder(255);
			var fi = new System.IO.FileInfo(path);
			
			int i = Win32.GetPrivateProfileString(Section, Key, "", temp, 255, fi.FullName);
			return temp.ToString();

		}
	}
}