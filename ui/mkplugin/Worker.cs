using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;

namespace ssi
{
    class Worker
    {
        public StringCollection Log = new StringCollection();
        Project project;
        Regex regex = new Regex (@"\$\((.*?)\)", RegexOptions.Compiled);

        public Worker(Project project)
        {
            this.project = project;
        }

        public void Run()
        {
            DirectoryInfo source = new DirectoryInfo(project.Source);
            DirectoryInfo target = new DirectoryInfo(project.Target + Path.DirectorySeparatorChar + project.Name);
            CreateFolder(target.FullName);
            Walk(source, target, project.Map);
        }

        bool CreateFolder(string path) 
        {
            try
            {               
                Directory.CreateDirectory(path);
            }
            catch (Exception e)
            {
                MessageBox.Show("Could not create project folders!\r\n\r\n" + e.ToString(), "Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                return false;
            }

            return true;
        }

        void CopyFile(string source, string target, Dictionary<string,string> map)
        {
            target = Replace(target, map);
            File.Copy(source, target, true);            
            string content = File.ReadAllText(target);
            content = Replace(content, map);
            File.WriteAllText(target, content);
        }

        string Replace(string s, Dictionary<string,string> map)
        {
            string result = s;
            foreach (KeyValuePair<string, string> entry in map)
            {
                Regex regex = new Regex(@"\$\(" + entry.Key + @"\)");
                result = regex.Replace(result, entry.Value);             
            }
            return result;   
        }

        void Walk(DirectoryInfo source, DirectoryInfo target, Dictionary<string, string> map)
        {
            FileInfo[] files = null;
            DirectoryInfo[] subDirs = null;

            try
            {
                files = source.GetFiles("*.*");
            }
            catch (UnauthorizedAccessException e)
            {
                Log.Add(e.Message);
            }

            catch (DirectoryNotFoundException e)
            {
                Console.WriteLine(e.Message);
            }

            if (files != null)
            {
                foreach (FileInfo fi in files)
                {             
                    Console.WriteLine(fi.FullName);
                    CopyFile(fi.FullName, target.FullName + Path.DirectorySeparatorChar + fi.Name, map);
                }

                subDirs = source.GetDirectories();

                foreach (DirectoryInfo dirInfo in subDirs)
                {                    
                    string path = target.FullName + Path.DirectorySeparatorChar + dirInfo.Name;
                    Console.WriteLine(path);

                    if (CreateFolder(path))
                    {                        
                        Walk(dirInfo, new DirectoryInfo (path), map);
                    }
                }
            }
        }
    }
}
