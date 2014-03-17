using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

using PatchServer;


namespace ClientPatcher
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new ClientPatchForm());
        }



    }
}
