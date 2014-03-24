using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using Newtonsoft.Json;
using PatchListGenerator;
using System.IO;
using System.Diagnostics;
using System.Security.Permissions;
using System.Security.AccessControl;

namespace ClientPatcher
{
    //Event when we Scan a File, used to notify UI.
    public delegate void ScanFileEventHandler(object sender, ScanEventArgs e);
    //Event when we Start a Download, used to notify UI.
    public delegate void StartDownloadEventHandler(object sender, StartDownloadEventArgs e);
    //Event when we Make progress in a Download, used to notify UI.
    public delegate void ProgressDownloadEventHandler(object sender, DownloadProgressChangedEventArgs e);
    //Event when we Complete a Download, used to notify UI.
    public delegate void EndDownloadEventHandler(object sender, EndDownloadEventArgs e);

    class ClientPatcher
    {
        private string PatchInfoJason = "";

        public List<ManagedFile> PatchFiles; //Loaded from the web server at PatchInfoURL
        public List<ManagedFile> LocalFiles; //Loaded with files that do NOT match

        public PatcherSettings CurrentProfile { get; set; }

        private long PatchTotalSize = 0;

        //Event when we Scan a File, used to notify UI.
        public event ScanFileEventHandler FileScanned;
        protected virtual void OnFileScan(ScanEventArgs e)
        {
            if (FileScanned != null)
                FileScanned(this, e);
        }

        //Event when we Start a Download, used to notify UI.
        public event StartDownloadEventHandler StartedDownload;
        protected virtual void OnStartDownload(StartDownloadEventArgs e)
        {
            if (StartedDownload != null)
                StartedDownload(this, e);
        }

        //Event when we Make progress in a Download, used to notify UI.
        public event ProgressDownloadEventHandler ProgressedDownload;
        protected virtual void OnProgressedDownload(DownloadProgressChangedEventArgs e)
        {
            if (ProgressedDownload != null)
                ProgressedDownload(this, e);
        }

        //Event when we Complete a Download, used to notify UI.
        public event EndDownloadEventHandler EndedDownload;
        protected virtual void OnEndDownload(EndDownloadEventArgs e)
        {
            if (EndedDownload != null)
                EndedDownload(this, e);
        }

        public ClientPatcher()
        {
            LocalFiles = new List<ManagedFile>();
        }

        public ClientPatcher(PatcherSettings settings)
        {
            LocalFiles = new List<ManagedFile>();
            CurrentProfile = settings;
        }

        public int DownloadJson()
        {
            WebClient wc = new WebClient();
            try
            {
                PatchInfoJason = wc.DownloadString(CurrentProfile.PatchInfoURL);
                PatchFiles = JsonConvert.DeserializeObject<List<ManagedFile>>(PatchInfoJason);
                return 1;
            }
            catch (WebException e)
            {
                Console.WriteLine("WebException Handler: {0}", e.ToString());
                return 0;
            }
        }

        private bool IsNewClient()
        {
            return !File.Exists(CurrentProfile.ClientFolder + "\\meridian.ini");
        }

        private void CreateFolderStructure()
        {
            Directory.CreateDirectory(CurrentProfile.ClientFolder);
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\resource\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\download\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\help\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\mail\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\ads\\");
        }

        private void CreateDefaultINI()
        {
            const string DefaultINI = @"[Comm]
ServerNumber=103
[Miscellaneous]
UserName=username
Download=10016";
            using (StreamWriter sw = new StreamWriter(CurrentProfile.ClientFolder + "\\meridian.ini"))
            {
                sw.Write(DefaultINI);
            }
        }

        private void CreateNewClient()
        {
            CreateFolderStructure();
            CreateDefaultINI();
        }

        public void ScanClient()
        {
            string fullpath;
            if (IsNewClient())
            {
                CreateNewClient();
            }

            foreach (ManagedFile PatchFile in PatchFiles)
            {
                fullpath = CurrentProfile.ClientFolder + PatchFile.basepath + PatchFile.filename;
                FileScanned(this, new ScanEventArgs(PatchFile.filename)); //Tells the form to update the progress bar
                ManagedFile LocalFile = new ManagedFile(fullpath);
                LocalFile.ComputeHash();
                if (PatchFile.myHash != LocalFile.myHash)
                {
                    LocalFiles.Add(LocalFile);
                    LocalFile.Length = PatchFile.Length;
                    PatchTotalSize += PatchFile.Length;
                }
            }
        }

        public void DownloadFiles()
        {
            WebClient client = new WebClient();

            foreach (ManagedFile File in LocalFiles)
            {
                string temp = File.basepath.Replace("\\", "/");
                try
                {
                    StartedDownload(this, new StartDownloadEventArgs(File.filename, File.Length));
                    //TODO: replace with Async downloader, send download progress report to UI
                    client.DownloadFile(CurrentProfile.PatchBaseURL + temp + File.filename, CurrentProfile.ClientFolder + File.basepath + File.filename);
                }
                catch (WebException e)
                {
                    Console.WriteLine("WebException Handler: {0}", e.ToString());
                    return;
                }
            }
        }

        public void DownloadFilesAsync()
        {
            WebClient client = new WebClient();
            foreach (ManagedFile File in LocalFiles)
            {
                string temp = File.basepath.Replace("\\", "/");
                try
                {
                    StartedDownload(this, new StartDownloadEventArgs(File.filename, File.Length));
                    client.DownloadProgressChanged += new DownloadProgressChangedEventHandler(client_DownloadProgressChanged);
                    client.DownloadFileAsync(new Uri(CurrentProfile.PatchBaseURL + temp + File.filename), CurrentProfile.ClientFolder + File.basepath + File.filename);
                }
                catch (WebException e)
                {
                    Console.WriteLine(String.Format("Exception: {0}", e.ToString()));
                }
            }
        }

        private void client_DownloadProgressChanged(object sender, DownloadProgressChangedEventArgs e)
        {
            OnProgressedDownload(e);
        }

        private void client_DownloadFileCompleted(object sender, DownloadDataCompletedEventArgs e)
        {

        }

    }

    public class ScanEventArgs : EventArgs
    {
        private string filename;
        public string Filename
        {
            get
            { 
                return this.filename; 
            }
        }

        public ScanEventArgs(string filename)
        {
            this.filename = filename;
        }
    }

    public class StartDownloadEventArgs : EventArgs
    {
        private long filesize;
        public long Filesize
        {
            get
            {
                return this.filesize;
            }
        }

        private string filename;
        public string Filename
        {
            get
            {
                return this.filename;
            }
        }

        public StartDownloadEventArgs(string filename, long filesize)
        {
            this.filename = filename;
            this.filesize = filesize;
        }
    }

    public class EndDownloadEventArgs : EventArgs
    {
    }
}
