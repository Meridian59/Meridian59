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

        /// <summary>
        /// uses kernel32.dll to read from an ini file
        /// </summary>
        /// <param name="lpAppName">ini file section</param>
        /// <param name="lpKeyName">key</param>
        /// <param name="lpDefault">default value if key not found in file</param>
        /// <param name="lpReturnString">string to hold return</param>
        /// <param name="nSize">size of return string</param>
        /// <param name="lpFilename">filename</param>
        /// <returns>status int</returns>
        [DllImport("KERNEL32.DLL",
        EntryPoint = "GetPrivateProfileStringW",
        SetLastError = true,
        CharSet = CharSet.Unicode,
        ExactSpelling = true,
        CallingConvention = CallingConvention.StdCall)]
        private static extern int GetPrivateProfileString(
          string lpAppName,
          string lpKeyName,
          string lpDefault,
          string lpReturnString,
          int nSize,
          string lpFilename);

        /// <summary>
        /// Uses kernel32.dll to write to ini files
        /// </summary>
        /// <param name="lpAppName">ini file section</param>
        /// <param name="lpKeyName">key</param>
        /// <param name="lpString">value</param>
        /// <param name="lpFilename">filename</param>
        /// <returns>status int</returns>
        [DllImport("KERNEL32.DLL",
        EntryPoint = "WritePrivateProfileStringW",
        SetLastError = true,
        CharSet = CharSet.Unicode,
        ExactSpelling = true,
        CallingConvention = CallingConvention.StdCall)]
        private static extern int WritePrivateProfileString(
        string lpAppName,
        string lpKeyName,
        string lpString,
        string lpFilename);

        /// <summary>
        /// gets a specific key's value from an ini file
        /// </summary>
        /// <param name="iniFile">filename</param>
        /// <param name="category">ini section</param>
        /// <param name="key">key</param>
        /// <param name="defaultValue">default value if not found</param>
        /// <returns>a string containing the key value, or default if none found</returns>
        public static string GetIni(string iniFile, string category, string key, string defaultValue)
        {
            string returnString = new string(' ', 1024);
            GetPrivateProfileString(category, key, defaultValue, returnString, 1024, iniFile);
            return returnString.Split('\0')[0];
        }

        /// <summary>
        /// Writes to a specific key in an ini file
        /// </summary>
        /// <param name="iniFile">filename</param>
        /// <param name="category">ini section</param>
        /// <param name="key">key</param>
        /// <param name="value">value</param>
        /// <returns>status int</returns>
        public static int WriteIni(string iniFile, string category, string key, string value)
        {
            return WritePrivateProfileString(category, key, value, iniFile);
        }
    }
}
