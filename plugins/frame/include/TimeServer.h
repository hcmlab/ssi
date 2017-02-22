// TimeServer.h
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

#pragma once

#ifndef SSI_FRAME_TIMESERVER_H
#define SSI_FRAME_TIMESERVER_H

#include "base/ITheFramework.h"
#include "ioput/socket/ip/UdpSocket.h"
#include "thread/Thread.h"

namespace ssi {

class TimeServer : public Thread {

public:

	TimeServer (int port);
	~TimeServer ();

	void enter ();
	void run ();
	void flush ();
	void close ();

private:
	
	static ssi_char_t *ssi_log_name;

	ITheFramework *_frame;
	UdpSocket *_sock;
	int _port;
	bool _close;
};

}

#endif
