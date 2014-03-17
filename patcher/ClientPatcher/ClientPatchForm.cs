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
using PatchServer;
using System.IO;
using System.Diagnostics;

namespace ClientPatcher
{
    public partial class ClientPatchForm : Form
    {
        private string PatchInfoURL { get; set; }
        private string ClientFolder { get; set; }
        private string PatchBaseURL { get; set; }
        private string PatchJason { get; set; }
        
        private long PatchTotalSize = 0;
        
        private List<ManagedFile> PatchFiles;
        private List<ManagedFile> LocalFiles;

        private List<PatcherSettings> Servers;

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
            string path = Directory.GetCurrentDirectory();
            if (File.Exists(path + "\\settings.txt"))
            {
                StreamReader file = File.OpenText(path + "\\settings.txt");
                JsonSerializer js = new JsonSerializer();
                //ps = JsonConvert.DeserializeObject<PatcherSettings>(file.ReadToEnd());
                Servers = JsonConvert.DeserializeObject<List<PatcherSettings>>(file.ReadToEnd());
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
                using (StreamWriter sw = new StreamWriter(path + "\\settings.txt"))
                {
                    sw.Write(JsonConvert.SerializeObject(Servers, Formatting.Indented));
                }
            }
            PatcherSettings Default = Servers.Find(x => x.Default == true);
            int index = Servers.FindIndex(x => x.Default == true);
            if (Default != null)
            {
                PatchInfoURL = Default.PatchInfoURL;
                ClientFolder = Default.ClientFolder;
                PatchBaseURL = Default.PatchBaseURL;
                ddlServer.SelectedIndex = index;
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
                client.DownloadFile(PatchBaseURL + temp + File.filename, ClientFolder + File.basepath + File.filename);
                pbProgress.PerformStep();
            }
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

        private void btnRetry_Click(object sender, EventArgs e)
        {
            btnRetry.Visible = false;
            DownloadJson();
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
                DownloadFiles();
            }
        }
    }
}
