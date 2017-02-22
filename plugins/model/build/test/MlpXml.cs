using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using Microsoft.Win32.SafeHandles;
using System.Threading;

namespace ssi
{
    public delegate void ProcessStartDelegate();
    public delegate void ProcessStopDelegate();
    public delegate void ProcessOutputDelegate(string line);
    public delegate void ProcessUpdateDelegate(double start, double duration, string name);

    public class MlpXmlTrain
    {
        public static event ProcessStartDelegate processStartEvent;
        public static event ProcessStopDelegate processStopEvent;
        public static event ProcessOutputDelegate processOutputEvent;

        public static string xmlTrainFilepath = "";
        public static string pipelineFilepath = "";
        
        public static string traindef = null;
        public static string trainer = null;
        public static string training = null;
        public static string log = null;
        public static int eval = -1;
        public static int kfolds = 2;
        public static bool reextract = false;

        static SafeFileHandle pipeHandle_read = null;
        static Process process = null;

        private static void OutputStreamHandler(object sendingProcess,
            DataReceivedEventArgs outLine)
        {
            if (!String.IsNullOrEmpty(outLine.Data))
            {
                if (processOutputEvent != null)
                {
                    processOutputEvent(outLine.Data);
                }
                else
                {
                    Console.WriteLine(outLine.Data);
                }
            }
        }

        public static void OutputLog()
        {
            int tries = 5;
            bool retry = true;
            if (log != null && log != "")
            {
                while (retry)
                {
                    try
                    {
                        string str = File.ReadAllText(log);
                        if (processOutputEvent != null)
                        {
                            processOutputEvent(str);
                        }
                        else
                        {
                            Console.WriteLine(str);
                        }
                        retry = false;
                    }
                    catch (System.IO.IOException e)
                    {
                        Thread.Sleep(1000);
                        if (--tries == 0)
                        {
                            retry = false;
                            Error.Show(e.ToString());
                        }
                    }
                    catch (Exception e)
                    {
                        retry = false;
                        Error.Show(e.ToString());
                    }
                }
            }
        }

        public static void Start()
        {
            if (processStartEvent != null)
            {
                processStartEvent();
            }

            pipeHandle_read = ssi.NamedPipe.Create(ssi.NamedPipe.PIPE_NAME_READ, ssi.NamedPipe.PIPE_BUFFER_SIZE, ssi.NamedPipe.MODE.READ);

            process = new Process();
            process.StartInfo.FileName = xmlTrainFilepath;
            //process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            //process.OutputDataReceived += OutputStreamHandler;
            StringBuilder arguments = new StringBuilder();
            arguments.Append (@"--train -eval " + eval + " -kfolds " + kfolds + " -remote");
            if (log != null && log != "")
            {
                arguments.Append(" -log " + log);
            }
            if (reextract)
            {
                arguments.Append(" -reex ");
            }
            arguments.Append(" " + pipelineFilepath + " " + traindef + " " + training + " " + trainer);
            process.StartInfo.Arguments = arguments.ToString ();

            if (processOutputEvent != null)
            {
                processOutputEvent("-------------------------------------------" + Environment.NewLine + process.StartInfo.FileName + " " + process.StartInfo.Arguments + Environment.NewLine + "-------------------------------------------");
            }
            else
            {
                Console.WriteLine("-------------------------------------------" + Environment.NewLine + process.StartInfo.FileName + " " + process.StartInfo.Arguments + Environment.NewLine + "-------------------------------------------");
            }

            try
            {
                process.Start();
            }
            catch (Exception e)
            {
                Error.Show("Could not start process 'xmltrain'");
                ssi.NamedPipe.Close(pipeHandle_read);

                if (processStopEvent != null)
                {
                    processStopEvent();
                }

                return;
            }

            //process.BeginOutputReadLine();

            ssi.NamedPipe.WaitForClient(pipeHandle_read);

            FileStream fStream_read = new FileStream(pipeHandle_read, FileAccess.Read, ssi.NamedPipe.PIPE_BUFFER_SIZE, true);
            string msg = null;
            do
            {
                msg = ssi.NamedPipe.ReadString(fStream_read);
                if (msg != null)
                {
                    Console.WriteLine(msg);
                }
            } while (msg != null);

            ssi.NamedPipe.Close(pipeHandle_read);                        

            if (processStopEvent != null)
            {
                processStopEvent();
            }

            process.Close();
        }

        public static void Cancel()
        {
            try {
                process.Kill();
            }
            catch (Exception e) {
                Error.Show(e.ToString());
            }
        }
    }

    public class MlpXmlRun
    {
        public static event ProcessStartDelegate processStartEvent;
        public static event ProcessStopDelegate processStopEvent;
        public static event ProcessOutputDelegate processOutputEvent;
        public static event ProcessUpdateDelegate processUpdateEvent;

        static SafeFileHandle pipeHandle_read = null;
        static SafeFileHandle pipeHandle_write = null;
        static Process process = null;

        public static string xmlTrainFilepath = "";
        public static string pipelineFilepath = "";

        public static string trainer = null;
        public static string signal = null;
        public static string anno = null;
        public static string user = null;
        public static string log = null;

        private static void OutputStreamHandler(object sendingProcess,
            DataReceivedEventArgs outLine)
        {
            if (!String.IsNullOrEmpty(outLine.Data))
            {
                if (processOutputEvent != null)
                {
                    processOutputEvent(outLine.Data);
                }
                else
                {
                    Console.WriteLine(outLine.Data);
                }
            }
        }

        public static void OutputLog()
        {
            int tries = 5;
            bool retry = true;
            if (log != null && log != "")
            {
                while (retry)
                {
                    try
                    {
                        string str = File.ReadAllText(log);
                        if (processOutputEvent != null)
                        {
                            processOutputEvent(str);
                        }
                        else
                        {
                            Console.WriteLine(str);
                        }
                        retry = false;
                    }
                    catch (System.IO.IOException e)
                    {
                        Thread.Sleep(1000);
                        if (--tries == 0)
                        {
                            retry = false;
                            Error.Show(e.ToString());
                        }
                    }
                    catch (Exception e)
                    {
                        retry = false;
                        Error.Show(e.ToString());
                    }
                }
            }
        }

        public static void Stop()
        {
            FileStream fStream_write = new FileStream(pipeHandle_write, FileAccess.Write, ssi.NamedPipe.PIPE_BUFFER_SIZE, true);
            ssi.NamedPipe.WriteString(fStream_write, "stop");
        }

        public static void Start()
        {
            if (processStartEvent != null)
            {
                processStartEvent();
            }

            pipeHandle_read = ssi.NamedPipe.Create(ssi.NamedPipe.PIPE_NAME_READ, ssi.NamedPipe.PIPE_BUFFER_SIZE, ssi.NamedPipe.MODE.READ);
            pipeHandle_write = ssi.NamedPipe.Create(ssi.NamedPipe.PIPE_NAME_WRITE, ssi.NamedPipe.PIPE_BUFFER_SIZE, ssi.NamedPipe.MODE.WRITE);

            process = new Process();
            process.StartInfo.FileName = xmlTrainFilepath;
            //process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            //process.OutputDataReceived += OutputStreamHandler;
            StringBuilder arguments = new StringBuilder();
            arguments.Append(@"--run -remote");
            if (trainer != null && trainer != "")
            {
                arguments.Append(" -trainer " + trainer);
            }
            if (signal != null && signal != "")
            {
                arguments.Append(" -signal " + signal);
            }
            if (anno != null && anno != "")
            {
                arguments.Append(" -anno " + anno);
            }
            if (user != null && user != "")
            {
                arguments.Append(" -user " + user);
            }
            if (log != null && log != "")
            {
                arguments.Append(" -log " + log);
            }
            arguments.Append(" " + pipelineFilepath);
            process.StartInfo.Arguments = arguments.ToString();

            if (processOutputEvent != null)
            {
                processOutputEvent("-------------------------------------------" + Environment.NewLine + process.StartInfo.FileName + " " + process.StartInfo.Arguments + Environment.NewLine + "-------------------------------------------");
            }
            else
            {
                Console.WriteLine("-------------------------------------------" + Environment.NewLine + process.StartInfo.FileName + " " + process.StartInfo.Arguments + Environment.NewLine + "-------------------------------------------");
            }

            try
            {
                process.Start();
            }
            catch (Exception e)
            {
                Error.Show("Could not start process 'xmltrain'");
                ssi.NamedPipe.Close(pipeHandle_read);
                ssi.NamedPipe.Close(pipeHandle_write);

                if (processStopEvent != null)
                {
                    processStopEvent();
                }

                return;
            }

            //process.BeginOutputReadLine();

            ssi.NamedPipe.WaitForClient(pipeHandle_write);
            ssi.NamedPipe.WaitForClient(pipeHandle_read);

            FileStream fStream_read = new FileStream(pipeHandle_read, FileAccess.Read, ssi.NamedPipe.PIPE_BUFFER_SIZE, true);
            string msg = null;
            do
            {
                msg = ssi.NamedPipe.ReadString(fStream_read);
                if (msg != null)
                {
                    if (processUpdateEvent != null)
                    {
                        string[] words = msg.Split(',');
                        if (words.Length != 3)
                        {
                            Error.Show("Invalid update string");
                        }
                        else
                        {
                            double start = Double.Parse(words[0], System.Globalization.CultureInfo.InvariantCulture);
                            double duration = Double.Parse(words[1], System.Globalization.CultureInfo.InvariantCulture);
                            string label = words[2].Substring(0, words[2].Length - 1);
                            processUpdateEvent(start, duration, label);
                        }
                    }
                    else
                    {
                        Console.WriteLine(msg);
                    }
                }
            } while (msg != null);

            ssi.NamedPipe.Close(pipeHandle_read);
            ssi.NamedPipe.Close(pipeHandle_write);

            if (processStopEvent != null)
            {
                processStopEvent();
            }

            process.Close();
        }

    }

}
