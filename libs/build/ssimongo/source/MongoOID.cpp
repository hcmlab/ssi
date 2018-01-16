// MongoOID.cpp
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

#include "MongoOID.h"
#include "base/String.h"

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

namespace ssi
{
	ssi_char_t *MongoOID::ssi_log_name = "mongooid__";

	MongoOID::MongoOID()
	{
		bson_oid_t *oid = (bson_oid_t*)malloc(sizeof(bson_oid_t));
		bson_oid_init(oid, NULL);
		_oid = oid;
	}

	MongoOID::MongoOID(const ssi_char_t *str)
	{
		bson_oid_t *oid = (bson_oid_t*)malloc(sizeof(bson_oid_t));
		bson_oid_init_from_string(oid, str);		
		_oid = oid;
	}

	MongoOID::MongoOID(void *oid)
	{
		bson_oid_t *to = (bson_oid_t*)malloc(sizeof(bson_oid_t));
		bson_oid_t *from = (bson_oid_t*)oid;
		bson_oid_copy(from, to);
		_oid = to;
	}

	MongoOID::MongoOID(const MongoOID &oid)
	{
		bson_oid_t *tmp = (bson_oid_t*)malloc(sizeof(bson_oid_t));
		bson_oid_copy ((bson_oid_t*)oid._oid, tmp);
		_oid = tmp;
	}

	MongoOID::~MongoOID()
	{		
		free(_oid);
	}

	void *MongoOID::get()
	{
		return _oid;
	}

	ssi_char_t *MongoOID::toString()
	{
		ssi_char_t str[25];
		bson_oid_t *oid = (bson_oid_t*)_oid;
		bson_oid_t copy;
		bson_oid_copy(oid, &copy);
		bson_oid_to_string(&copy, str);
		return ssi_strcpy(str);
	}

	void MongoOID::print(FILE *file)
	{
		ssi_char_t *str = toString();
		ssi_fprint(file, str);
		delete[] str;
	}

}