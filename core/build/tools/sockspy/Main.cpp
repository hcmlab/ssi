// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/02
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "SocketSpy.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	//**** READ COMMAND LINE ****//

	CmdArgParser cmd;
	cmd.info (info);

	int port;
	ssi_char_t *debug = 0;
	ssi_char_t *host = 0;
	ssi_char_t *work_dir = 0;
	int buffer_size;
	int type;
	ssi_video_params_t vparams;
	int protocol;
	int dim;
	ssi_char_t *fork_urls = 0;

	cmd.addMasterSwitch ("--binary");

	cmd.addText("\nArguments:");
	cmd.addICmdArg("port", &port, "port");

	cmd.addText ("\nOptions:");
	cmd.addSCmdOption ("-host", &host, "", "host (if not set, any) \t\t[<any>]");
	cmd.addSCmdOption ("-dir", &work_dir, ".", "working directory \t\t[.]");
	cmd.addSCmdOption ("-debug", &debug, "", "directs debug output to file \t[]");
	cmd.addICmdOption ("-protocol", &protocol, Socket::UDP, "protocol (0=UDP,1=TCP) \t[0]");
	cmd.addICmdOption ("-buffer", &buffer_size, Socket::MAX_MTU_SIZE, "max buffer size in bytes\t[MTU size]");
	cmd.addICmdOption ("-type", &type, SSI_FLOAT, "sample type (1=CHAR, 2=UCHAR, 3=SHORT 4=USHORT, 5=INT, 6=UINT, 7=LONG, 8=ULONG, 9=FLOAT, 10=DOUBLE, 11=LDOUBLE, 14=BOOL)");			
	cmd.addICmdOption ("-dim", &dim, 1, "samples dimension \t[1]");				

	cmd.addMasterSwitch ("--ascii");

	cmd.addText("\nArguments:");
	cmd.addICmdArg("port", &port, "port");

	cmd.addText ("\nOptions:");
	cmd.addSCmdOption ("-host", &host, "", "host (if not set, any) \t\t[<any>]");
	cmd.addSCmdOption ("-dir", &work_dir, ".", "working directory \t\t[.]");
	cmd.addSCmdOption ("-debug", &debug, "", "directs debug output to file \t[]");
	cmd.addICmdOption ("-protocol", &protocol, Socket::UDP, "protocol (0=UDP,1=TCP) \t[0]");
	cmd.addICmdOption ("-buffer", &buffer_size, Socket::MAX_MTU_SIZE, "max buffer size in bytes\t[MTU size]");

	cmd.addMasterSwitch ("--osc");

	cmd.addText("\nArguments:");
	cmd.addICmdArg("port", &port, "port");

	ssi_time_t plot_update = 0;
	bool log_binary, log_ascii, log_header, log_console;
	char *log_delim = 0, *log_format = 0;

	cmd.addText ("\nOptions:");
	cmd.addSCmdOption ("-host", &host, "", "host (if not set, any) \t\t[<any>]");
	cmd.addSCmdOption ("-dir", &work_dir, ".", "working directory \t\t[.]");
	cmd.addSCmdOption ("-debug", &debug, "", "directs debug output to file \t[]");
	cmd.addICmdOption ("-protocol", &protocol, Socket::UDP, "protocol (0=UDP,1=TCP) \t[0]");
	cmd.addBCmdOption ("-console", &log_console, false, "print stream on console \t\t[false]");
	cmd.addBCmdOption ("-binary", &log_binary, false, "log streams (binary) \t\t[false]");
	cmd.addBCmdOption ("-ascii", &log_ascii, false, "log streams (ascii) \t\t[false]");
	cmd.addSCmdOption ("-delim", &log_delim, " ", "ascii delimiter \t\t\t[ ]");
	cmd.addSCmdOption ("-format", &log_format, ".5", "ascii format \t\t\t[.5]");
	cmd.addBCmdOption ("-header", &log_header, false, "add header \t\t\t[false]");

	cmd.addMasterSwitch ("--image");

	cmd.addText("\nArguments:");
	cmd.addICmdArg("port", &port, "port");

	cmd.addText ("\nOptions:");
	cmd.addSCmdOption ("-host", &host, "", "host (if not set, any) \t\t[<any>]");
	cmd.addSCmdOption ("-dir", &work_dir, ".", "working directory \t\t[.]");
	cmd.addSCmdOption ("-debug", &debug, "", "directs debug output to file \t[]");
	cmd.addICmdOption ("-protocol", &protocol, Socket::UDP, "protocol (0=UDP,1=TCP) \t[0]");
	cmd.addICmdOption ("-buffer", &buffer_size, Socket::MAX_MTU_SIZE, "max buffer size in bytes\t[MTU size]");
	cmd.addICmdOption ("-width", &vparams.widthInPixels, 640, "video width in pixels\t[640]");
	cmd.addICmdOption ("-height", &vparams.heightInPixels, 480, "video height in pixels\t[480]");
	cmd.addICmdOption ("-depth", &vparams.depthInBitsPerChannel, 8, "depth per pixel in bits \t[8]");
	cmd.addICmdOption ("-channels", &vparams.numOfChannels, 3, "channels per pixel\t[3]");	

	cmd.addMasterSwitch ("--fork");

	cmd.addText("\nArguments:");
	cmd.addICmdArg("port", &port, "port");
	cmd.addSCmdArg("urls", &fork_urls, "fork urls seperated by ; (syntax <host>:<port>)");

	cmd.addText ("\nOptions:");
	cmd.addSCmdOption ("-host", &host, "", "host (if not set, any) \t\t[<any>]");
	cmd.addSCmdOption ("-dir", &work_dir, ".", "working directory \t\t[.]");
	cmd.addBCmdOption ("-console", &log_console, false, "debug on console \t\t[false]");
	cmd.addSCmdOption ("-debug", &debug, "", "directs debug output to file \t[]");
	cmd.addICmdOption ("-protocol", &protocol, Socket::UDP, "protocol (0=UDP,1=TCP) \t[0]");
	cmd.addICmdOption ("-buffer", &buffer_size, Socket::MAX_MTU_SIZE, "max buffer size in bytes\t[MTU size]");

	cmd.addMasterSwitch ("--peek");
	cmd.addText("\nArguments:");
	cmd.addICmdArg("port", &port, "port");
	
	cmd.addText ("\nOptions:");
	cmd.addSCmdOption ("-host", &host, "", "host (if not set, any) \t\t[<any>]");
	cmd.addSCmdOption ("-dir", &work_dir, ".", "working directory \t\t[.]");
	cmd.addBCmdOption ("-console", &log_console, false, "debug on console \t\t[false]");
	cmd.addSCmdOption ("-debug", &debug, "", "directs debug output to file \t[]");
	cmd.addICmdOption ("-protocol", &protocol, Socket::UDP, "protocol (0=UDP,1=TCP) \t[0]");
	cmd.addICmdOption ("-buffer", &buffer_size, Socket::MAX_MTU_SIZE, "max buffer size in bytes\t[MTU size]");

	cmd.addText ("\n\n!!! Please note that command arguments are case sensitive !!!");

	//AC_HANNING, AC_GAUSS, FCC_NORMAL, FCC_ACCURATE

	if (cmd.read (argc, argv)) {

		if (debug && debug[0] != '\0') {
			ssimsg = new FileMessage (debug);
		}

		// print build version
		ssi_print ("%s", info);

		// get timestamp
		ssi_char_t now[ssi_time_size];
		ssi_now (now);

		// create work dir
		ssi_mkdir (work_dir);

		// spy options
		SocketSpy::options opts;

		switch (cmd.master_switch) {

			case 1: {
				
				opts.format = SocketSpy::FORMAT::BINARY;
				opts.protocol = ssi_cast (Socket::TYPE, protocol);
				opts.port = port;
				opts.host = host;
				opts.work_dir = work_dir;
				opts.buffer_size = buffer_size;
				opts.type = ssi_cast (ssi_type_t, type);
				opts.dim = dim;

				break;
			}

			case 2: {
				
				opts.format = SocketSpy::FORMAT::ASCII;
				opts.protocol = ssi_cast (Socket::TYPE, protocol);
				opts.port = port;
				opts.host = host;
				opts.work_dir = work_dir;
				opts.buffer_size = buffer_size;

				break;
			}

			case 3: {
			
				opts.format = SocketSpy::FORMAT::OSC;
				opts.protocol = ssi_cast (Socket::TYPE, protocol);
				opts.port = port;
				opts.host = host;
				opts.work_dir = work_dir;
				opts.now = now;
				opts.log_ascii = log_ascii;
				opts.log_binary = log_binary;
				opts.log_console = log_console;
				opts.log_delim = log_delim;
				opts.log_format = log_format;
				opts.log_header = log_header;		

				break;
			}		

			case 4: {
				
				opts.format = SocketSpy::FORMAT::IMAGE;
				opts.protocol = ssi_cast (Socket::TYPE, protocol);
				opts.port = port;
				opts.host = host;
				opts.work_dir = work_dir;
				opts.buffer_size = buffer_size;
				opts.vparams = vparams;

				break;
			}

			case 5: {
				
				opts.format = SocketSpy::FORMAT::FORK;
				opts.protocol = ssi_cast (Socket::TYPE, protocol);
				opts.port = port;
				opts.host = host;
				opts.log_console = log_console;
				opts.work_dir = work_dir;
				opts.buffer_size = buffer_size;
				opts.fork_urls = fork_urls;

				break;
			}

			case 6: {
				
				opts.format = SocketSpy::FORMAT::PEEK;
				opts.protocol = ssi_cast (Socket::TYPE, protocol);
				opts.port = port;
				opts.host = host;
				opts.log_console = log_console;
				opts.work_dir = work_dir;
				opts.buffer_size = buffer_size;

				break;
			}

		}

		// start spy
		SocketSpy spy (opts);
		spy.start ();

		// wait for key
		ssi_print ("\nPress enter to stop!\n");
		getchar ();

		//stop spy
		spy.stop ();

		if (debug && debug[0] != '\0') {
			delete ssimsg;
			ssimsg = 0;
		}
	}

	delete[] work_dir;
	delete[] fork_urls;
	delete[] log_delim;
	delete[] log_format;
	delete[] host;
	delete[] debug;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
