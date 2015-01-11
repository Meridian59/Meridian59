using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace m59bind
{
    public partial class KeyModForm : Form
    {
        object targetButton;

        public KeyModForm(object target)
        {
            InitializeComponent();
            targetButton = target;
        }

        private string stripModifiers(string input)
        {
            string rString = "";

            rString = input.Replace("+alt", "");
            rString = rString.Replace("+ctrl", "");
            rString = rString.Replace("+shift", "");
            rString = rString.Replace("+any", "");

            return rString;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void buttonAlt_Click(object sender, EventArgs e)
        {
            Button b = (Button) targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+alt";
            this.Close();
        }

        private void buttonCtrl_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+ctrl";
            this.Close();
        }

        private void buttonShift_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+shift";
            this.Close();
        }

        private void buttonAny_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+any";
            this.Close();
        }

        private void buttonNone_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            this.Close();
        }
    }
}
