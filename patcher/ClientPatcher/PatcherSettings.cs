using System;
using Newtonsoft.Json;

namespace ClientPatcher
{
    public class PatcherSettings
    {
        public string ServerName { get; set; }     //What do we call this profile?
        public string PatchInfoUrl { get; set; }   //Where is the file containing md5 hashes to compare?
        public string ClientFolder { get; set; }   //Where is the local copy of the client?
        public string PatchBaseUrl { get; set; }   //Where to download individual files?
        public string Guid { get; set; }           //Will be used to get updated settings from server
        public string FullInstallUrl { get; set; } //Path to a .zip file of the full client to download for first run
        public bool Default { get; set; }          //Is this profile the default-selected at start up?

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
                case 1:
                    ServerName = "Korea 1";
                    PatchInfoUrl = "http://m59.iptime.org/1/patchinfo.txt";
                    ClientFolder = "%PROGRAMFILES%\\Open Meridian\\Meridian 1";
                    ClientFolder = Environment.ExpandEnvironmentVariables(ClientFolder);
                    PatchBaseUrl = "http://m59.iptime.org/1/clientpatch";
                    Guid = "3B89295C-F19C-46C3-8B8A-2F51F2C3A8C9";
                    Default = false;
                    break;

                case 104:
                    ServerName = "104";
                    PatchInfoUrl = "http://ww1.openmeridian.org/104/patchinfo.txt";
                    ClientFolder = "%PROGRAMFILES%\\Open Meridian\\Meridian 104";
                    ClientFolder = Environment.ExpandEnvironmentVariables(ClientFolder);
                    PatchBaseUrl = "http://ww1.openmeridian.org/104/clientpatch";
                    Guid = "EACFDF63-65A6-46C3-AC99-1C5BAB07EDEB";
                    Default = false;
                    break;
                
                //case 103:
                default:
                    ServerName = "103";
                    PatchInfoUrl = "http://ww1.openmeridian.org/103/patchinfo.txt";
                    ClientFolder = "%PROGRAMFILES%\\Open Meridian\\Meridian 103";
                    ClientFolder = Environment.ExpandEnvironmentVariables(ClientFolder);
                    PatchBaseUrl = "http://ww1.openmeridian.org/103/clientpatch";
                    Guid = "5AD1FB01-A84A-47D1-85B8-5F85FB0C201E";
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
