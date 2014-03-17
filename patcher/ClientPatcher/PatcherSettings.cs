using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;
using System.IO;

namespace ClientPatcher
{
    public class PatcherSettings
    {
        public string ServerName { get; set; }
        public string PatchInfoURL { get; set; }
        public string ClientFolder { get; set; }
        public string PatchBaseURL { get; set; }
        public bool Default { get; set; }

        public PatcherSettings()
        {
            ServerName = "103";
            PatchInfoURL = "http://build.openmeridian.org/103/patchinfo.txt";
            ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 103";
            PatchBaseURL = "http://build.openmeridian.org/103/clientpatch";
            Default = true;
        }

        public PatcherSettings(int template)
        {
            switch (template)
            {
                case 104:
                    ServerName = "104";
                    PatchInfoURL = "http://build.openmeridian.org/104/patchinfo.txt";
                    ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 103";
                    PatchBaseURL = "http://build.openmeridian.org/104/clientpatch";
                    Default = false;
                    break;
                
                case 103:
                default:
                    ServerName = "103";
                    PatchInfoURL = "http://build.openmeridian.org/103/patchinfo.txt";
                    ClientFolder = "C:\\Program Files\\Open Meridian\\Meridian 103";
                    PatchBaseURL = "http://build.openmeridian.org/103/clientpatch";
                    Default = true;
                    break;
            }
        }

        public PatcherSettings(string servername, string patchinfourl, string clientfolder, string patchbaseurl)
        {
            ServerName = servername;
            PatchInfoURL = patchinfourl;
            ClientFolder = clientfolder;
            PatchBaseURL = patchbaseurl;
            Default = false;
        }

        public PatcherSettings(string servername, string patchinfourl, string clientfolder, string patchbaseurl, bool defaultserver)
        {
            ServerName = servername;
            PatchInfoURL = patchinfourl;
            ClientFolder = clientfolder;
            PatchBaseURL = patchbaseurl;
            Default = defaultserver;
        }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this, Formatting.Indented);
        }
    }
}
