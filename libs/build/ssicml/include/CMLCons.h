// CMLCons.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/26
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

#ifndef SSI_CML_CONS_H
#define SSI_CML_CONS_H

#include "SSI_Cons.h"

namespace ssi {

	class CMLCons
	{

	public:

		struct COLLECTION
		{
			enum List
			{
				SESSIONS,
				ANNOTATORS,
				ROLES,
				SCHEMES,
				ANNOTATIONS,
				ANNOTATIONS_DATA,
				NUM,
			};

			static const ssi_char_t *NAMES[NUM];
		};		

		struct ANNOTATION
		{
			enum List
			{	
				SESSION_ID,
				ANNOTATOR_ID,
				ROLE_ID,
				SCHEME_ID,
				DATA_ID,
				DATA_BACKUP_ID,
				DATE,
				IS_FINISHED,
				IS_LOCKED,
				LABELS,
				LABEL_NAME,
				LABEL_FROM,
				LABEL_TO,
				LABEL_ID,
				LABEL_CONF,				
				LABEL_SCORE,					
				SCORE_RATE,
				SCORE_MIN,
				SCORE_MAX,
				NUM,
			};

			static const ssi_char_t *NAMES[NUM];
		};

		struct SCHEME
		{				
			enum List
			{
				NAME,
				TYPE,
				LABELS,
				LABEL_ID,
				LABEL_NAME,
				NUM,
			};

			static const ssi_char_t *NAMES[NUM];
		};

	};

}

#endif
