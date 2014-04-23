using System;
using System.IO;

namespace PatchListGenerator
{
    class PatchListGenerator
    {
        static string _jsonOutputPath;
        static string _clientPath;
        
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

            char[] delim = { '=' };

            foreach (string argument in args)
            {
                string[] param = argument.Split(delim);
                switch (param[0])
                {
                    case "--client":
                        _clientPath = param[1];
                        break;
                    case "--outfile":
                        _jsonOutputPath = param[1];
                        break;
                    default:
                        Console.WriteLine(String.Format("Parameter: '{0}' not known."));
                        return;
                }
            }

            if ((_clientPath == null) | (_jsonOutputPath == null))
            {
                Console.WriteLine("Missing Parameter");
                Console.WriteLine("--client=[path] - Base folder to scan for patch info");
                Console.WriteLine("--outfile=[path] - File to store patch info in");
                return;
            }

            Console.WriteLine(String.Format("Scan Folder: {0}",_clientPath));
            Console.WriteLine(String.Format("Output File: {0}",_jsonOutputPath));

            ClientScanner clientscanner;

            Console.WriteLine("Scanning...");
            //Creates list of latest file hashes
            clientscanner = new ClientScanner(_clientPath);
            clientscanner.ScanSource();
            Console.WriteLine(String.Format("Scanned {0} Files", clientscanner.Files.Count));

            using (StreamWriter sw = new StreamWriter(_jsonOutputPath))
            {
                sw.Write(clientscanner.ToJson());
            }

            Console.WriteLine("File Written! Goodbye!");
        }
    }
}
