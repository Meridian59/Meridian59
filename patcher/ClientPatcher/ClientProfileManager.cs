using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ClientPatcher
{
    class ClientProfileManager
    {
        private string SettingsPath; //Path to JSON file settings.txt
        private List<PatcherSettings> Servers; //Loaded from settings.txt, or generated on first run and then saved.

        public void LoadSettings()
        {
            SettingsPath = Directory.GetCurrentDirectory() + "\\settings.txt";
            if (File.Exists(SettingsPath))
            {
                StreamReader file = File.OpenText(SettingsPath);
                JsonSerializer js = new JsonSerializer();
                //ps = JsonConvert.DeserializeObject<PatcherSettings>(file.ReadToEnd());
                Servers = JsonConvert.DeserializeObject<List<PatcherSettings>>(file.ReadToEnd());
                file.Close();
            }
            else
            {
                Servers = new List<PatcherSettings>();
                Servers.Add(new PatcherSettings(103));
                Servers.Add(new PatcherSettings(104));
                SaveSettings();
            }
            PatcherSettings DefaultProfile = Servers.Find(x => x.Default == true);
            int index = Servers.FindIndex(x => x.Default == true);
            if (DefaultProfile != null)
            {
                CurrentProfile = DefaultProfile;
            }
        }

        private void SaveSettings()
        {
            using (StreamWriter sw = new StreamWriter(SettingsPath))
            {
                sw.Write(JsonConvert.SerializeObject(Servers, Formatting.Indented));
            }
        }

        public void AddProfile(string clientfolder, string patchbaseurl, string patchinfourl, string servername, bool isdefault)
        {
            PatcherSettings ps = new PatcherSettings();
            ps.ClientFolder = clientfolder;
            ps.PatchBaseURL = patchbaseurl;
            ps.PatchInfoURL = patchinfourl;
            ps.ServerName = servername;
            ps.Default = isdefault;

            Servers.Add(ps);
            SaveSettings();
            LoadSettings();
        }

        public void AddProfile(PatcherSettings newprofile)
        {
            Servers.Add(newprofile);
            SaveSettings();
            LoadSettings();
        }

    }
}
