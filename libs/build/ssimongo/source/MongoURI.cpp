// MongoURI.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/19
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

#include "MongoURI.h"
#include "base/String.h"

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

namespace ssi
{
	ssi_char_t *MongoURI::ssi_log_name = "mongouri__";

	MongoURI::MongoURI(const ssi_char_t *ip, ssi_size_t port, const ssi_char_t *username, const ssi_char_t *password)
	{
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_sprint(address, "%s:%u", ip, port);

		_address = ssi_strcpy(address);

		ssi_char_t uri[SSI_MAX_CHAR];
		ssi_sprint(uri, "mongodb://%s:%s@%s", username, password, address);

		_uri = ssi_strcpy(uri);		
	}
	
	MongoURI::~MongoURI()
	{
		delete[] _address; _address = 0;
		delete[] _uri; _uri = 0;
	}

	const ssi_char_t *MongoURI::getURI()
	{
		return _uri;
	}

	const ssi_char_t *MongoURI::getAddress()
	{
		return _address;
	}

}