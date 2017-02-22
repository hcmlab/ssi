using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using Bespoke.Common.Osc;
using System.IO;

namespace FusionUI
{

    class UDPReceiver
    {
        int port_number;

        public int Port_number
        {
            get
            {
                return port_number;
            }
            set
            {
                port_number = value;
                if (running == true)
                {
                    stop();
                    start();
                }
            }
        }
        UdpClient Listener;
        IPEndPoint ipLocal;
        bool running = true;
        //public event OscElementHandler UDPReceived;
        public event FJOscElementHandler FJUDPReceived;
        private AsyncCallback asyncCallBackDelegate;
        private IAsyncResult asyncResults;
        public UDPReceiver()
        {
            port_number = 2222;
            Listener = new UdpClient(port_number);
            ipLocal = new IPEndPoint(IPAddress.Any, port_number);
            asyncCallBackDelegate = new AsyncCallback(OnDataReceived);
            //UdpState s = new UdpState();
            if (running == true)
            {
                WaitForData();
            }
        }

        public UDPReceiver(int port_number_)
        {
            port_number = port_number_;
            Listener = new UdpClient(port_number);
            ipLocal = new IPEndPoint(IPAddress.Any, port_number);
            asyncCallBackDelegate = new AsyncCallback(OnDataReceived);
            if (running == true)
            {
                WaitForData();
            }
        }

        public void stop()
        {
            running = false;
            //if (asyncResults != null)
            //{
            //    if (asyncResults.IsCompleted != true)
            //        ;//Listener.EndReceive(asyncResults, ref ipLocal);
            //}
            Listener.Close();
            //asyncResults = null;
        }

        public void start()
        {
            running = true;
            Listener = new UdpClient(port_number);
            ipLocal = new IPEndPoint(IPAddress.Any, port_number);
            WaitForData();
        }
       
        private void WaitForData()
        {
            try
            {
                asyncResults = Listener.BeginReceive(asyncCallBackDelegate, true);
            }
            catch (ObjectDisposedException e)
            {
                Console.WriteLine("Caught: {0}", e.Message);
            }
        }

        private void OnDataReceived(IAsyncResult asyn)
        {
            //asyncResults = asyn;
            if (running == true)
            {
                byte[] bytes;
                try
                {
                    bytes = Listener.EndReceive(asyn, ref ipLocal);
                }
                catch (ArgumentException e)
                {
                    //System.Windows.Forms.MessageBox.Show(e.Message, "Warning", System.Windows.Forms.MessageBoxButtons.OK);
                    return;
                }
                catch (ObjectDisposedException oe)
                {
                    //System.Windows.Forms.MessageBox.Show(oe.Message, "Warning", System.Windows.Forms.MessageBoxButtons.OK);
                    return;
                }


                byte[] length = BitConverter.GetBytes(bytes.Length);



                byte[] bytes2 = new byte[bytes.Length + length.Length];
                for (int i = 0; i < length.Length; i++)
                {
                    bytes2[i] = length[i];
                }
                for (int i = 0; i < bytes.Length; i++)
                {
                    bytes2[i + length.Length] = bytes[i];
                }
                Stream s = new MemoryStream(bytes2);
                OscPacket element = OscPacket.FromByteArray(ipLocal, bytes);                

                FJUDPReceived(element);

                WaitForData();
            }
            else
            {
                try
                {
                    Listener.EndReceive(asyn, ref ipLocal);
                }
                catch (ArgumentException e)
                {
                    //System.Windows.Forms.MessageBox.Show(e.Message, "Warning", System.Windows.Forms.MessageBoxButtons.OK);
                    return;
                }
                catch (ObjectDisposedException oe)
                {
                    //System.Windows.Forms.MessageBox.Show(oe.Message, "Warning", System.Windows.Forms.MessageBoxButtons.OK);
                    return;
                }
            }
        }
    }
}
