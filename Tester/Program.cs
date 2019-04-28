/*
 * ExplorerSortOrder - Copyright © 2019 by Kevin Routley.
 */

using System;
using System.Windows.Forms;

namespace Tester
{
    static class Program
    {
        [STAThread]
        static void Main(string[] argv)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
