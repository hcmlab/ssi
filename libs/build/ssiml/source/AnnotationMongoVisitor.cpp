// AnnotationMongoVisitor.cpp
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

#include "AnnotationMongoVisitor.h"
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

void AnnotationSchemeVisitor::visit(MongoDocument &document, void *data)
{
	std::map<String, ssi_size_t> *classes = (std::map<String, ssi_size_t> *) data;

	ssi_char_t *name = document.string(Annotation::KEY_NAMES[Annotation::KEY::SCHEME_LABEL_NAME]);
	ssi_size_t id = (ssi_size_t)document.int32(Annotation::KEY_NAMES[Annotation::KEY::SCHEME_LABEL_ID]);
	(*classes)[String(name)] = id;

	delete[] name;
}

void AnnotationDataVisitor::visit(MongoDocument &document, void *data)
{
	Annotation *anno = (Annotation *) data;

	if (anno->getScheme())
	{
		if (anno->getScheme()->type == IAnnotation::TYPE::DISCRETE)
		{
			ssi_size_t id = (ssi_size_t)document.int32(Annotation::KEY_NAMES[Annotation::KEY::ANNOTATION_SEGMENT_ID]);
			ssi_time_t from = (ssi_time_t)document.real(Annotation::KEY_NAMES[Annotation::KEY::ANNOTATION_SEGMENT_FROM]);
			ssi_time_t to = (ssi_time_t)document.real(Annotation::KEY_NAMES[Annotation::KEY::ANNOTATION_SEGMENT_TO]);
			ssi_real_t conf = (ssi_real_t)document.real(Annotation::KEY_NAMES[Annotation::KEY::ANNOTATION_SEGMENT_CONF]);
			anno->add(from, to, id, conf);
		}
		else
		{
			ssi_real_t score = (ssi_real_t)document.real(Annotation::KEY_NAMES[Annotation::KEY::ANNOTATION_FRAME_SCORE]);
			ssi_real_t conf = (ssi_real_t)document.real(Annotation::KEY_NAMES[Annotation::KEY::ANNOTATION_FRAME_CONF]);
			anno->add(score, conf);
		}
	}
}

}
