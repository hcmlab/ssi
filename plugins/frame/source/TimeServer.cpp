// FrameMonitor.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/10/15
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

#include "TimeServer.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *TimeServer::ssi_log_name = "timeserver";

TimeServer::TimeServer (int port)
: _port (port),
	_sock (0) {

	_frame = Factory::GetFramework ();
	Thread::setName (getName());
}

TimeServer::~TimeServer () {
}

void TimeServer::enter () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start time server (port=%d)", _port); 
	_sock = new UdpSocket ();
	_sock->Bind (IpEndpointName ("localhost", _port));

	_close = false;
}

void TimeServer::run () {

	if (!_close) {
		/*
		TcpSocket *client;		
		_sock->create ();		
		_sock->listen (_port);	

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "wait for client (port=%d)", _port); 

		//ssi_tic ();
		_sock->accept (&client);
		//ssi_toc_print ();

		ssi_size_t time = _frame->GetElapsedTimeMs ();
		client->send (&time, sizeof (time));			
		_sock->shutdown (SD_BOTH);	
		_sock->close (); */
		
		char data;
		IpEndpointName client;
		int received = _sock->ReceiveFrom (client, &data, 1, 1000);
		if (received != -1) {
			ssi_size_t time = _frame->GetElapsedTimeMs ();			
			_sock->SendTo (client, ssi_pcast (const char, &time), sizeof (time));

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "time sent (port=%d)", client.port); 
		}

	} else {
		sleep_ms (100);
	}
}

void TimeServer::flush () {	

	delete _sock; _sock = 0;
	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop time server (port=%d)", _port); 
}

void TimeServer::close () {
	_close = true;
}

}
