using System;
using System.Collections.Generic;
using System.Net;
using Newtonsoft.Json;
using PatchListGenerator;
using System.IO;
using System.ComponentModel;
using System.Threading;

namespace ClientPatcher
{
    #region Event Delegates and Args
    //Event when we Scan a File, used to notify UI.
    public delegate void ScanFileEventHandler(object sender, ScanEventArgs e);
    public class ScanEventArgs : EventArgs
    {
        private string filename;
        public string Filename
        {
            get
            {
                return filename;
            }
        }

        public ScanEventArgs(string filename)
        {
            this.filename = filename;
        }
    }
    //Event when we Start a Download, used to notify UI.
    public delegate void StartDownloadEventHandler(object sender, StartDownloadEventArgs e);
    public class StartDownloadEventArgs : EventArgs
    {
        private long filesize;
        public long Filesize
        {
            get
            {
                return filesize;
            }
        }

        private string filename;
        public string Filename
        {
            get
            {
                return filename;
            }
        }

        public StartDownloadEventArgs(string filename, long filesize)
        {
            this.filename = filename;
            this.filesize = filesize;
        }
    }
    //Event when we Make progress in a Download, used to notify UI.
    public delegate void ProgressDownloadEventHandler(object sender, DownloadProgressChangedEventArgs e);
    //Event when we Complete a Download, used to notify UI.
    public delegate void EndDownloadEventHandler(object sender, AsyncCompletedEventArgs e);

    #endregion

    class ClientPatcher
    {
        private string _patchInfoJason = "";

        public List<ManagedFile> PatchFiles; //Loaded from the web server at PatchInfoURL
        public List<ManagedFile> LocalFiles; //Loaded with files that do NOT match
        public WebClient MyWebClient;

        bool _continueAsync;

        public PatcherSettings CurrentProfile { get; set; }

        private long _patchTotalSize;

        #region Events
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
        protected virtual void OnEndDownload(AsyncCompletedEventArgs e)
        {
            if (EndedDownload != null)
                EndedDownload(this, e);
        }
        #endregion

        public ClientPatcher()
        {
            LocalFiles = new List<ManagedFile>();
            MyWebClient = new WebClient();
        }
        public ClientPatcher(PatcherSettings settings)
        {
            LocalFiles = new List<ManagedFile>();
            MyWebClient = new WebClient();
            CurrentProfile = settings;
        }
        public int DownloadJson()
        {
            WebClient wc = new WebClient();
            try
            {
                _patchInfoJason = wc.DownloadString(CurrentProfile.PatchInfoUrl);
                PatchFiles = JsonConvert.DeserializeObject<List<ManagedFile>>(_patchInfoJason);
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
        private void CreateDefaultIni()
        {
            const string defaultIni = @"[Comm]
ServerNumber=103
[Miscellaneous]
UserName=username
Download=10016
";
            using (StreamWriter sw = new StreamWriter(CurrentProfile.ClientFolder + "\\meridian.ini"))
            {
                sw.Write(defaultIni);
            }
        }
        private void CreateNewClient()
        {
            CreateFolderStructure();
            CreateDefaultIni();
        }
        public void ScanClient()
        {
            string fullpath;
            if (IsNewClient())
            {
                CreateNewClient();
            }

            foreach (ManagedFile patchFile in PatchFiles)
            {
                fullpath = CurrentProfile.ClientFolder + patchFile.Basepath + patchFile.Filename;
                FileScanned(this, new ScanEventArgs(patchFile.Filename)); //Tells the form to update the progress bar
                ManagedFile localFile = new ManagedFile(fullpath);
                localFile.ComputeHash();
                if (patchFile.MyHash != localFile.MyHash)
                {
                    LocalFiles.Add(localFile);
                    localFile.Length = patchFile.Length;
                    _patchTotalSize += patchFile.Length;
                }
            }
        }
        public void DownloadFiles()
        {
            foreach (ManagedFile file in LocalFiles)
            {
                string temp = file.Basepath.Replace("\\", "/");
                try
                {
                    StartedDownload(this, new StartDownloadEventArgs(file.Filename, file.Length));
                    MyWebClient.DownloadFile(CurrentProfile.PatchBaseUrl + temp + file.Filename, CurrentProfile.ClientFolder + file.Basepath + file.Filename);
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
            foreach (ManagedFile file in LocalFiles)
            {
                string temp = file.Basepath.Replace("\\", "/");
                StartedDownload(this, new StartDownloadEventArgs(file.Filename, file.Length));
                DownloadFileAsync(CurrentProfile.PatchBaseUrl + temp + file.Filename, CurrentProfile.ClientFolder + file.Basepath + file.Filename);
                while (!_continueAsync)
                {
                    //Wait for the previous file to finish
                    Thread.Sleep(10);
                }
            }
        }
        public void DownloadFileAsync(string url, string path)
        {
            using (WebClient client = new WebClient())
            {
                try
                {
                    client.DownloadProgressChanged += client_DownloadProgressChanged;
                    client.DownloadFileCompleted += client_DownloadFileCompleted;
                    client.DownloadFileAsync(new Uri(url), path);
                }
                catch (WebException e)
                {
                    Console.WriteLine(String.Format("Exception: {0}", e.ToString()));
                }
                _continueAsync = false;
            }
        }

        private void client_DownloadFileCompleted(object sender, AsyncCompletedEventArgs e)
        {
            OnEndDownload(e);
            _continueAsync = true;
        }
        private void client_DownloadProgressChanged(object sender, DownloadProgressChangedEventArgs e)
        {
            OnProgressedDownload(e);
        }

    }
}
