// ev_energyFeatures.c
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

#define DERIV_LENGTH 5

int getLoudnessFeatures(mx_real_t *features, mx_real_t** energy, int ser){
    /* Energie-Merkmale: */
    int fcount=0, i, j, en_max_ind=-1, n_maxs=0;
	mx_real_t en_max=-MX_REAL_MAX;
	fextract_series_t **derivedSeries;

	if (ser >0) {
		/* Position des Maximums bestimmen */
		for (i=0;i<ser;i++) {
			mx_real_t value = energy[0][i];
			if (value > en_max) {
				en_max=value;
				en_max_ind=i;
			}
		}

		/* Energie-Serien und Statistiken */
		for (j=0;j<ME_DERIV;j++) {
			fcount+=getStatistics(features+fcount,energy[j],ser);
			if (j==0) {
				derivedSeries = getDerivedSeries(energy[j],ser,1);
				n_maxs=derivedSeries[0]->nSeries;
				for (i=0;i<5;i++) {
					fcount+=getStatistics(features+fcount,derivedSeries[i]->series,derivedSeries[i]->nSeries);
					series_destroy(derivedSeries[i]);
				}
				rs_free(derivedSeries);
				
			}
			else {
				derivedSeries = getDerivedSeries(energy[j],ser,0);
				for (i=0;i<2;i++) {
					fcount+=getStatistics(features+fcount,derivedSeries[i]->series,derivedSeries[i]->nSeries);
					series_destroy(derivedSeries[i]);
				}
				rs_free(derivedSeries);
			}
		
		}
		/* zusaetzliche Energie-Features */
		features[fcount++]=1.0*en_max_ind/ser;
		features[fcount++]=1.0*n_maxs/ser;

	}
	else {
		for (;fcount<N_ENERGY_FEATURES;fcount++)
			features[fcount]=-MX_REAL_MAX;
	}

	return fcount;

}






