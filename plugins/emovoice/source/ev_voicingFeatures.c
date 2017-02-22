// ev_voicingFeatures.c
// author: Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// created: 
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt
//
// *************************************************************************************************
//
// This file is part of EmoVoice/SSI developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ev_extract.h"
#include "ev_serien.h"

int getVoicingFeatures(mx_real_t *features, mx_real_t *pitch, int nframes) {
    int i, voiced_length=0, unvoiced_length=0, fcount=0;
	fextract_series_t *vseries = series_create(MAX_SERIES);
	fextract_series_t *useries = series_create(MAX_SERIES);

	if (nframes <=0) {
		for (i=0;i<N_VOICING_FEATURES;i++)
			features[fcount++]=-1;
		return fcount;
	}

    for (i=0;i<nframes-1;i++) {
		if (pitch[i]!=0)
		    voiced_length++;
	    else
	    	unvoiced_length++;
		if (i>0 && pitch[i-1] !=0 && pitch[i]==0) {
			series_add(vseries,voiced_length);
			voiced_length=0;
		}
 		if (i>0 && pitch[i-1] ==0 && pitch[i]!=0) {
			series_add(useries,unvoiced_length);
			unvoiced_length=0;
		}
    }
	if (pitch[nframes-1]!=0)
		series_add(vseries,++voiced_length);
	else
		series_add(useries,++unvoiced_length);

	fcount+=getStatistics(features+fcount,vseries->series,vseries->nSeries);
	fcount+=getStatistics(features+fcount,useries->series,useries->nSeries);
	features[fcount++]=1.0*vseries->nSeries/nframes;
	
	series_destroy(vseries);
	series_destroy(useries);
    
    return fcount;
}


