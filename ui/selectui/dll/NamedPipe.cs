using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using System.IO;

namespace ssi
{

    public class NamedPipe
    {
        public const string PIPE_NAME_WRITE = "\\\\.\\pipe\\MlpXmlRead";
        public const string PIPE_NAME_READ = "\\\\.\\pipe\\MlpXmlWrite";
        public const int PIPE_BUFFER_SIZE = 4096;
        public const uint PIPE_ACCESS_INBOUND = (0x00000001);
        public const uint PIPE_ACCESS_OUTBOUND = (0x00000002);
        public const uint PIPE_ACCESS_DUPLEX = (0x00000003);
        public const uint FILE_FLAG_OVERLAPPED = (0x40000000);
        public const uint GENERIC_READ = (0x80000000);
        public const uint GENERIC_WRITE = (0x40000000);
        public const uint OPEN_EXISTING = 3;

        public enum MODE
        {
            READ = 0,
            WRITE
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern SafeFileHandle CreateFile(
           String pipeName,
           uint dwDesiredAccess,
           uint dwShareMode,
           IntPtr lpSecurityAttributes,
           uint dwCreationDisposition,
           uint dwFlagsAndAttributes,
           IntPtr hTemplate);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern SafeFileHandle CreateNamedPipe(
           String pipeName,
           uint dwOpenMode,
           uint dwPipeMode,
           uint nMaxInstances,
           uint nOutBufferSize,
           uint nInBufferSize,
           uint nDefaultTimeOut,
           IntPtr lpSecurityAttributes);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern int ConnectNamedPipe(
           SafeFileHandle hNamedPipe,
           IntPtr lpOverlapped);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern int CloseHandle(
           SafeFileHandle hNamedPipe);

        public static void WriteString(FileStream fStream, string message)
        {
#if DEBUG
            Console.Write("NamedPipe: send message '" + message + "'.. ");
#endif

            ASCIIEncoding encoder = new ASCIIEncoding();
            byte[] sendBuffer = new byte[message.Length + 1];
            for (int i = 0; i < message.Length; i++)
            {
                sendBuffer[i] = (byte)message[i];
            }
            sendBuffer[message.Length] = 0;

            fStream.Write(sendBuffer, 0, sendBuffer.Length);
            fStream.Flush();

#if DEBUG
            Console.WriteLine("OK");
#endif
        }

        public static string ReadString(FileStream fStream)
        {
#if DEBUG
            Console.Write("NamedPipe: read message .. ");
#endif

            byte[] buffer = new byte[PIPE_BUFFER_SIZE];

            int bytesRead = fStream.Read(buffer, 0, PIPE_BUFFER_SIZE);
            if (bytesRead == 0)
            {
#if DEBUG
                Console.WriteLine("lost client");
#endif
                return null;
            }

            string msg = Encoding.UTF8.GetString(Encoding.Convert(Encoding.GetEncoding("iso-8859-1"), Encoding.UTF8, buffer), 0, bytesRead);

#if DEBUG
            Console.WriteLine("'" + msg + "' OK");
#endif

            return msg;
        }

        public static SafeFileHandle Connect(string name, MODE mode)
        {
#if DEBUG
            Console.Write("NamedPipe: connect.. ");
#endif
            SafeFileHandle pipeHandle = null;

            switch (mode)
            {
                case MODE.READ:
                    pipeHandle =
                       CreateFile(
                          name,
                          GENERIC_READ,
                          0,
                          IntPtr.Zero,
                          OPEN_EXISTING,
                          FILE_FLAG_OVERLAPPED,
                          IntPtr.Zero);
                    break;
                case MODE.WRITE:
                    pipeHandle =
                       CreateFile(
                          name,
                          GENERIC_WRITE,
                          0,
                          IntPtr.Zero,
                          OPEN_EXISTING,
                          FILE_FLAG_OVERLAPPED,
                          IntPtr.Zero);
                    break;
            }

            //could not get a handle to the named pipe
            if (pipeHandle.IsInvalid)
            {
                Error.Show("NamedPipe: Invalid pipeline handle");
                return null;
            }

            return pipeHandle;
        }

        public static bool WaitForClient(SafeFileHandle pipeHandle)
        {
#if DEBUG
            Console.Write("NamedPipe: wait for client.. ");
#endif

            // wait or client
            int success = ConnectNamedPipe(
               pipeHandle,
               IntPtr.Zero);

            //failed to connect client pipe
            /*if (success == 0)
            {
                MessageBox.Show("NamedPipe: Could not connect client", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }*/

#if DEBUG
            Console.WriteLine("OK");
#endif
            return true;
        }

        public static SafeFileHandle Create(string name, uint size, MODE mode)
        {
#if DEBUG
            Console.Write("NamedPipe: create pipe.. ");
#endif

            SafeFileHandle pipeHandle = null;

            switch (mode)
            {
                case MODE.READ:
                    pipeHandle = CreateNamedPipe(
                        name,
                        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                        0,
                        255,
                        size,
                        size,
                        0,
                        IntPtr.Zero);
                    break;
                case MODE.WRITE:
                    pipeHandle = CreateNamedPipe(
                        name,
                        PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                        0,
                        255,
                        size,
                        size,
                        0,
                        IntPtr.Zero);
                    break;
            }

            //failed to create named pipe
            if (pipeHandle.IsInvalid)
            {
                Error.Show("NamedPipe: Invalid pipeline handle");
                return null;
            }

#if DEBUG
            Console.WriteLine("OK");
#endif

            return pipeHandle;
        }

        public static void Close(SafeFileHandle handle)
        {
            try
            {
                // result = CloseHandle(handle) != 0;                
            }
            catch (Exception e)
            {
                Error.Show("NamedPipe: Invalid pipeline handle");
            }

            handle = null;
        }
    }

}
