using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using Newtonsoft.Json;


namespace PatchListGenerator
{
    class ClientScanner
    {
        private List<string> ScanFolder { get; set; }
        private List<string> ScanExtensions { get; set; }
        public List<ManagedFile> Files { get; set; }
        
        public ClientScanner()
        {
            ScanFolder = new List<string>();
            ScanExtensions = new List<string>();
            Files = new List<ManagedFile>();

            ScanFolder.Add("C:\\Meridian59-master\\run\\localclient\\");
            ScanFolder.Add("C:\\Meridian59-master\\run\\localclient\\resource");
            ScanExtensions.Add(".roo");
            ScanExtensions.Add(".dll");
            ScanExtensions.Add(".rsb");
            ScanExtensions.Add(".exe");
            ScanExtensions.Add(".bgf");
            ScanExtensions.Add(".wav");
            ScanExtensions.Add(".mp3");
        }

        public void ScanSource()
        {
            foreach (string folder in ScanFolder) //
            {
                if (!System.IO.Directory.Exists(folder))
                {
                    //Folder doesn't exist =(
                    throw new Exception();
                }
                // Process the list of files found in the directory. 
                string[] fileEntries = Directory.GetFiles(folder);
                foreach (string fileName in fileEntries)
                {
                    string ext = fileName.Substring(fileName.Length - 4);
                    if (ScanExtensions.Contains(ext))
                    {
                        // do something with fileName
                        ManagedFile file;
                        file = new ManagedFile(fileName);
                        file.ParseFilePath();
                        file.ComputeHash();
                        file.FillLength();
                        Files.Add(file);
                    }
                }
            }
        }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(Files, Formatting.Indented);
        }

        public void Report()
        {
            foreach (ManagedFile file in Files)
                Console.WriteLine(file.ToString());
        }

    }
}
