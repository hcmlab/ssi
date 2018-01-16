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

#include "CMLVisitor.h"
#include "CMLAnnotation.h"
#include "CMLCons.h"
#include "base/StringList.h"
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

void CMLVisitorAnnotationScheme::visit(MongoDocument &document, void *data)
{
	std::map<String, ssi_size_t> *classes = (std::map<String, ssi_size_t> *) data;

	ssi_char_t *name = document.getString(CMLCons::SCHEME::NAMES[CMLCons::SCHEME::LABEL_NAME]);
	int32_t id = 0;
	if (document.getInt32(CMLCons::SCHEME::NAMES[CMLCons::SCHEME::LABEL_ID], id))
	{
		(*classes)[String(name)] = (ssi_size_t)id;
	}

	delete[] name;
}

void CMLVisitorAnnotationData::visit(MongoDocument &document, void *data)
{
	Annotation *anno = (Annotation *)data;

	if (anno->getScheme())
	{
		if (anno->getScheme()->type == SSI_SCHEME_TYPE::DISCRETE)
		{
			int32_t id = 0;
			document.getInt32(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_ID], id);
			double from = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_FROM], from);
			double to = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_TO], to);
			double conf = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_CONF], conf);
			anno->add((ssi_time_t)from, (ssi_time_t)to, (ssi_size_t)id, (ssi_real_t)conf);
		}
		else if (anno->getScheme()->type == SSI_SCHEME_TYPE::CONTINUOUS)
		{
			double score = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_SCORE], score);
			double conf = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_CONF], conf);
			anno->add((ssi_real_t)score, (ssi_real_t)conf);
		}
		else if (anno->getScheme()->type == SSI_SCHEME_TYPE::FREE)
		{
			ssi_char_t *name = document.getString(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_NAME]);
			double from = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_FROM], from);
			double to = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_TO], to);
			double conf = 0;
			document.getDouble(CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::LABEL_CONF], conf);
			anno->add((ssi_time_t)from, (ssi_time_t)to, name, (ssi_real_t)conf);
			delete[] name;
		}
	}
}

}
