using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FusionUI
{
    public partial class PortSelectionForm : Form
    {
        public PortSelectionForm()
        {
            InitializeComponent();
            Port = defaultPort;
        }

        int defaultPort = 2222;
        int port;

        public int Port
        {
            get { return port; }
            set { port = value; }
        }

        public PortSelectionForm(int _port)
        {
            InitializeComponent();
            Port = _port;
        }

        private void PortSelectionForm_Load(object sender, EventArgs e)
        {
            numericUpDown1.Value = (decimal)Port;
        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            Port = (int)(numericUpDown1.Value + 0.5m);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            Port = 2222;
            numericUpDown1.Value = (decimal)Port;
        }
    }
}
