// PyhonChannel.h
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

#pragma once

#ifndef SSI_PYTHON_CHANNEL_H
#define SSI_PYTHON_CHANNEL_H

#include "base/IChannel.h"

namespace ssi {

class PythonChannel : public IChannel {

public:

	PythonChannel(const ssi_char_t *name, const ssi_char_t *info);
	virtual ~PythonChannel();	

	const ssi_char_t *getName();
	const ssi_char_t *getInfo();
	ssi_stream_t getStream();
	ssi_stream_t* getStreamPtr();

protected:

	ssi_char_t *_name;
	ssi_char_t *_info;
	ssi_stream_t _stream;

};

}

#endif
