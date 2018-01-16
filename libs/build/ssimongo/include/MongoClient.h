// MongoClient.h
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

#ifndef SSI_MONGO_CLIENT_H
#define	SSI_MONGO_CLIENT_H

#include "SSI_Cons.h"
#include "MongoCollection.h"
#include "MongoURI.h"

typedef struct _mongoc_client_t mongoc_client_t;
typedef struct _mongoc_uri_t mongoc_uri_t;
typedef struct _mongoc_database_t mongoc_database_t;

namespace ssi
{
	
	class MongoClient
	{

	public:

		MongoClient();
		virtual ~MongoClient();

		bool connect(MongoURI &uri, const ssi_char_t *db_name, bool auto_create = false, ssi_size_t timeout_ms = 1000);		
		bool check();
		bool is_connected();		
		const ssi_char_t *getName();
		void close();

		bool has(MongoCollection &collection);
		bool get(MongoCollection &collection);
		bool add(MongoCollection &collection);

	protected:

		static ssi_char_t *ssi_log_name;

		ssi_char_t *_db_name;
		mongoc_client_t *_client;
		mongoc_database_t *_db;
		ssi_char_t *_name;
		mongoc_uri_t *_uri;		
	};

}

#endif