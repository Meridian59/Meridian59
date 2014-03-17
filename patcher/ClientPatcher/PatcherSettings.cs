using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;
using System.IO;

namespace ClientPatcher
{
    class PatcherSettings
    {
        public string MyProperty { get; set; }
        public string PatchInfoURL { get; set; }
        public string ClientFolder { get; set; }
        public string PatchBaseURL { get; set; }

        public PatcherSettings()
        {
            PatchInfoURL = "http://localhost/patchinfo.txt";
            ClientFolder = "C:\\Users\\dstone\\Downloads\\meridian_103";
            PatchBaseURL = "http://localhost/clientpatch";
        }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this, Formatting.Indented);
        }
    }
}
