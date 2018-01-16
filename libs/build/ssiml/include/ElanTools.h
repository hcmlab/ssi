// ElanTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/08/21
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

#ifndef SSI_MODEL_ELANTOOLS_H
#define SSI_MODEL_ELANTOOLS_H

#include "SampleList.h"
#include "ElanTier.h"
#include "ElanDocument.h"
#include "Annotation.h"
#include "ModelTools.h"


namespace ssi {

class ElanTools {

public:

	static void LoadSampleList (SampleList &samples,		
		ssi_stream_t &stream,
		ElanTier &elanTier,
		const ssi_char_t *user_name,
		bool useTierNameAsLabel = false);
	static void LoadSampleList (SampleList &samples,
		ssi_size_t num,
		ssi_stream_t *streams[],
		ElanTier &elanTier,
		const ssi_char_t *user_name,
		bool useTierNameAsLabel = false);

        enum AnnoBoolOp
        {
            AND, OR, XOR, SUB
        };


        //check if time is within elan annotation track (tier)
        static bool ElanTier_within(ElanTier* elanTier, int time);

        //add a track resulting of two original ones merged using boolOp
        //(sub: second - fist)
        static void AddTrack_boolOp(ElanDocument* elanDoc,
                                    char* tier0,
                                    char* tier1,
                                    char* newTier,
                                    AnnoBoolOp boolOp);

        //convert ssi annotation to elan document
        static void Ssi2elan(Annotation anno,
                             ElanDocument* elanDoc);

        static void Ssi2elan(const char* annopath,
                             ElanDocument* elanDoc);

        static void Ssi2elanOld(const char* annopath,
                             ElanDocument* elanDoc);

        //merge second eland docs annotations into first
        static void Merge2first(ElanDocument &first,
                               ElanDocument &second);

        //convert elan annotation to ssi annotation
        static void Elan2ssi(ElanDocument* elanDoc,
                             old::Annotation *ssiAnno);

        static void Elan2ssi(ElanDocument* elanDoc,
                             Annotation *ssiAnno);



protected:

	static ssi_char_t *ssi_log_name;
};

}

#endif
