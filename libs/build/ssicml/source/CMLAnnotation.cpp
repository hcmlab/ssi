// CMLAnnotation.cpp
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

#include "CMLAnnotation.h"
#include "CMLCons.h"
#include "CMLVisitor.h"
#include "MongoClient.h"
#include "MongoDocument.h"
#include "MongoOID.h"
#include "MongoDate.h"
#include "Annotation.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *CMLAnnotation::ssi_log_name_static = "annotation";

bool CMLAnnotation::Remove(MongoClient *client,
	const ssi_char_t *session,
	const ssi_char_t *role,
	const ssi_char_t *scheme,
	const ssi_char_t *annotator)
{
	ssi_msg_static(SSI_LOG_LEVEL_BASIC, "remove annotation '%s'", client->getName());
	if (ssi_log_level >= SSI_LOG_LEVEL_BASIC)
	{
		ssi_print_off("session   = %s\n", session);
		ssi_print_off("annotator = %s\n", annotator);
		ssi_print_off("role      = %s\n", role);
		ssi_print_off("scheme    = %s\n", scheme);
	}

	// get ids

	MongoOID *session_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SESSIONS], session);
	MongoOID *annotator_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATORS], annotator);
	MongoOID *role_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ROLES], role);
	MongoOID *scheme_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SCHEMES], scheme);

	if (!(session_id && annotator_id && role_id && scheme_id))
	{
		return false;
	}

	// get annotation

	MongoDocument query;

	MongoCollection annotation_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATIONS]);
	if (!client->get(annotation_collection))
	{
		return false;
	}

	MongoCollection annotation_data_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATIONS_DATA]);
	if (!client->get(annotation_data_collection))
	{
		return false;
	}

	query.clear();
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SESSION_ID], session_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ANNOTATOR_ID], annotator_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ROLE_ID], role_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCHEME_ID], scheme_id);

	MongoDocument annotation_document;
	if (annotation_collection.findFirst(query, annotation_document))
	{			
		{
			MongoOID *oid_data = annotation_document.getOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATA_ID]);

			ssi_char_t *oid_s = oid_data->toString();
			ssi_msg_static(SSI_LOG_LEVEL_BASIC, "remove annotation data '%s'", oid_s);			
			delete[] oid_s;

			MongoDocument query_data;
			query_data.setOid(oid_data);
			if (!annotation_data_collection.remove(query_data))
			{
				return false;
			}

			delete oid_data;
		}

		{
			MongoOID *oid = annotation_document.getOid();		

			ssi_char_t *oid_s = oid->toString();
			ssi_msg_static(SSI_LOG_LEVEL_BASIC, "remove annotation '%s'", oid_s);
			delete oid;
			delete[] oid_s;

			if (!annotation_collection.remove(query))
			{
				return false;
			}
		}
	}

	delete session_id;
	delete annotator_id;
	delete role_id;
	delete scheme_id;

	return true;
}


bool CMLAnnotation::Load(Annotation *annotation, 
	MongoClient *client,
	const ssi_char_t *session,
	const ssi_char_t *role,
	const ssi_char_t *scheme,
	const ssi_char_t *annotator,
	bool externalTraining)
{
	ssi_msg_static(SSI_LOG_LEVEL_BASIC, "load annotation '%s'", client->getName());
	if (ssi_log_level >= SSI_LOG_LEVEL_BASIC)
	{
		ssi_print_off("session   = %s\n", session);
		ssi_print_off("annotator = %s\n", annotator);
		ssi_print_off("role      = %s\n", role);
		ssi_print_off("scheme    = %s\n", scheme);
	}

	MongoDocument query;

	// get ids

	MongoOID *session_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SESSIONS], session);
	MongoOID *annotator_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATORS], annotator);
	MongoOID *role_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ROLES], role);
	MongoOID *scheme_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SCHEMES], scheme);

	if (!(session_id && annotator_id && role_id && scheme_id))
	{
		return false;
	}

	// get scheme

	MongoCollection scheme_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SCHEMES]);
	if (!client->get(scheme_collection))
	{
		return false;
	}

	query.setOid(scheme_id);

	MongoDocument scheme_document;
	if (!scheme_collection.findFirst(query, scheme_document))
	{
		ssi_wrn("scheme not found '%s'", scheme_id->toString());
		return false;
	}

	if (!SetScheme(annotation, &scheme_document, externalTraining))
	{
		return false;
	}

	// get annotation

	MongoCollection annotation_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATIONS]);
	if (!client->get(annotation_collection))
	{
		return false;
	}

	MongoCollection annotation_data_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATIONS_DATA]);
	if (!client->get(annotation_data_collection))
	{
		return false;
	}

	query.clear();
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SESSION_ID], session_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ANNOTATOR_ID], annotator_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ROLE_ID], role_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCHEME_ID], scheme_id);

	MongoDocument annotation_document;	
	if (annotation_collection.findFirst(query, annotation_document))
	{	
		MongoOID *id = annotation_document.getOid();

		{
			ssi_char_t *oid_s = id->toString();
			ssi_msg_static(SSI_LOG_LEVEL_BASIC, "load annotation '%s'", oid_s);
			delete[] oid_s;
		}

		MongoOID *data_id = annotation_document.getOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATA_ID]);
		
		{
			ssi_char_t *oid_s = data_id->toString();
			ssi_msg_static(SSI_LOG_LEVEL_BASIC, "load annotation data '%s'", oid_s);
			delete[] oid_s;
		}

		MongoDocument query_data;
		query_data.setOid(data_id);

		MongoDocument annotation_data_document;
		if (annotation_data_collection.findFirst(query_data, annotation_data_document))
		{
			if (!AddMongo(annotation, &annotation_data_document))
			{
				return false;
			}
		}
		else
		{
			ssi_msg_static(SSI_LOG_LEVEL_BASIC, "annotation data not found");
			return false;
		}

		delete id;
		delete data_id;
	}	
	else
	{
		ssi_msg_static(SSI_LOG_LEVEL_BASIC, "annotation not found");
		return false;
	}

	// set meta

	annotation->setMeta("annotator", annotator);
	annotation->setMeta("role", role);

	delete session_id;
	delete annotator_id;
	delete role_id;
	delete scheme_id;

	return true;
}

bool CMLAnnotation::Save(Annotation *annotation,
	MongoClient *client,
	const ssi_char_t *session,
	const ssi_char_t *role,
	const ssi_char_t *scheme,
	const ssi_char_t *annotator,
	bool is_finished,
	bool is_locked,
	bool force)
{
	ssi_msg_static(SSI_LOG_LEVEL_BASIC, "save annotation '%s'", client->getName());
	if (ssi_log_level >= SSI_LOG_LEVEL_BASIC)
	{
		ssi_print_off("session   = %s\n", session);
		ssi_print_off("annotator = %s\n", annotator);
		ssi_print_off("role      = %s\n", role);
		ssi_print_off("scheme    = %s\n", scheme);
	}	

	MongoDocument query;

	MongoOID *session_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SESSIONS], session);
	MongoOID *annotator_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATORS], annotator);
	MongoOID *role_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ROLES], role);
	MongoOID *scheme_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SCHEMES], scheme);

	if (!(session_id && annotator_id && role_id && scheme_id))
	{
		return false;
	}

	MongoCollection annotation_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATIONS]);
	if (!client->get(annotation_collection))
	{
		return false;
	}

	MongoCollection annotation_data_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::ANNOTATIONS_DATA]);
	if (!client->get(annotation_data_collection))
	{
		return false;
	}

	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SESSION_ID], session_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ANNOTATOR_ID], annotator_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ROLE_ID], role_id);
	query.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCHEME_ID], scheme_id);

	// update meta

	MongoOID *data_id = new MongoOID();;
	MongoOID *data_id_old = 0;

	MongoDocument annotation_document;
	if (!annotation_collection.findFirst(query, annotation_document))
	{	
		MongoOID oid;
		annotation_document.setOid(&oid);
		annotation_document.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SESSION_ID], session_id);
		annotation_document.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ANNOTATOR_ID], annotator_id);
		annotation_document.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::ROLE_ID], role_id);
		annotation_document.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCHEME_ID], scheme_id);
		annotation_document.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATA_ID], data_id);

		{
			ssi_char_t *oid_s = oid.toString();
			ssi_msg_static(SSI_LOG_LEVEL_BASIC, "insert annotation '%s'", oid_s);
			delete[] oid_s;
		}

		if (!annotation_collection.insert(annotation_document))
		{
			return false;
		}		
	}
	else
	{
		MongoOID *oid = annotation_document.getOid();
		ssi_char_t *oid_s = oid->toString();

		bool is_already_locked = false;
		if (annotation_document.getBool(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::IS_LOCKED], is_already_locked))
		{
			if (is_already_locked && !force)
			{
				ssi_msg_static(SSI_LOG_LEVEL_BASIC, "annotation is locked '%s'", oid_s);
				return false;
			}
		}

		// query old anno and remove
		data_id_old = annotation_document.getOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATA_BACKUP_ID]);
		if (data_id_old)
		{
			MongoDocument query_old;
			query_old.setOid(data_id_old);
			annotation_data_collection.remove(query_old);

			ssi_char_t *oid_s = data_id_old->toString();
			ssi_msg_static(SSI_LOG_LEVEL_BASIC, "remove annotation data '%s'", oid_s);
			delete[] oid_s;

			delete data_id_old;
			data_id_old = 0;
		}

		// current anno becomes old anno
		data_id_old = annotation_document.getOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATA_ID]);

		ssi_msg_static(SSI_LOG_LEVEL_BASIC, "update annotation '%s'", oid_s);

		delete oid;
		delete[] oid_s;
	}

	// update annotation
	{
		MongoDocument update;		
		update.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATA_ID], data_id);
		if (data_id_old)
		{
			update.setOid(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATA_BACKUP_ID], data_id_old);
		}
		MongoDate date;
		update.setDate(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::DATE], &date);
		update.setBool(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::IS_FINISHED], is_finished);
		update.setBool(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::IS_LOCKED], is_locked);		

		if (!annotation_collection.update(query, update))
		{
			return false;
		}
	}

	// insert new annotation data

	MongoDocuments labels;
	for (Annotation::iterator it = annotation->begin(); it != annotation->end(); it++)
	{
		MongoDocument label;
		label.setDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_CONF], it->confidence);
		switch (annotation->getScheme()->type)
		{
		case SSI_SCHEME_TYPE::DISCRETE:
		{			
			label.setDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_FROM], it->discrete.from);
			label.setDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_TO], it->discrete.to);
			label.setInt32(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_ID], it->discrete.id);
		}
		break;
		case SSI_SCHEME_TYPE::CONTINUOUS:
		{
			label.setDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_SCORE], it->continuous.score);
		}
		break;
		case SSI_SCHEME_TYPE::FREE:
		{		
			label.setDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_FROM], it->discrete.from);
			label.setDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_TO], it->discrete.to);
			label.setString(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_NAME], it->free.name);
		}
		break;
		}
		labels.push_back(label);
	}

	{
		MongoDocument insert;
		insert.setOid(data_id);
		insert.setArray(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABELS], labels);		
		if (!annotation_data_collection.insert(insert))
		{
			return false;
		}

		ssi_char_t *oid_s = data_id->toString();
		ssi_msg_static(SSI_LOG_LEVEL_BASIC, "insert annotation data '%s'", oid_s);
		delete[] oid_s;
	}

	delete session_id;
	delete annotator_id;
	delete role_id;
	delete scheme_id;
	delete data_id;

	return true;
}

MongoOID *CMLAnnotation::GetOID(MongoClient *client, 
	const ssi_char_t *collection, 
	const ssi_char_t *name)
{
	MongoCollection c(collection);
	if (!client->get(c))
	{
		ssi_wrn("collection not found '%s'", collection);
		return 0;
	}

	MongoDocument query("name", name);
	MongoDocument d;
	if (!c.findFirst(query, d))
	{
		ssi_wrn("document not found '%s'", name);
	}
		
	return d.getOid();
}

bool CMLAnnotation::SetScheme(Annotation *annotation,
	MongoClient *client,
	const ssi_char_t *scheme, bool externalTraining)
{
	ssi_msg_static(SSI_LOG_LEVEL_BASIC, "load scheme '%s'", client->getName());
	if (ssi_log_level >= SSI_LOG_LEVEL_BASIC)
	{		
		ssi_print_off("scheme    = %s\n", scheme);
	}

	MongoDocument query;

	MongoOID *scheme_id = GetOID(client, CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SCHEMES], scheme);

	if (!scheme_id)
	{
		return false;
	}

	// get scheme

	MongoCollection scheme_collection(CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::SCHEMES]);
	if (!client->get(scheme_collection))
	{
		return false;
	}

	query.setOid(scheme_id);

	MongoDocument scheme_document;
	if (!scheme_collection.findFirst(query, scheme_document))
	{
		ssi_wrn("scheme not found '%s'", scheme_id->toString());
		return false;
	}



	if (!SetScheme(annotation, &scheme_document, externalTraining))
	{
		return false;
	}

	delete scheme_id;

	return true;
}

bool CMLAnnotation::SetScheme(Annotation *annotation,
	MongoDocument *document, bool externalTraining)
{
	annotation->release();

	ssi_char_t *name = document->getString(CMLCons::SCHEME::NAMES[CMLCons::SCHEME::NAME]);
	if (!name)
	{
		ssi_wrn("missing key '%s'", CMLCons::SCHEME::NAMES[CMLCons::SCHEME::NAME])
		return false;
	}

	ssi_char_t *type = document->getString(CMLCons::SCHEME::NAMES[CMLCons::SCHEME::TYPE]);
	if (!type)
	{
		ssi_wrn("missing key '%s'", CMLCons::SCHEME::NAMES[CMLCons::SCHEME::TYPE])
		return false;
	}
	if (ssi_strcmp(type, SSI_SCHEME_NAMES[SSI_SCHEME_TYPE::DISCRETE]))
	{
		std::map<String, ssi_size_t> classes;
		document->getArray(CMLCons::SCHEME::NAMES[CMLCons::SCHEME::LABELS], CMLVisitorAnnotationScheme(), &classes);
		if (!annotation->setDiscreteScheme(name, classes, externalTraining))
		{
			ssi_wrn("missing class names '%s'", CMLCons::SCHEME::NAMES[CMLCons::SCHEME::TYPE])
			return false;
		}
	}
	else if (ssi_strcmp(type, SSI_SCHEME_NAMES[SSI_SCHEME_TYPE::CONTINUOUS]))
	{
		ssi_time_t sr = 0;
		if (!document->getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCORE_RATE], sr))
		{
			ssi_wrn("missing key '%s'", CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCORE_RATE])
			return false;
		}
		double min = 0;
		if (!document->getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCORE_MIN], min))
		{
			ssi_wrn("missing key '%s'", CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCORE_MIN])
			return false;
		}
		double max = 0;
		if (!document->getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCORE_MAX], max))
		{
			ssi_wrn("missing key '%s'", CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::SCORE_MAX])
			return false;
		}

		annotation->setContinuousScheme(name, (ssi_time_t)sr, (ssi_real_t)min, (ssi_real_t)max);
	}
	else if (ssi_strcmp(type, SSI_SCHEME_NAMES[SSI_SCHEME_TYPE::FREE]))
	{
		if (!annotation->setFreeScheme(name))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	delete[] name;
	delete[] type;

	return true;
}

bool CMLAnnotation::AddMongo(Annotation *annotation,
	MongoDocument *document)
{
	if (annotation->getScheme())
	{		
		return document->getArray(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABELS], CMLVisitorAnnotationData(), annotation);		
	}

	return false;
}

}
