// IMessage.h
// author: Florian Lingenfelsr <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/06/12
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

#ifndef SSI_IMESSAGE_H
#define SSI_IMESSAGE_H

namespace ssi {

class IMessage {

public:

	virtual void print (const char* text, ...) = 0;
	virtual void err (const char* logname, const char* file, int line, const char* text, ...) = 0;
	virtual void wrn (const char* logname, const char* file, int line, const char* text, ...) = 0;
        virtual void msg (const char* logname, const char* text, ...) = 0;

};

}

#endif
