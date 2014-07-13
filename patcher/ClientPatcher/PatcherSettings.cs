using System;
using Newtonsoft.Json;

namespace ClientPatcher
{
    public class PatcherSettings
    {
        public string ServerName { get; set; }
        public string PatchInfoUrl { get; set; }
        public string ClientFolder { get; set; }
        public string PatchBaseUrl { get; set; }
        public bool Default { get; set; }

        public PatcherSettings()
        {
            ServerName = "103";
            PatchInfoUrl = "http://ww1.openmeridian.org/103/patchinfo.txt";
            ClientFolder = "%PROGRAMFILES%\\Open Meridian\\Meridian 103";
            ClientFolder = Environment.ExpandEnvironmentVariables(ClientFolder);
            PatchBaseUrl = "http://ww1.openmeridian.org/103/clientpatch";
            Default = true;
        }

        public PatcherSettings(int template)
        {
            switch (template)
            {
                case 104:
                    ServerName = "104";
                    PatchInfoUrl = "http://ww1.openmeridian.org/104/patchinfo.txt";
                    ClientFolder = "%PROGRAMFILES%\\Open Meridian\\Meridian 104";
                    ClientFolder = Environment.ExpandEnvironmentVariables(ClientFolder);
                    PatchBaseUrl = "http://ww1.openmeridian.org/104/clientpatch";
                    Default = false;
                    break;
                
                case 103:
                default:
                    ServerName = "103";
                    PatchInfoUrl = "http://ww1.openmeridian.org/103/patchinfo.txt";
                    ClientFolder = "%PROGRAMFILES%\\Open Meridian\\Meridian 103";
                    ClientFolder = Environment.ExpandEnvironmentVariables(ClientFolder);
                    PatchBaseUrl = "http://ww1.openmeridian.org/103/clientpatch";
                    Default = true;
                    break;
            }
        }

        public PatcherSettings(string servername, string patchinfourl, string clientfolder, string patchbaseurl)
        {
            ServerName = servername;
            PatchInfoUrl = patchinfourl;
            ClientFolder = clientfolder;
            PatchBaseUrl = patchbaseurl;
            Default = false;
        }

        public PatcherSettings(string servername, string patchinfourl, string clientfolder, string patchbaseurl, bool defaultserver)
        {
            ServerName = servername;
            PatchInfoUrl = patchinfourl;
            ClientFolder = clientfolder;
            PatchBaseUrl = patchbaseurl;
            Default = defaultserver;
        }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this, Formatting.Indented);
        }
    }
}
