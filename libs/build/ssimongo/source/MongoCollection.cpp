// MongoCollection.cpp
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

#include "MongoCollection.h"
#include "BsonTools.h"

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

namespace ssi
{
	ssi_char_t *MongoCollection::ssi_log_name = "mongocoll_";

	MongoCollection::MongoCollection(const ssi_char_t *name)
	: _collection(0)
	{
		_name = ssi_strcpy(name);
	}

	MongoCollection::~MongoCollection()
	{
		delete[] _name;

		if (_collection)
		{
			mongoc_collection_destroy(_collection);
		}
	}

	void MongoCollection::print(FILE *file)
	{
		if (_collection)
		{
			mongoc_cursor_t *cursor;
			bson_error_t error;
			const bson_t *doc;
			bson_t *query;

			query = bson_new();
			cursor = mongoc_collection_find(_collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

			while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
				BsonTools::Print(doc, file);
			}

			if (mongoc_cursor_error(cursor, &error)) {
				ssi_wrn("could not print colleciton '%s'", error.message);
			}

			mongoc_cursor_destroy(cursor);
			bson_destroy(query);
		}
	}

	const ssi_char_t *MongoCollection::getName()
	{
		return _name;
	}

	void MongoCollection::set(mongoc_collection_t *collection)
	{
		if (_collection)
		{
			mongoc_collection_destroy(_collection);
		}
		_collection = collection;
	}

	bool MongoCollection::findFirst(MongoDocument &query, MongoDocument &document)
	{
		if (!_collection)
		{
			ssi_wrn("collection not initalized '%s'", _name);
			return false;
		}

		bool found = false;
		const bson_t *result = 0;

		mongoc_cursor_t *cursor = mongoc_collection_find(_collection, MONGOC_QUERY_NONE, 0, 1, 0, query.get(), NULL, NULL);
		if (mongoc_cursor_next(cursor, &result))
		{
			found = true;
			bson_copy_to(result, document.get());
		}

		return found;
	}

	bool MongoCollection::find(MongoDocument &query, MongoDocuments &documents)
	{
		if (!_collection)
		{
			ssi_wrn("collection not initalized '%s'", _name);
			return false;
		}

		bool found = false;
		const bson_t *result = 0;

		mongoc_cursor_t *cursor = mongoc_collection_find(_collection, MONGOC_QUERY_NONE, 0, 0, 0, query.get(), NULL, NULL);
		while (mongoc_cursor_next(cursor, &result))
		{
			found = true;
			MongoDocument document;
			bson_copy_to(result, document.get());
			documents.push_back(document);
		}

		return found;
	}

	bool MongoCollection::insert(MongoDocument &document)
	{
		if (!_collection)
		{
			ssi_wrn("collection not initalized '%s'", _name);
			return false;
		}

		bson_error_t error;
		mongoc_write_concern_t *concern = mongoc_write_concern_new(); //MONGOC_WRITE_CONCERN_W_DEFAULT;
		if (!mongoc_collection_insert(_collection, MONGOC_INSERT_NONE, document.get(), concern, &error))
		{
			ssi_wrn("insertion failed '%s'", error.message);
			return false;
		}
		mongoc_write_concern_destroy(concern);

		return true;
	}

	bool MongoCollection::update(MongoDocument &query, MongoDocument &document)
	{
		if (!_collection)
		{
			ssi_wrn("collection not initalized '%s'", _name);
			return false;
		}

		bson_error_t error;
		bson_t *update = bson_new();
		bson_append_document(update, "$set", -1, document.get());

		bool result = mongoc_collection_update(_collection, MONGOC_UPDATE_NONE, query.get(), update, NULL, &error);
		if (!result)
		{
			ssi_wrn("update failed '%s'", error.message);
		}
		
		bson_destroy(update);

		return result;
	}

	bool  MongoCollection::remove(MongoDocument &query)
	{
		if (!_collection)
		{
			ssi_wrn("collection not initalized '%s'", _name);
			return false;
		}

		bson_error_t error;

		bool result = mongoc_collection_remove(_collection, MONGOC_REMOVE_SINGLE_REMOVE, query.get(), NULL, &error);
		if (!result)
		{
			printf("deletion failed '%s'", error.message);
		}

		return result;
	}
}