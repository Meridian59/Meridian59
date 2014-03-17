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
        private string PatchInfoURL;
        private string ClientFolder;
        private string PatchBaseURL;
        private string PatchJason = "";
        
        private long PatchTotalSize = 0;
        
        private List<ManagedFile> PatchFiles;
        private List<ManagedFile> LocalFiles;

        private void DownloadJson()
        {
            WebClient wc = new WebClient();
            PatchJason = wc.DownloadString(PatchInfoURL);
        }

        public ClientPatchForm()
        {
            InitializeComponent();
        }

        public void LoadSettings()
        {
            PatcherSettings ps;
            string path = Directory.GetCurrentDirectory();
            if (File.Exists(path + "\\settings.txt"))
            {
                StreamReader file = File.OpenText(path + "\\settings.txt");
                JsonSerializer js = new JsonSerializer();
                ps = JsonConvert.DeserializeObject<PatcherSettings>(file.ReadToEnd());
            }
            else
            {
                ps = new PatcherSettings();
                using (StreamWriter sw = new StreamWriter(path + "\\settings.txt"))
                {
                    sw.Write(ps.ToJson());
                }
            }
            PatchInfoURL = ps.PatchInfoURL;
            ClientFolder = ps.ClientFolder;
            PatchBaseURL = ps.PatchBaseURL;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            LocalFiles = new List<ManagedFile>();
            timer1.Start();
            btnPlay.Enabled = false;

            LoadSettings();
            
            lblStatus.Text = "Downloading Patch Information....";
            DownloadJson();
            PatchFiles = JsonConvert.DeserializeObject<List<ManagedFile>>(PatchJason);
            
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
            DownloadFiles();
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

        private void timer1_Tick(object sender, EventArgs e)
        {
            timer1.Enabled = false;
            ScanClient();
        }

        private void btnPlay_Click(object sender, EventArgs e)
        {
            ProcessStartInfo meridian = new ProcessStartInfo();
            meridian.FileName = ClientFolder + "\\meridian.exe";
            meridian.WorkingDirectory = ClientFolder + "\\";
            Process.Start(meridian);
            Application.Exit();
        }

    }
}
