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
    public partial class HelpForm : Form
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public HelpForm()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Closes the form
        /// </summary>
        /// <param name="sender">object that initiated this event</param>
        /// <param name="e">event details</param>
        private void buttonOK_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}
