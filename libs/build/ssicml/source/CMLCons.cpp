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

#include "CMLCons.h"

namespace ssi
{
	const ssi_char_t *CMLCons::COLLECTION::NAMES[CMLCons::COLLECTION::NUM] =
	{
		"Sessions",
		"Annotators",
		"Roles",
		"Schemes",
		"Annotations",
		"AnnotationData",
	};

	const ssi_char_t *CMLCons::ANNOTATION::NAMES[CMLCons::ANNOTATION::NUM] =
	{
		"session_id",
		"annotator_id",
		"role_id",
		"scheme_id",
		"data_id",
		"data_backup_id",
		"date",
		"isFinished",
		"isLocked",
		"labels",
		"name",
		"from",
		"to",
		"id",
		"conf",		
		"score",		
		"sr",
		"min",
		"max",
	};

	const ssi_char_t *CMLCons::SCHEME::NAMES[CMLCons::SCHEME::NUM] =
	{
		"name",
		"type",
		"labels",
		"id",
		"name",
	};
}