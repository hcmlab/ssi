// ARTKPlusTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/29
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

#include "ARTKPlusTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

void ARTKPlusTools::ClearStruct (marker_s &s) {
	s.visible = false;
	s.id = -1;
	s.center.x = -1;
	s.center.y = -1;
	for (int i = 0; i < 4; i++) {
		s.vertex[i].x = -1;
		s.vertex[i].y = -1;
	}
}

void ARTKPlusTools::PrintStruct (ssi_size_t dim,
	marker_s *ss,
	FILE *file) {

	for (ssi_size_t i = 0; i < dim; i++) {
		if (ss[i].visible) {
			ssi_fprint (file, "marker #%03d\n", ss[i].id);		
			ssi_fprint (file, "  position: %.1f %.1f\n", ss[i].center.x, ss[i].center.y);	
			ssi_fprint (file, "  vertex: %.1f %.1f -> %.1f %.1f -> %.1f %.1f -> %.1f %.1f\n", ss[i].vertex[0].x, ss[i].vertex[0].y, ss[i].vertex[1].x, ss[i].vertex[1].y, ss[i].vertex[2].x, ss[i].vertex[2].y, ss[i].vertex[3].x, ss[i].vertex[3].y);	
		}
	}
}

void ARTKPlusTools::FlipImage (BYTE *dst, const BYTE *src, ssi_video_params_t &params) {

	BYTE *dstPtr = NULL;
	const BYTE *srcPtr = NULL;
	int copyLength = 0;
	int widthStepInBytes = ssi_video_stride (params);
	int biBitCount = params.depthInBitsPerChannel * params.numOfChannels;
	dstPtr = dst + ((params.heightInPixels - 1) * widthStepInBytes);
	srcPtr = src;
	switch(biBitCount)
	{
	case 24:
		copyLength = params.widthInPixels * 3;				
		break;
	case 32:
		copyLength = params.widthInPixels * 4;
		break;
	default:
		ssi_err ("biBitCount = %d not supported", biBitCount);
		break;
	}
	for(int j = 0; j < params.heightInPixels; ++j)
	{
		memcpy(dstPtr, srcPtr, copyLength);
		dstPtr -= widthStepInBytes;
		srcPtr += widthStepInBytes;
	}

}

}
