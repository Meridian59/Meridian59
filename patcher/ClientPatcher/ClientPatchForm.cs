using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using Newtonsoft.Json;
using PatchListGenerator;
using System.IO;
using System.Diagnostics;

namespace ClientPatcher
{
    enum ChangeType
    {
        NONE = 0,
        ADD_PROFILE = 1,
        MOD_PROFILE = 2
    }

    public partial class ClientPatchForm : Form
    {
        private string PatchInfoURL { get; set; } //URL with JSON Patch information
        private string ClientFolder { get; set; } //Folder to save files in
        private string PatchBaseURL { get; set; } //URL to download files from
        private string PatchJason { get; set; } //Probably doesnt need to be global, used to store the JSON downloaded before converting to a list.

        ChangeType changetype = ChangeType.NONE;
        
        private long PatchTotalSize = 0;
        

        private List<ManagedFile> PatchFiles; //Loaded from the web server at PatchInfoURL
        private List<ManagedFile> LocalFiles; //Loaded with files that do NOT match

        private string SettingsPath; //Path to JSON file settings.txt
        private List<PatcherSettings> Servers; //Loaded from settings.txt, or generated on first run and then saved.

        private int DownloadJson()
        {
            WebClient wc = new WebClient();
            try
            {
                PatchJason = wc.DownloadString(PatchInfoURL);
                PatchFiles = JsonConvert.DeserializeObject<List<ManagedFile>>(PatchJason);
                return 1;
            }
            catch (WebException e)
            {
                Console.WriteLine("WebException Handler: {0}", e.ToString());
                lblStatus.Text = "Unable to download PatchInfo from: " + PatchInfoURL;
                return 0;
            }

        }

        public ClientPatchForm()
        {
            InitializeComponent();
        }

        public void LoadSettings()
        {
            ddlServer.Items.Clear();
            SettingsPath = Directory.GetCurrentDirectory() + "\\settings.txt";
            if (File.Exists(SettingsPath))
            {
                StreamReader file = File.OpenText(SettingsPath);
                JsonSerializer js = new JsonSerializer();
                //ps = JsonConvert.DeserializeObject<PatcherSettings>(file.ReadToEnd());
                Servers = JsonConvert.DeserializeObject<List<PatcherSettings>>(file.ReadToEnd());
                file.Close();
                foreach (PatcherSettings settings in Servers)
                {
                    ddlServer.Items.Add(settings.ServerName);
                }
            }
            else
            {
                Servers = new List<PatcherSettings>();
                Servers.Add(new PatcherSettings(103));
                ddlServer.Items.Add(Servers[0].ServerName);
                Servers.Add(new PatcherSettings(104));
                ddlServer.Items.Add(Servers[1].ServerName);
                SaveSettings();
            }
            PatcherSettings Default = Servers.Find(x => x.Default == true);
            int index = Servers.FindIndex(x => x.Default == true);
            if (Default != null)
            {
                PatchInfoURL = Default.PatchInfoURL;
                ClientFolder = Default.ClientFolder;
                PatchBaseURL = Default.PatchBaseURL;
                ddlServer.SelectedIndex = index;
                lblStatus.Text = String.Format("Server {0} selected. Client located at: {1}", Default.ServerName, Default.ClientFolder);
            }
        }

        private void SaveSettings()
        {
            using (StreamWriter sw = new StreamWriter(SettingsPath))
            {
                sw.Write(JsonConvert.SerializeObject(Servers, Formatting.Indented));
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            PatchJason = "";
            LocalFiles = new List<ManagedFile>();
            btnPlay.Enabled = false;

            LoadSettings();
            
        }

        private void ScanClient()
        {
            string fullpath;
            lblStatus.Text = "Scanning existing content....";
            lblStatus.Update();
            pbProgress.Maximum = PatchFiles.Count;
            foreach (ManagedFile PatchFile in PatchFiles)
            {
                fullpath = ClientFolder + PatchFile.basepath + PatchFile.filename; 
                ManagedFile LocalFile = new ManagedFile(fullpath);
                LocalFile.ComputeHash();
                if (PatchFile.myHash != LocalFile.myHash)
                {
                    LocalFiles.Add(LocalFile);
                    LocalFile.Length = PatchFile.Length;
                    PatchTotalSize += PatchFile.Length;
                }
                pbProgress.PerformStep();
                lblStatus.Text = "Scanning existing content...." + PatchFile.filename;
                lblStatus.Update();
            }
        }

        private void DownloadFiles()
        {
            WebClient client = new WebClient();

            pbProgress.Value = 0;
            pbProgress.Maximum = LocalFiles.Count;
            pbProgress.Update();
            lblStatus.Text = "Downloading Content....";
            lblStatus.Update();

            foreach (ManagedFile File in LocalFiles)
            {
                lblStatus.Text = "Downloading Content...." + File.filename + "(" + (File.Length/1024) + "KB)" ;
                lblStatus.Update();
                this.Update();
                string temp = File.basepath.Replace("\\", "/");
                try
                {
                    client.DownloadFile(PatchBaseURL + temp + File.filename, ClientFolder + File.basepath + File.filename);
                }
                catch (WebException e)
                {
                    Console.WriteLine("WebException Handler: {0}", e.ToString());
                    lblStatus.Text = "Unable to download file from: " + PatchBaseURL + temp + File.filename;
                    return;
                }
                pbProgress.PerformStep();
            }
            lblStatus.Text = "Patching Complete!";
            lblStatus.Update();
            btnPlay.Enabled = true;
        }

        private void btnPlay_Click(object sender, EventArgs e)
        {
            ProcessStartInfo meridian = new ProcessStartInfo();
            meridian.FileName = ClientFolder + "\\meridian.exe";
            meridian.WorkingDirectory = ClientFolder + "\\";
            Process.Start(meridian);
            Application.Exit();
        }

        private void ddlServer_SelectionChangeCommitted(object sender, EventArgs e)
        {
            //lblStatus.Text = ddlServer.SelectedItem.ToString();
            PatcherSettings selected = Servers.Find(x => x.ServerName == ddlServer.SelectedItem.ToString());
            if (selected != null)
            {
                PatchInfoURL = selected.PatchInfoURL;
                ClientFolder = selected.ClientFolder;
                PatchBaseURL = selected.PatchBaseURL;
                lblStatus.Text = String.Format("Server {0} selected. Client located at: {1}", selected.ServerName, selected.ClientFolder);
                btnPlay.Enabled = false;
            }
        }

        private void btnPatch_Click(object sender, EventArgs e)
        {
            lblStatus.Text = "Downloading Patch Information....";
            if (DownloadJson() == 1)
            {
                ScanClient();
                if (LocalFiles.Count > 0)
                    DownloadFiles();
                else
                {
                    lblStatus.Text = "Patching Complete!";
                    lblStatus.Update();
                    btnPlay.Enabled = true;
                }
            }
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            groupProfileSettings.Enabled = true;
            changetype = ChangeType.ADD_PROFILE;
        }

        private void btnStartModify_Click(object sender, EventArgs e)
        {
            groupProfileSettings.Enabled = true;
            int selected = Servers.FindIndex(x => x.ServerName == ddlServer.SelectedItem.ToString());
            txtClientFolder.Text = Servers[selected].ClientFolder;
            txtPatchBaseURL.Text = Servers[selected].PatchBaseURL;
            txtPatchInfoURL.Text = Servers[selected].PatchInfoURL;
            txtServerName.Text = Servers[selected].ServerName;
            cbDefaultServer.Checked = Servers[selected].Default;
            changetype = ChangeType.MOD_PROFILE;
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            switch (changetype)
            {
                case ChangeType.ADD_PROFILE:
                    AddProfile();
                    groupProfileSettings.Enabled = false;
                    break;
                case ChangeType.MOD_PROFILE:
                    ModProfile();
                    groupProfileSettings.Enabled = false;
                    break;
                default:
                    break;
            }
        }

        private void AddProfile()
        {
            PatcherSettings ps = new PatcherSettings();
            if (txtClientFolder.Text == "" |
                txtPatchBaseURL.Text == "" |
                txtPatchInfoURL.Text == "" |
                txtServerName.Text == "")
            {
                lblStatus.Text = "Please fill out the fields before Saving";
                return;
            }
            ps.ClientFolder = txtClientFolder.Text;
            ps.PatchBaseURL = txtPatchBaseURL.Text;
            ps.PatchInfoURL = txtPatchInfoURL.Text;
            ps.ServerName = txtServerName.Text;
            ps.Default = cbDefaultServer.Checked;

            Servers.Add(ps);
            changetype = ChangeType.NONE;
            SaveSettings();
            LoadSettings();
        }

        private void ModProfile()
        {
            int selected = Servers.FindIndex(x => x.ServerName == ddlServer.SelectedItem.ToString());
            Servers[selected].ClientFolder = txtClientFolder.Text;
            Servers[selected].PatchBaseURL = txtPatchBaseURL.Text;
            Servers[selected].PatchInfoURL = txtPatchInfoURL.Text;
            Servers[selected].ServerName = txtServerName.Text;
            Servers[selected].Default = cbDefaultServer.Checked;
            changetype = ChangeType.NONE;
            SaveSettings();
            LoadSettings();
        }
    }
}
