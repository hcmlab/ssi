// WekaServer.cpp
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

#include "WekaServer.h"

namespace ssi {

WekaServer::WekaServer (ssi_size_t port,
	ssi_size_t n_buffer,
	const ssi_char_t *model,
	const ssi_char_t *class_path,
	const ssi_char_t *lib_path)
: Thread (true) {

	_port = port;
	_n_buffer = n_buffer;
	_model = ssi_strcpy (model);
	_class_path = ssi_strcpy (class_path);
	_lib_path = ssi_strcpy (lib_path);

	this->setName ("wekaserver");
}

WekaServer::~WekaServer () {

	delete[] _model; _model = 0;
	delete[] _class_path; _class_path = 0;
	delete[] _lib_path; _lib_path = 0;
}

}
