// PyhonChannel.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/03/02
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

#include "PythonChannel.h"

namespace ssi {

PythonChannel::PythonChannel(const ssi_char_t *name, const ssi_char_t *info)
: _name(0),
_info(0)
{
	ssi_stream_init(_stream, 0, 0, 0, SSI_UNDEF, 0, 0);

	_name = ssi_strcpy(name);
	_info = ssi_strcpy(info);
}

PythonChannel::~PythonChannel() 
{
	ssi_stream_destroy(_stream);

	delete[] _name;
	delete[] _info;
}

const ssi_char_t *PythonChannel::getName()
{
	return _name;
}

const ssi_char_t *PythonChannel::getInfo()
{
	return _info;
}

ssi_stream_t PythonChannel::getStream()
{
	return _stream;
}

ssi_stream_t* PythonChannel::getStreamPtr()
{
	return &_stream;
}

}

