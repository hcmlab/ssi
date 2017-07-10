using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Sockets;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Imaging;
using System.Windows.Threading;

namespace ssi
{


    public class Pipeline : IMedia
    {
        /*
         *  Call XMLpipe.exe when annotation contains pipeline flag: example pipeline="bn\laughter\bn.pipeline" (relative to xmlpipe folder)
         *  
         *
         * */

        UdpClient client;


        ////alternative way, set SSI pipeline in foreground, send enter
        //[DllImport("user32.dll")]
        //private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        //[DllImport("user32.dll")]
        //private static extern bool SetForegroundWindow(IntPtr hWnd);

        AnnoList annoList;
        bool pipelineExecuted = false;
        string pipelinepath;
        Process process = new Process();

        public Pipeline(AnnoList annoList, string pipelinepath)
        {
            this.annoList = annoList;
            this.pipelinepath = pipelinepath;
            client = new UdpClient("127.0.0.1", 1111);


            CallXMLPipe(pipelinepath);


        }

        ~Pipeline()
        {
            try
            {
                client.Close();

            }
            catch { }


        }

        private int findItem(double position)
        {
            if (annoList.Count == 0)
            {
                return -1;
            }

            for (int index = 0; index < annoList.Count; index++)
            {
                if (position <= annoList[index].Stop)
                {
                    return index;
                }
            }

            return -1;
        }

        public void move(double newPosition, double threshold)
        {
        }


        public string CallXMLPipe(string pipelinepath)
        {
            string result = "";

            try
            {

                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.WindowStyle = ProcessWindowStyle.Normal;
                string folder = AppDomain.CurrentDomain.BaseDirectory + "xmlpipe\\";
                startInfo.FileName = folder + "xmlpipe.exe";
                startInfo.Arguments = folder + pipelinepath;
                process.StartInfo = startInfo;
                process.Start();
              
               // process.WaitForExit();
            }
            catch (Exception ex)
            {
                MessageTools.Error(ex.ToString());
            }

            return result;
        }


        public void Clear()
        {
        }

        public string GetDirectory()
        {
            if (annoList.Source.HasFile)
            {
                return annoList.Source.File.Directory;
            }
            return "";
        }


        public string GetFilepath()
        {
            if (annoList.Source.HasFile)
            {
                return annoList.Source.File.Path;
            }
            return "";
        }

        public double GetLength()
        {
            return annoList[annoList.Count - 1].Stop;
        }

        public MediaType GetMediaType()
        {
            return MediaType.PIPELINE;
        }

        public WriteableBitmap GetOverlay()
        {
            return null;
        }

        public double GetPosition()
        {
            return 0;
        }

        public double GetSampleRate()
        {
            return 0;
        }

        public UIElement GetView()
        {
            return null;
        }

        public double GetVolume()
        {
            return 0;
        }

        public void SetVolume(double volume)
        {
        }

        public bool HasAudio()
        {
            return false;
        }

        public void Move(double time)
        {
            if (annoList != null && annoList.Count > 0)
            {
                move(time, 0.2);
            }
        }

        public void Pause()
        {
            ////alternative way, set SSI pipeline in foreground, send enter
            //IntPtr zero = IntPtr.Zero;
            //string folder = AppDomain.CurrentDomain.BaseDirectory + "xmlpipe\\xmlpipe.exe";

            //zero = FindWindow(null, folder);
            //if (zero != IntPtr.Zero)
            //{
            //    SetForegroundWindow(zero);
            //    System.Windows.Forms.SendKeys.SendWait("{ENTER}");
            //    System.Windows.Forms.SendKeys.Flush();
            //}

            string message = "SSI:STOP:RUN1\0";
            byte[] bytes = Encoding.ASCII.GetBytes(message);

            client.Send(bytes, bytes.Length);

        }

        public void Play()
        {
            //here we make sure the pipeline is run only once. (to make sure data does not get overwritten on play/pause. This could be changed in the future. To run it again, reload the annotation
            if (pipelineExecuted == false)
            {
                pipelineExecuted = true;

                string message = "SSI:STRT:RUN1\0";
                byte[] bytes = Encoding.ASCII.GetBytes(message);

                client.Send(bytes, bytes.Length);

            }

        }

        public void Stop()
        {
           
        }
    }
}
