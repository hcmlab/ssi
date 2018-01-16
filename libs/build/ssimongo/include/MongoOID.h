// MongoOID.h
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

#ifndef SSI_MONGO_OID_H
#define	SSI_MONGO_OID_H

#include "SSI_Cons.h"

#define SSI_MONGO_OID_SIZE 12
#define SSI_MONGO_OID_BYTES SSI_MONGO_OID_SIZE*sizeof(uint8_t)

namespace ssi
{

	class MongoOID
	{

	public:

		MongoOID();
		MongoOID(void *oid);
		MongoOID(const ssi_char_t *str);
		MongoOID(const MongoOID &oid);
		virtual ~MongoOID();
		
		ssi_char_t *toString();		
		void *get();

		void print(FILE *file = stdout);

	protected:

		void *_oid;
		static ssi_char_t *ssi_log_name;		
	};

}

#endif