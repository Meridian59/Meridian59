using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Security.Cryptography;
using Newtonsoft.Json;

namespace PatchServer
{

    class ManagedFile
    {
        [JsonIgnore]
        public string path; //C:\whatever\
        [JsonIgnore]
        public string filepath;

        public string basepath = "\\";
        public string filename { get; set; } //whatever.roo
        public string myHash { get; set; } //what is the hash of this file

        public ManagedFile(string filepath)
        {
            this.filepath = filepath;
            if (filepath.Contains("resource"))
                basepath = "\\resource\\";
            Constructed();
        }

        private void Constructed()
        {
            ParseFilePath();
            myHash = ComputeHash();
            //Console.WriteLine(ToJson());
        }

        public void ParseFilePath()
        {
            path = Path.GetDirectoryName(filepath);
            filename = Path.GetFileName(filepath);
        }

        public string ComputeHash()
        {
            MD5 md5 = MD5.Create();
            FileStream stream = File.OpenRead(filepath);
            return ByteArrayToString(md5.ComputeHash(stream));
        }

        private string ByteArrayToString(byte[] bytearray)
        {
            string hex = BitConverter.ToString(bytearray);
            return hex.Replace("-", "");
        }

        public override string ToString()
        {
            return String.Format("File: {0,20} Hash: {1}", filename, myHash);
        }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this, Formatting.Indented);
        }
    }
}
