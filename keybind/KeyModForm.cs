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
        object targetButton;    // the control being modified

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="target">the target control</param>
        public KeyModForm(object target)
        {
            InitializeComponent();
            targetButton = target;
        }

        /// <summary>
        /// removes key modifiers from a comtrol's text
        /// </summary>
        /// <param name="input">control text</param>
        /// <returns>modified control text</returns>
        private string stripModifiers(string input)
        {
            string rString = "";

            rString = input.Replace("+alt", "");
            rString = rString.Replace("+ctrl", "");
            rString = rString.Replace("+shift", "");
            rString = rString.Replace("+any", "");

            return rString;
        }

        /// <summary>
        /// closes the form
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// adds alt to a key string
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonAlt_Click(object sender, EventArgs e)
        {
            Button b = (Button) targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+alt";
            this.Close();
        }

        /// <summary>
        /// adds ctrl to a key string
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonCtrl_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+ctrl";
            this.Close();
        }

        /// <summary>
        /// adds shift to a key string
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonShift_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+shift";
            this.Close();
        }

        /// <summary>
        /// adds any to a key string
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonAny_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            b.Text += "+any";
            this.Close();
        }

        /// <summary>
        /// removes modifiers from a key string
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonNone_Click(object sender, EventArgs e)
        {
            Button b = (Button)targetButton;
            b.Text = stripModifiers(b.Text);
            this.Close();
        }
    }
}
