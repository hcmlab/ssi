// MongoDBTools.cpp
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

#include "MongoDBTools.h"

namespace ssi
{

ssi_char_t *MongoDBTools::ssi_log_name = "mongotools";

bool MongoDBTools::Write(const ssi_char_t *url, const ssi_char_t *database_id, const ssi_char_t *collection_id, bson_t &document)
{
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;

	client = mongoc_client_new(url);
	collection = mongoc_client_get_collection(client, database_id, collection_id);

	if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, &document, NULL, &error))
	{
		ssi_wrn_static("write operation failed '%s'", error.message);
		return false;
	}

	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);

	return true;
}

bool MongoDBTools::Read(const ssi_char_t *url, const ssi_char_t *database_id, const ssi_char_t *collection_id, bson_t &document, bson_t &query)
{
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	mongoc_cursor_t *cursor;
	const bson_t *result;

	client = mongoc_client_new(url);
	collection = mongoc_client_get_collection(client, database_id, collection_id);
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 1, 0, &query, NULL, NULL);

	bool found = false;
	if (mongoc_cursor_next(cursor, &result))
	{
		found = true;
		bson_copy_to(result, &document);
	}

	mongoc_cursor_destroy(cursor);
	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);

	return found;
}

bool MongoDBTools::Remove(const ssi_char_t *url, const ssi_char_t *database_id, const ssi_char_t *collection_id, bson_t &document)
{
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;

	client = mongoc_client_new(url);
	collection = mongoc_client_get_collection(client, database_id, collection_id);

	bool result = !mongoc_collection_remove(collection, MONGOC_REMOVE_SINGLE_REMOVE, &document, NULL, &error);
	if (result)
	{
		ssi_wrn_static("remove operation failed '%s'", error.message);
	}

	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);

	return result;
}

bool MongoDBTools::Update(const ssi_char_t *url, const ssi_char_t *database_id, const ssi_char_t *collection_id, bson_t &query, bson_t &update)
{
	mongoc_client_t *client;
	mongoc_collection_t *collection;
	bson_error_t error;

	client = mongoc_client_new(url);
	collection = mongoc_client_get_collection(client, database_id, collection_id);

	bool result = mongoc_collection_update(collection, MONGOC_UPDATE_NONE, &query, &update, NULL, &error);
	if (!result)
	{
		printf("update failed '%s'", error.message);
	}

	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);

	return result;
}

bool MongoDBTools::Check(const ssi_char_t *url, const ssi_char_t *database_id, const ssi_char_t *collection_id, bson_t &query)
{
	bson_t document;
	bson_init(&document);
	bool result = Read(url, database_id, collection_id, document, query);
	bson_destroy(&document);

	return result;
}

const bson_value_t *bson_doc_get_oid(bson_t &document, const ssi_char_t *key)
{
	bson_iter_t iter;

	if (bson_iter_init_find(&iter, &document, key))
	{
		return bson_iter_value(&iter);
	}

	return 0;
}

}