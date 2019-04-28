using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace Tester
{
    public partial class Form1 : Form
    {
        [DllImport("ExplorerSortOrder.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int take2(string path, ref StringBuilder str, int len, ref Int32 ascend);

        public Form1()
        {
            InitializeComponent();
        }

        private void getFolderSort(string path)
        {
            lblPath.Text = path;
            int ascend = -1;
            StringBuilder sb = new StringBuilder(200);
            int res = take2(path, ref sb, sb.Capacity, ref ascend);
            if (res == 0 && ascend >= 0)
            {
                lblResults.Text = sb.ToString() + (ascend > 0 ? "(ascend)" : "(descend)");
            }
            else
            {
                lblResults.Text = "fail";
            }
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            var path = textBox1.Text;
            getFolderSort(path);
        }

        private void Panel1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.Copy;
        }

        private void Panel1_DragDrop(object sender, DragEventArgs e)
        {
            string filePath = ((string[])e.Data.GetData(DataFormats.FileDrop))[0];
            string folder = System.IO.Path.GetDirectoryName(filePath);
            getFolderSort(folder);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            var args = Environment.GetCommandLineArgs();
            if (args.Length >= 2)
            {
                string folder = System.IO.Path.GetDirectoryName(args[1]);
                getFolderSort(folder);
            }
        }
    }
}
