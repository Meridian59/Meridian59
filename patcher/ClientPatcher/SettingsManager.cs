using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using Newtonsoft.Json;
using PatchListGenerator;
using System.IO;
using System.Diagnostics;

namespace ClientPatcher
{
    class SettingsManager
    {
        private string SettingsPath; //Path to JSON file settings.txt
        public List<PatcherSettings> Servers { get; set; } //Loaded from settings.txt, or generated on first run and then saved.

        public SettingsManager()
        {
            SettingsPath = Directory.GetCurrentDirectory() + "\\settings.txt";
        }

        public void LoadSettings()
        {
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
        }

        public void SaveSettings()
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

        public PatcherSettings FindByName(string name)
        {
            return Servers.Find(x => x.ServerName == name);
        }

        public PatcherSettings GetDefault()
        {
            return Servers.Find(x => x.Default == true);
        }

    }
}
