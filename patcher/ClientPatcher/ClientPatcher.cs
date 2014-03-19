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
 

    class ClientPatcher
    {
        private string PatchInfoJason;

        private List<ManagedFile> PatchFiles; //Loaded from the web server at PatchInfoURL
        private List<ManagedFile> LocalFiles; //Loaded with files that do NOT match

        private PatcherSettings CurrentProfile;

        private long PatchTotalSize = 0;

        private int DownloadJson()
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

        private void ScanClient()
        {
            string fullpath;
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\resource\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\download\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\help\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\mail\\");
            Directory.CreateDirectory(CurrentProfile.ClientFolder + "\\ads\\");

            foreach (ManagedFile PatchFile in PatchFiles)
            {
                fullpath = CurrentProfile.ClientFolder + PatchFile.basepath + PatchFile.filename;
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

        private void DownloadFiles()
        {
            WebClient client = new WebClient();

            foreach (ManagedFile File in LocalFiles)
            {
                string temp = File.basepath.Replace("\\", "/");
                try
                {
                    client.DownloadFile(CurrentProfile.PatchBaseURL + temp + File.filename, CurrentProfile.ClientFolder + File.basepath + File.filename);
                }
                catch (WebException e)
                {
                    Console.WriteLine("WebException Handler: {0}", e.ToString());
                    return;
                }
            }
        }



    }
}
