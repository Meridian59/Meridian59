using System;
using System.Collections.Generic;
using System.Linq;
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
        private readonly string _filename;
        public string Filename
        {
            get
            {
                return _filename;
            }
        }

        public ScanEventArgs(string filename)
        {
            _filename = filename;
        }
    }
    //Event when we Start a Download, used to notify UI.
    public delegate void StartDownloadEventHandler(object sender, StartDownloadEventArgs e);
    public class StartDownloadEventArgs : EventArgs
    {
        private readonly long _filesize;
        public long Filesize
        {
            get
            {
                return _filesize;
            }
        }

        private readonly string _filename;
        public string Filename
        {
            get
            {
                return _filename;
            }
        }

        public StartDownloadEventArgs(string filename, long filesize)
        {
            _filename = filename;
            _filesize = filesize;
        }
    }
    //Event when we Make progress in a Download, used to notify UI.
    public delegate void ProgressDownloadEventHandler(object sender, DownloadProgressChangedEventArgs e);
    //Event when we Complete a Download, used to notify UI.
    public delegate void EndDownloadEventHandler(object sender, AsyncCompletedEventArgs e);

    #endregion

    class ClientPatcher
    {
        const string CacheFile = "\\cache.txt";

        private string _patchInfoJason = "";

        public List<ManagedFile> PatchFiles; //Loaded from the web server at PatchInfoURL
        public List<ManagedFile> downloadFiles; //Loaded with files that do NOT match
        private List<ManagedFile> cacheFiles; //Loaded with cache.txt to compare to PatchFiles

        public WebClient MyWebClient;

        bool _continueAsync;

        public PatcherSettings CurrentProfile { get; set; }

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
            downloadFiles = new List<ManagedFile>();
            MyWebClient = new WebClient();
        }
        public ClientPatcher(PatcherSettings settings)
        {
            downloadFiles = new List<ManagedFile>();
            MyWebClient = new WebClient();
            CurrentProfile = settings;
        }
        public int DownloadPatchDefinition()
        {
            var wc = new WebClient();
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

        public bool HasCache()
        {
            return File.Exists(CurrentProfile.ClientFolder + CacheFile);
        }

        private void CreateFolderStructure()
        {
            try
            {
                Directory.CreateDirectory(CurrentProfile.ClientFolder);
                Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\resource\\");
                Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\download\\");
                Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\help\\");
                Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\mail\\");
                Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\ads\\");
            }
            catch (Exception e)
            {
                
                throw new IOException("Unable to CreateFolderStructure()" + e);
            }
            
        }

        private void CreateDefaultIni()
        {
            try
            {
                const string defaultIni = @"[Comm]
ServerNumber=103
[Miscellaneous]
UserName=username
Download=10016
";
                using (var sw = new StreamWriter(CurrentProfile.ClientFolder + "\\meridian.ini"))
                {
                    sw.Write(defaultIni);
                }
            }
            catch (Exception e)
            {

                throw new IOException("Unable to CreateDefaultIni()" + e);
            }
            
        }

        private void CreateNewClient()
        {
            CreateFolderStructure();
            CreateDefaultIni();
            //TODO: Replace this with code to download an initial .zip of the client.
        }

        public void ScanClient()
        {
            if (IsNewClient())
            {
                CreateNewClient();
            }
            CompareFiles();
        }

        public void GenerateCache()
        {
            string fullpath = CurrentProfile.ClientFolder;
            var scanner = new ClientScanner(fullpath);
            scanner.ScanSource();
            using (var sw = new StreamWriter(fullpath + CacheFile))
            {
                sw.Write(scanner.ToJson());
            }    
        }

        public void SavePatchAsCache()
        {
            string fullpath = CurrentProfile.ClientFolder;
            using (var sw = new StreamWriter(fullpath + CacheFile))
            {
                sw.Write(JsonConvert.SerializeObject(PatchFiles));
            }
        }

        public void LoadCache()
        {
            if (HasCache())
            {
                StreamReader file = File.OpenText(CurrentProfile.ClientFolder + CacheFile); //Open the file

                cacheFiles = JsonConvert.DeserializeObject<List<ManagedFile>>(file.ReadToEnd()); //convert
                file.Close(); //close
            }

        }

        public void CompareCache()
        {
            foreach (ManagedFile patchFile in PatchFiles)
            {
                FileScanned(this, new ScanEventArgs(patchFile.Filename)); //Tells the form to update the progress bar
                ManagedFile currentFile =
                    cacheFiles.FirstOrDefault(x => x.Basepath + x.Filename == patchFile.Basepath + patchFile.Filename);
                if (currentFile == null) //file not in cache, download it.
                    downloadFiles.Add(patchFile);
                else
                    if (patchFile.MyHash != currentFile.MyHash)
                    {
                        currentFile.Length = patchFile.Length;
                        downloadFiles.Add(currentFile);
                    }

            }
        }

        public void CompareFiles()
        {
            foreach (ManagedFile patchFile in PatchFiles)
            {
                string fullpath = CurrentProfile.ClientFolder + patchFile.Basepath + patchFile.Filename;
                FileScanned(this, new ScanEventArgs(patchFile.Filename)); //Tells the form to update the progress bar
                var localFile = new ManagedFile(fullpath);
                localFile.ComputeHash();
                if (patchFile.MyHash != localFile.MyHash)
                {
                    downloadFiles.Add(localFile);
                    localFile.Length = patchFile.Length;
                }
            }
        }

        public void DownloadFiles()
        {
            foreach (ManagedFile file in downloadFiles)
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
            foreach (ManagedFile file in downloadFiles)
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
            using (var client = new WebClient())
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
