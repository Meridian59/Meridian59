using System;
using System.IO;
using System.Security.Cryptography;
using Newtonsoft.Json;

namespace PatchListGenerator
{

    public class ManagedFile
    {
        [JsonIgnore]
        public string Path; //C:\whatever\
        [JsonIgnore]
        public string Filepath;
        [JsonIgnore]
        public bool Download = false;

        public string Basepath = "\\";
        public string Filename { get; set; } //whatever.roo
        public string MyHash { get; set; } //what is the hash of this file
        public long Length = 0;

        public ManagedFile()
        {
        }

        public ManagedFile(string filepath)
        {
            Filepath = filepath;
            ParseFilePath();
        }

        public void ParseFilePath()
        {
            Path = System.IO.Path.GetDirectoryName(Filepath);
            Filename = System.IO.Path.GetFileName(Filepath);
            if (Filepath.Contains("resource"))
                Basepath = "\\resource\\";
        }

        public void ComputeHash()
        {
            MD5 md5 = MD5.Create();
            if (!File.Exists(Filepath))
            {
                MyHash = "";
                return;
            }
            FileStream stream = File.OpenRead(Filepath);
            MyHash = ByteArrayToString(md5.ComputeHash(stream));
        }

        private string ByteArrayToString(byte[] bytearray)
        {
            string hex = BitConverter.ToString(bytearray);
            return hex.Replace("-", "");
        }

        public override string ToString()
        {
            return String.Format("File: {0,20} Hash: {1} Size: {2}", Filename, MyHash, Length);
        }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this, Formatting.Indented);
        }

        public void FillLength()
        {
            FileInfo file = new FileInfo(Filepath);
            Length = file.Length;
        }
    }
}
