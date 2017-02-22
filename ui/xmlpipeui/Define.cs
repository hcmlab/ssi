using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;

namespace ssi
{
    public class Define
    {
        public const string INI_FILE_NAME = "xmlpipeui.ini";
        public const char CHAR_COMMENT = '#';
        public const char CHAR_EQUAL = '=';
        public const char CHAR_META_DELIM = ',';
        public const string CHAR_META_START = "$(";
        public const char CHAR_META_END = ')';
        public const string REGEX_VARIABLE = @"\$\((?<var>.+?)\)";        
        public const string REGEX_META = @"\$\((.*)\)(.*)";
        public const string REGEX_META_OPTIONS = @"(.*)\{(.*)\}";
        public const string FILEEX_CONFIG = ".pipeline-config";
        public const string FILEEX_PIPELINE = ".pipeline";
        public const string FILE_PIPEEXE = "xmlpipe.exe";

        public static void ShowErrorMessage(string message)
        {
            MessageBox.Show(message, "Error", MessageBoxButton.OK);
        }
    }
}
