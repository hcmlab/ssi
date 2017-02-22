// ev_mfccFeatures.c
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

#include "ev_messages.h"
#include "ev_memory.h"

#include "ev_extract.h"
#include "ev_serien.h"


int getMFCCFeatures(mx_real_t *features, mx_real_t** mfcc, int ser) {
	int fcount=0, i, j, k;
	
	if (ser) {
	    /* MFCC-Merkmale: */
	    mx_real_t *mean_mfcc=(mx_real_t*) rs_malloc(ser*sizeof(mx_real_t),"mean mfcc series");
		fextract_series_t **derivedSeries;

		for (k=0;k<ME_DERIV;k++) {
			for (j=0;j<ser;j++) {
				mean_mfcc[j]=0;
				for (i=0;i<MFCCS;i++)
					mean_mfcc[j]+=mfcc[k*MFCCS+i][j];
				mean_mfcc[j]/=MFCCS;
			}
			fcount+=getStatistics(features+fcount,mean_mfcc,ser); 	
			derivedSeries = getDerivedSeries(mean_mfcc,ser,0);
			for (j=0;j<2;j++) {
				fcount+=getStatistics(features+fcount,derivedSeries[j]->series,derivedSeries[j]->nSeries);
				series_destroy(derivedSeries[j]);
			}
			rs_free(derivedSeries);
			
			for (i=0;i<MFCCS;i++) {
				fcount+=getStatistics(features+fcount,mfcc[k*MFCCS+i],ser);
				derivedSeries = getDerivedSeries(mfcc[k*MFCCS+i],ser,0);
				for (j=0;j<2;j++) {
					fcount+=getStatistics(features+fcount,derivedSeries[j]->series,derivedSeries[j]->nSeries);
					series_destroy(derivedSeries[j]);
				}
				rs_free(derivedSeries);
			}
		}
		rs_free(mean_mfcc);
  	} 
	else {
    	for (;fcount < N_MFCC_FEATURES;fcount++)  // N_MFCC_FEATURES = (12+1)*3*3*9
			features[fcount]=-1;
	}
	
  return fcount;
}
