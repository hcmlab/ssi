using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using Microsoft.Win32.SafeHandles;

namespace ssi
{
    public delegate void ProcessStartDelegate();
    public delegate void ProcessStopDelegate();
    public delegate void ProcessOutputDelegate(string line);
    public delegate void ProcessUpdateDelegate(double start, double duration, string name);

    class MlpXmlTrain
    {
        public static event ProcessStartDelegate processStartEvent;
        public static event ProcessStopDelegate processStopEvent;
        public static event ProcessOutputDelegate processOutputEvent;

        public static string xmlTrainFilepath = "";
        public static string pipelineFilepath = "";

        public static string trainerFilepath = "";
        public static string trainingFilepath = "";

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

        public static void Start()
        {
            if (processStartEvent != null)
            {
                processStartEvent();
            }

            pipeHandle_read = ssi.NamedPipe.Create(ssi.NamedPipe.PIPE_NAME_READ, ssi.NamedPipe.PIPE_BUFFER_SIZE, ssi.NamedPipe.MODE.READ);

            process = new Process();
            process.StartInfo.FileName = xmlTrainFilepath;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            process.OutputDataReceived += OutputStreamHandler;
            process.StartInfo.Arguments = @"--train -remote " + pipelineFilepath + " " + trainingFilepath + " " + trainerFilepath;

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

            process.BeginOutputReadLine();

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
            process.Kill();
        }
    }

    class MlpXmlRun
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

        public static string trainerFilepath = "";
        public static string signalFilepath = "";
        public static string annoFilepath = "";     

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
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.CreateNoWindow = true;
            process.OutputDataReceived += OutputStreamHandler;
            StringBuilder arguments = new StringBuilder();
            arguments.Append(@"--run -remote");
            if (trainerFilepath != "")
            {
                arguments.Append(" -trainer " + trainerFilepath);
            }
            if (signalFilepath != "")
            {
                arguments.Append(" -signal " + signalFilepath);
            }
            if (annoFilepath != "")
            {
                arguments.Append(" -anno " + annoFilepath);
            }
            arguments.Append(" " + pipelineFilepath);
            process.StartInfo.Arguments = arguments.ToString();

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

            process.BeginOutputReadLine();

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
                            processUpdateEvent(Double.Parse(words[0]), Double.Parse(words[1]), words[2]);
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
