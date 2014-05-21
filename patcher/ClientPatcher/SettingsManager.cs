using System;
using System.Collections.Generic;
using System.Security.Principal;
using Newtonsoft.Json;
using System.IO;
using System.Security.AccessControl;

namespace ClientPatcher
{
    class SettingsManager
    {
        private string SettingsPath; //Path to JSON file settings.txt
        private string SettingsFile;

        public List<PatcherSettings> Servers { get; set; } //Loaded from settings.txt, or generated on first run and then saved.

        public SettingsManager()
        {
            SettingsPath = "%PROGRAMFILES%\\Open Meridian";
            SettingsFile = "\\settings.txt";
            SettingsPath = Environment.ExpandEnvironmentVariables(SettingsPath);
        }

        public void LoadSettings()
        {
            if (File.Exists(SettingsPath))
            {
                StreamReader file = File.OpenText(SettingsPath+SettingsFile); //Open the file

                JsonSerializer js = new JsonSerializer(); //Object we use to convert txt
                Servers = JsonConvert.DeserializeObject<List<PatcherSettings>>(file.ReadToEnd()); //convert
                file.Close(); //close
            }
            else
            {
                Servers = new List<PatcherSettings>();
                Servers.Add(new PatcherSettings(103)); //default entries, with "templates" defined in the class
                Servers.Add(new PatcherSettings(104));
                GrantAccess();
                SaveSettings();
            }
        }
        public void SaveSettings()
        {
            using (StreamWriter sw = new StreamWriter(SettingsPath + SettingsFile)) //open file
            {
                sw.Write(JsonConvert.SerializeObject(Servers, Formatting.Indented)); //write shit
            }
        }
        //used when adding from form
        public void AddProfile(string clientfolder, string patchbaseurl, string patchinfourl, string servername, bool isdefault)
        {
            PatcherSettings ps = new PatcherSettings();
            ps.ClientFolder = clientfolder;
            ps.PatchBaseUrl = patchbaseurl;
            ps.PatchInfoUrl = patchinfourl;
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
            return Servers.Find(x => x.Default);
        }
        private void GrantAccess()
        {
            DirectorySecurity dSecurity = new DirectorySecurity();
            dSecurity.AddAccessRule(new FileSystemAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), FileSystemRights.Modify | FileSystemRights.Synchronize,
                                                                      InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                                                                      PropagationFlags.None, AccessControlType.Allow));
            dSecurity.SetAccessRuleProtection(false, true);
            Directory.CreateDirectory(SettingsPath, dSecurity);
        }

    }
}
