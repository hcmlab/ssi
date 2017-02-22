// WekaServer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/03/04
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_WEKASERVER_H
#define SSI_WEKASERVER_H

#include "thread/Thread.h"
#include <jni.h>

namespace ssi {

class WekaServer : public Thread {

public:

	WekaServer (ssi_size_t port,
		ssi_size_t n_buffer,
		const ssi_char_t *model,
		const ssi_char_t *class_path,
		const ssi_char_t *lib_path);
	virtual ~WekaServer ();

protected:

	ssi_size_t _port;
	ssi_size_t _n_buffer;
	ssi_char_t *_model;
	ssi_char_t *_class_path;
	ssi_char_t *_lib_path;
	
};

}

#endif
