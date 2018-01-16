// CMLVisitor.cpp
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

#include "CMLHelper.h"
#include "MongoClient.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

bool CMLHelper::GetCollectionNames(MongoClient *client,
	CMLCons::COLLECTION::List collection,
	StringList &names)
{
	MongoCollection c(CMLCons::COLLECTION::NAMES[collection]);
	if (!client->get(c))
	{
		ssi_wrn("collection not found '%s'", CMLCons::COLLECTION::NAMES[collection]);
		return false;
	}

	MongoDocument query;
	MongoDocuments documents;
	if (c.find(query, documents))
	{
		for (MongoDocuments::iterator it = documents.begin(); it != documents.end(); it++)
		{
			const ssi_char_t *name = it->getString("name");
			if (name)
			{
				names.add(name);
			}
			else
			{
				ssi_wrn("missing key 'name'");
			}
		}
	}

	return true;
}

MongoOID *CMLHelper::GetOIDFromName(MongoClient *client,
	CMLCons::COLLECTION::List collection,
	const ssi_char_t *name)
{
	MongoCollection c(CMLCons::COLLECTION::NAMES[collection]);
	if (!client->get(c))
	{
		ssi_wrn("collection not found '%s'", CMLCons::COLLECTION::NAMES[collection]);
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

}
