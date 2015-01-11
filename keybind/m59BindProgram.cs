using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace m59bind
{
    static class m59BindProgram
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new M59Bind());
        }

        [DllImport("KERNEL32.DLL", EntryPoint = "GetPrivateProfileStringW",
        SetLastError = true,
        CharSet = CharSet.Unicode, ExactSpelling = true,
        CallingConvention = CallingConvention.StdCall)]
        private static extern int GetPrivateProfileString(
          string lpAppName,
          string lpKeyName,
          string lpDefault,
          string lpReturnString,
          int nSize,
          string lpFilename);

        [DllImport("KERNEL32.DLL", EntryPoint = "WritePrivateProfileStringW",
        SetLastError = true,
        CharSet = CharSet.Unicode, ExactSpelling = true,
        CallingConvention = CallingConvention.StdCall)]
        private static extern int WritePrivateProfileString(
        string lpAppName,
        string lpKeyName,
        string lpString,
        string lpFilename);

        private static List<string> GetCategories(string iniFile)
        {
            string returnString = new string(' ', 65536);
            GetPrivateProfileString(null, null, null, returnString, 65536, iniFile);
            List<string> result = new List<string>(returnString.Split('\0'));
            result.RemoveRange(result.Count - 2, 2);
            return result;
        }

        private static List<string> GetKeys(string iniFile, string category)
        {
            string returnString = new string(' ', 32768);
            GetPrivateProfileString(category, null, null, returnString, 32768, iniFile);
            List<string> result = new List<string>(returnString.Split('\0'));
            result.RemoveRange(result.Count-2,2);
            return result;
        }

        public static string GetIni(string iniFile, string category, string key, string defaultValue)
        {
            string returnString = new string(' ', 1024);
            GetPrivateProfileString(category, key, defaultValue, returnString, 1024, iniFile);
            return returnString.Split('\0')[0];
        }

        public static int WriteIni(string iniFile, string category, string key, string value)
        {
            return WritePrivateProfileString(category, key, value, iniFile);
        }
    }
}
