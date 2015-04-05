using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Principal;
using Newtonsoft.Json;
using System.IO;
using System.Security.AccessControl;
using System.Net;

namespace ClientPatcher
{
    class SettingsManager
    {
        private readonly string _settingsPath; //Path to JSON file settings.txt
        private readonly string _settingsFile;

        public List<PatcherSettings> Servers { get; set; } //Loaded from settings.txt, or generated on first run and then saved.

        public SettingsManager()
        {
            _settingsPath = "%PROGRAMFILES%\\Open Meridian";
            _settingsFile = "\\settings.txt";
            _settingsPath = Environment.ExpandEnvironmentVariables(_settingsPath);
        }

        public void GetNewSettings()
        {
            try
            {
                var myClient = new WebClient();
                var webSettingsList =
                    JsonConvert.DeserializeObject<List<PatcherSettings>>(
                        myClient.DownloadString("http://ww1.openmeridian.org/settings.txt")); //Download the settings from the web, store them in a list.

                foreach (PatcherSettings currentSettings in Servers) //Loop through loaded settings from settings.txt
                {
                    PatcherSettings settings = currentSettings;
                    PatcherSettings temp = webSettingsList.First(i => i.Guid == settings.Guid); //Find the server loaded settings that match the local profile by Guid
                    if (temp != null) //If null, we have profiles to add from the remote server
                    {
                        if (currentSettings.PatchBaseUrl != temp.PatchBaseUrl ||
                            currentSettings.PatchInfoUrl != temp.PatchInfoUrl ||
                            currentSettings.ServerName != temp.ServerName) //If different
                        {
                            currentSettings.PatchBaseUrl = temp.PatchBaseUrl; //Update the server-side settings only
                            currentSettings.PatchInfoUrl = temp.PatchInfoUrl;
                            currentSettings.ServerName = temp.ServerName;
                        }
                        //Todo: add the new profilses from the server
                    }
                }
            }
            catch (Exception e)
            {
                throw new InvalidOperationException("Unable to download settings from server." + e);
            }
            
        }

        public void LoadSettings()
        {
            if (File.Exists(_settingsPath + _settingsFile))
            {
                StreamReader file = File.OpenText(_settingsPath+_settingsFile); //Open the file

                Servers = JsonConvert.DeserializeObject<List<PatcherSettings>>(file.ReadToEnd()); //convert
                file.Close(); //close

                //TODO GetNewSettings() goes here once the infrastructure to support it is in place
            }
            else
            {
                Servers = new List<PatcherSettings>();
                Servers.Add(new PatcherSettings(103)); //default entries, with "templates" defined in the class
                Servers.Add(new PatcherSettings(104));
                Servers.Add(new PatcherSettings(1));
                GrantAccess();
                SaveSettings();
            }
        }

        public void SaveSettings()
        {
            try
            {
                using (var sw = new StreamWriter(_settingsPath + _settingsFile)) //open file
                {
                    sw.Write(JsonConvert.SerializeObject(Servers, Formatting.Indented)); //write shit
                }
            }
            catch (Exception e)
            {
                
                throw new Exception("Unable to SaveSettings()" + e);
            }
            
        }

        //used when adding from form
        public void AddProfile(string clientfolder, string patchbaseurl, string patchinfourl, string servername, bool isdefault)
        {
            var ps = new PatcherSettings
            {
                ClientFolder = clientfolder,
                PatchBaseUrl = patchbaseurl,
                PatchInfoUrl = patchinfourl,
                ServerName = servername,
                Default = isdefault
            };

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
            try
            {
                var dSecurity = new DirectorySecurity();
                dSecurity.AddAccessRule(new FileSystemAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), FileSystemRights.Modify | FileSystemRights.Synchronize,
                                                                          InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                                                                          PropagationFlags.None, AccessControlType.Allow));
                dSecurity.SetAccessRuleProtection(false, true);
                Directory.CreateDirectory(_settingsPath, dSecurity);
            }
            catch (Exception e)
            {
                
                throw new Exception("Unable to GrantAccess()" + e);
            }
            
        }

    }
}
