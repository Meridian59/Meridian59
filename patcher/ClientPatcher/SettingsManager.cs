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
        private string SettingsPath; //Path to JSON file settings.txt
        private string SettingsFile;

        public List<PatcherSettings> Servers { get; set; } //Loaded from settings.txt, or generated on first run and then saved.

        public SettingsManager()
        {
            SettingsPath = "%PROGRAMFILES%\\Open Meridian";
            SettingsFile = "\\settings.txt";
            SettingsPath = Environment.ExpandEnvironmentVariables(SettingsPath);
        }

        public void GetNewSettings()
        {
            try
            {
                WebClient myClient = new WebClient();
                List<PatcherSettings> webSettingsList =
                    JsonConvert.DeserializeObject<List<PatcherSettings>>(
                        myClient.DownloadString("http://ww1.openmeridian.org/settings.txt")); //Download the settings from the web, store them in a list.

                foreach (PatcherSettings currentSettings in Servers) //Loop through loaded settings from settings.txt
                {
                    PatcherSettings temp = webSettingsList.First(i => i.Guid == currentSettings.Guid); //Find the server loaded settings that match the local profile by Guid
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
            catch (Exception)
            {
                
                throw new System.InvalidOperationException("Unable to download settings from server.");
            }
            
        }

        public void LoadSettings()
        {
            if (File.Exists(SettingsPath + SettingsFile))
            {
                StreamReader file = File.OpenText(SettingsPath+SettingsFile); //Open the file

                Servers = JsonConvert.DeserializeObject<List<PatcherSettings>>(file.ReadToEnd()); //convert
                file.Close(); //close

                foreach (PatcherSettings patcherSettings in Servers)
                {//TODO was for the move to EC2, should probably be removed
                    patcherSettings.PatchBaseUrl = patcherSettings.PatchBaseUrl.Replace("build", "ww1");
                    patcherSettings.PatchInfoUrl = patcherSettings.PatchInfoUrl.Replace("build", "ww1");
                }

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
                using (StreamWriter sw = new StreamWriter(SettingsPath + SettingsFile)) //open file
                {
                    sw.Write(JsonConvert.SerializeObject(Servers, Formatting.Indented)); //write shit
                }
            }
            catch (Exception)
            {
                
                throw new Exception("Unable to SaveSettings()");
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
            try
            {
                DirectorySecurity dSecurity = new DirectorySecurity();
                dSecurity.AddAccessRule(new FileSystemAccessRule(new SecurityIdentifier(WellKnownSidType.WorldSid, null), FileSystemRights.Modify | FileSystemRights.Synchronize,
                                                                          InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                                                                          PropagationFlags.None, AccessControlType.Allow));
                dSecurity.SetAccessRuleProtection(false, true);
                Directory.CreateDirectory(SettingsPath, dSecurity);
            }
            catch (Exception)
            {
                
                throw new Exception("Unable to GrantAccess()");
            }
            
        }

    }
}
