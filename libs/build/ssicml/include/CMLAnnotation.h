// CMLAnnotation.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/04
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

#ifndef SSI_CML_ANNOTATION_H
#define SSI_CML_ANNOTATION_H

#include "SSI_Cons.h"

namespace ssi {

class MongoDocument;
class MongoOID;
class MongoClient;
class Annotation;

class CMLAnnotation
{

public:

	static bool Load(Annotation *annotation,
		MongoClient *client,
		const ssi_char_t *session,		
		const ssi_char_t *role,
		const ssi_char_t *scheme,
		const ssi_char_t *annotator,
		bool externalTraining = false);
	static bool Save(Annotation *annotation, 
		MongoClient *client,
		const ssi_char_t *session,		
		const ssi_char_t *role,
		const ssi_char_t *scheme,
		const ssi_char_t *annotator,
		bool is_finished = true,
		bool is_locked = false,
		bool force = false);
	static bool Remove(MongoClient *client,
		const ssi_char_t *session,		
		const ssi_char_t *role,
		const ssi_char_t *scheme,
		const ssi_char_t *annotator);

	static bool SetScheme(Annotation *annotation,
		MongoClient *client,
		const ssi_char_t *scheme, bool externalTraining = false);
	static bool SetScheme(Annotation *annotation,
		MongoDocument *document, bool externalTraining = false);
	static bool AddMongo(Annotation *annotation,
		MongoDocument *document);

protected:

	static ssi_char_t *ssi_log_name_static;

	static MongoOID *GetOID(MongoClient *client, 
		const ssi_char_t *collection, 
		const ssi_char_t *name);
};

}

#endif
