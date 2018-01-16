// BsonTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/17
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

#ifndef SSI_MONGO_BSONTOOLS
#define SSI_MONGO_BSONTOOLS

#include "SSI_Cons.h"

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

namespace ssi {

class BsonTools
{

public:

	static const ssi_char_t *OID;
	static const ssi_size_t N_OID;

	static void Print(const bson_t *bson, FILE *file = stdout);
	static bool Oid(const bson_t *bson, bson_oid_t &oid);
	static const bson_value_t *Value(const bson_t *bson, const ssi_char_t *key);
	static bool Value(const bson_value_t *value, bson_type_t type, void *ptr);
	static bool SubDoc(const bson_t *bson, const ssi_char_t *key, bson_iter_t &iter);
	static void Init(bson_visitor_t &visitor);

protected:

	static ssi_char_t *ssi_log_name;

};

}

#endif
