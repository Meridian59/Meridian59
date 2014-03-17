using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace PatchServer
{
    class PatchMain
    {
        static string JsonOutputPath = "C:\\wamp\\www\\patchinfo.txt";
        
        static void Main(string[] args)
        {
            ClientScanner clientscanner;
            
            //Creates list of latest file hashes
            clientscanner = new ClientScanner();
            clientscanner.ScanSource();
            //Console.WriteLine(clientscanner.ToJson());

            using (StreamWriter sw = new StreamWriter(JsonOutputPath))
            {
                sw.Write(clientscanner.ToJson());
            }
            
            Console.ReadKey();
        }
    }
}
