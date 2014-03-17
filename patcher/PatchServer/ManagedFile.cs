using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Security.Cryptography;
using Newtonsoft.Json;

namespace PatchListGenerator
{

    public class ManagedFile
    {
        [JsonIgnore]
        public string path; //C:\whatever\
        [JsonIgnore]
        public string filepath;
        [JsonIgnore]
        public bool download = false;

        public string basepath = "\\";
        public string filename { get; set; } //whatever.roo
        public string myHash { get; set; } //what is the hash of this file
        public long Length = 0;

        public ManagedFile()
        {
        }

        public ManagedFile(string filepath)
        {
            this.filepath = filepath;
            ParseFilePath();
        }

        public void ParseFilePath()
        {
            path = Path.GetDirectoryName(filepath);
            filename = Path.GetFileName(filepath);
            if (filepath.Contains("resource"))
                basepath = "\\resource\\";
        }

        public void ComputeHash()
        {
            MD5 md5 = MD5.Create();
            if (!File.Exists(filepath))
            {
                myHash = "";
                return;
            }
            FileStream stream = File.OpenRead(filepath);
            myHash = ByteArrayToString(md5.ComputeHash(stream));
        }

        private string ByteArrayToString(byte[] bytearray)
        {
            string hex = BitConverter.ToString(bytearray);
            return hex.Replace("-", "");
        }

        public override string ToString()
        {
            return String.Format("File: {0,20} Hash: {1} Size: {2}", filename, myHash, Length);
        }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this, Formatting.Indented);
        }

        public void FillLength()
        {
            FileInfo file = new FileInfo(filepath);
            Length = file.Length;
        }
    }
}
