// BsonTools.cpp
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

#include "BsonTools.h"

namespace ssi
{

ssi_char_t *BsonTools::ssi_log_name = "bsontools_";
const ssi_char_t *BsonTools::OID = "_id";
const ssi_size_t BsonTools::N_OID = 25;

void BsonTools::Print(const bson_t *bson, FILE *file)
{
	char *str = bson_as_json(bson, NULL);
	ssi_fprint(file, "%s\n", str);
	bson_free(str);
}

const bson_value_t *BsonTools::Value(const bson_t *bson, const ssi_char_t *key)
{
	bson_iter_t iter;

	if (bson_iter_init_find(&iter, bson, key))
	{
		return bson_iter_value(&iter);
	}

	return 0;
}

bool BsonTools::Value(const bson_value_t *value, bson_type_t type, void *ptr)
{
	if (value)
	{
		if (value->value_type == type)
		{
			switch (type)
			{
			case BSON_TYPE_BOOL:
			{
				*((bool *)ptr) = value->value.v_bool;
				return true;
			}
			case BSON_TYPE_UTF8:
			{
				ssi_strcpy((ssi_char_t*)ptr, value->value.v_utf8.str);
				return true;
			}
			case BSON_TYPE_INT32:
			{
				*((int32_t *)ptr) = value->value.v_int32;
				return true;
			}
			case BSON_TYPE_INT64:
			{
				*((int64_t *)ptr) = value->value.v_int64;
				return true;
			}
			case BSON_TYPE_DOUBLE:
			{
				*((double *)ptr) = value->value.v_double;
				return true;
			}
			}
		}
	}

	return false;
}

bool BsonTools::Oid(const bson_t *bson, bson_oid_t &oid)
{
	if (bson_has_field(bson, OID))
	{
		const bson_value_t *value = Value(bson, OID);
		bson_oid_copy(&value->value.v_oid, &oid);

		return true;
	}

	return false;
}

bool BsonTools::SubDoc(const bson_t *bson, const ssi_char_t *key, bson_iter_t &iter)
{
	if (bson_iter_init_find(&iter, bson, key))
	{
		if ((BSON_ITER_HOLDS_DOCUMENT(&iter) ||
			BSON_ITER_HOLDS_ARRAY(&iter)))
		{
			return true;
		}
	}

	return false;
}

void BsonTools::Init(bson_visitor_t &visitor)
{
	visitor.visit_before = 0;
	visitor.visit_after = 0;
	visitor.visit_corrupt = 0;
	visitor.visit_double = 0;
	visitor.visit_utf8 = 0;
	visitor.visit_document = 0;
	visitor.visit_array = 0;
	visitor.visit_binary = 0;
	visitor.visit_undefined = 0;
	visitor.visit_oid = 0;
	visitor.visit_bool = 0;
	visitor.visit_date_time = 0;
	visitor.visit_null = 0;
	visitor.visit_regex = 0;
	visitor.visit_dbpointer = 0;
	visitor.visit_code = 0;
	visitor.visit_symbol = 0;
	visitor.visit_codewscope = 0;
	visitor.visit_int32 = 0;
	visitor.visit_timestamp = 0;
	visitor.visit_int64 = 0;
	visitor.visit_maxkey = 0;
	visitor.visit_minkey = 0;
	visitor.visit_undefined = 0;
}

}