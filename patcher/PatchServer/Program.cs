using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace PatchListGenerator
{
    class PatchListGenerator
    {
        static string JsonOutputPath = null;
        static string ClientPath = null;
        
        static void Main(string[] args)
        {
            /*
             * -c|--client [path] - Base folder to scan for patch info
             * -o|--outfile [path] - File to store patch info in
             */

            if (args.Length < 2)
            {
                Console.WriteLine("Not enough parameters");
                Console.WriteLine("--client=[path] - Base folder to scan for patch info");
                Console.WriteLine("--outfile=[path] - File to store patch info in");
                return;
            }

            char[] delim = new char[] { '=' };

            foreach (string argument in args)
            {
                string[] param = argument.Split(delim);
                switch (param[0])
                {
                    case "--client":
                        ClientPath = param[1];
                        break;
                    case "--outfile":
                        JsonOutputPath = param[1];
                        break;
                    default:
                        Console.WriteLine(String.Format("Parameter: '{0}' not known."));
                        return;
                }
            }

            if ((ClientPath == null) | (JsonOutputPath == null))
            {
                Console.WriteLine("Missing Parameter");
                Console.WriteLine("--client=[path] - Base folder to scan for patch info");
                Console.WriteLine("--outfile=[path] - File to store patch info in");
                return;
            }

            Console.WriteLine(String.Format("Scan Folder: {0}",ClientPath));
            Console.WriteLine(String.Format("Output File: {0}",JsonOutputPath));

            ClientScanner clientscanner;

            Console.WriteLine("Scanning...");
            //Creates list of latest file hashes
            clientscanner = new ClientScanner(ClientPath);
            clientscanner.ScanSource();
            Console.WriteLine(String.Format("Scanned {0} Files", clientscanner.Files.Count));

            using (StreamWriter sw = new StreamWriter(JsonOutputPath))
            {
                sw.Write(clientscanner.ToJson());
            }

            Console.WriteLine("File Written! Goodbye!");
        }
    }
}
