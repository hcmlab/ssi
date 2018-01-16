// MongoDocument.h
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
//*************************************************************************************************

#pragma once

#ifndef SSI_MONGO_DOCUMENT_H
#define	SSI_MONGO_DOCUMENT_H

#include "SSI_Cons.h"

typedef struct _bson_t bson_t;

namespace ssi
{
	class MongoOID;
	class MongoDate;
	class MongoDocuments;

	class MongoDocument
	{

	friend class MongoCollection;

	public:

		class DocumentVisitor
		{
		public:
			virtual void visit(MongoDocument &document, void *data) = 0;
		};

		struct document_visitor_arg_t {
			DocumentVisitor *visitor;
			void *data;
		};

	public:

		MongoDocument();
		MongoDocument(const MongoDocument &document);
		MongoDocument(bson_t *borrow);
		MongoDocument(MongoOID *value);
		MongoDocument(const ssi_char_t *key, MongoOID *value);
		MongoDocument(const ssi_char_t *key, const ssi_char_t *value);
		MongoDocument(const ssi_char_t *key, double value);
		MongoDocument(const ssi_char_t *key, int32_t value);
		MongoDocument(const ssi_char_t *key, int64_t value);
		MongoDocument(const ssi_char_t *key, bool value);
		MongoDocument(const ssi_char_t *key, MongoDate *value);
		virtual ~MongoDocument();

		void clear();

		MongoOID *getOid();
		MongoOID *getOid(const ssi_char_t *key);
		bool setOid(MongoOID *oid);
		bool setOid(const ssi_char_t *key, MongoOID *oid);
		
		bool getBool(const ssi_char_t *key, bool &value);
		bool setBool(const ssi_char_t *key, bool value);

		ssi_char_t *getString(const ssi_char_t *key, bool convertToUtf8 = true);
		bool setString(const ssi_char_t *key, const ssi_char_t *value, bool convertToUtf8 = true);

		bool getInt32(const ssi_char_t *key, int32_t &value);
		bool setInt32(const ssi_char_t *key, int32_t value);

		bool getInt64(const ssi_char_t *key, int64_t &value);
		bool setInt64(const ssi_char_t *key, int64_t value);

		bool getDouble(const ssi_char_t *key, double &value);
		bool setDouble(const ssi_char_t *key, double value);

		MongoDate *getDate(const ssi_char_t *key);
		bool setDate(const ssi_char_t *key, MongoDate *date);

		bool getArray(const ssi_char_t *key, DocumentVisitor &visitor, void *data);		
		bool setArray(const ssi_char_t *key, MongoDocuments &documents);
		
		static ssi_char_t *UnicodeToUTF8(const ssi_char_t *str);
		static ssi_char_t *UTF8ToUnicode(const ssi_char_t *str);

		void print(FILE *file = stdout);

	protected:

		static ssi_char_t *ssi_log_name;

		bson_t *get();
		bson_t *_document;

		bool _borrowed;
	};

	class MongoDocuments : public std::vector<MongoDocument>
	{

	public:

		void print(FILE *file = stdout);
	};
}

#endif