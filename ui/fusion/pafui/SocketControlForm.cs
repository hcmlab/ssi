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
    public partial class SocketControlForm : Form
    {
        public SocketControlForm(System.Net.IPEndPoint currentSocket)
        {
            InitializeComponent();
            alfredRemoteSocket = currentSocket;
        }

        private System.Net.IPEndPoint tmpDefaultSocket;
        private System.Net.IPEndPoint alfredRemoteSocket;

        public System.Net.IPEndPoint AlfredRemoteSocket
        {
            get { return alfredRemoteSocket; }
            set { alfredRemoteSocket = value; }
        }

        private void AlfredControlForm_Load(object sender, EventArgs e)
        {
            textBox1.Text = alfredRemoteSocket.Address.ToString();
            numericUpDown1.Value = (decimal)alfredRemoteSocket.Port;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            tmpDefaultSocket = new System.Net.IPEndPoint(System.Net.IPAddress.Loopback, 5554);
            textBox1.Text = tmpDefaultSocket.Address.ToString();
            numericUpDown1.Value = (decimal)tmpDefaultSocket.Port;
            alfredRemoteSocket = tmpDefaultSocket;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            System.Net.IPAddress tmpParseAddress;
            try
            {
                tmpParseAddress = System.Net.IPAddress.Parse(textBox1.Text);
            }
            catch (FormatException fe)
            {
                //System.Windows.Forms.MessageBox.Show("Not a valid IP-Address: " + textBox1.Text, "Error!", MessageBoxButtons.OK);
                //textBox1.Text = alfredRemoteSocket.Address.ToString();
                return;
            }
            catch (ArgumentNullException ne)
            {
                textBox1.Text = alfredRemoteSocket.Address.ToString();
                return;
            }

            alfredRemoteSocket.Address = tmpParseAddress;
        }

        private void textBox1_Leave(object sender, EventArgs e)
        {
            System.Net.IPAddress tmpParseAddress;
            try
            {
                tmpParseAddress = System.Net.IPAddress.Parse(textBox1.Text);
            }
            catch (FormatException fe)
            {
                //System.Windows.Forms.MessageBox.Show("Not a valid IP-Address: " + textBox1.Text, "Error!", MessageBoxButtons.OK);
                //textBox1.Text = alfredRemoteSocket.Address.ToString();
                return;
            }
            catch (ArgumentNullException ne)
            {
                textBox1.Text = alfredRemoteSocket.Address.ToString();
                return;
            }

            alfredRemoteSocket.Address = tmpParseAddress;
        }
    }
}
