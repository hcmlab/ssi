// ev_serien.c
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

#include "ev_memory.h"
#include "ev_messages.h"

#include "ev_serien.h"

static int _cmp_mx_real(const void *_big, const void *_small);

fextract_series_t *series_create(int max){
  fextract_series_t *data_series= (fextract_series_t *) rs_malloc(sizeof(fextract_series_t),"data series");
  data_series->maxSeries=max;
  data_series->nSeries=0;
  data_series->series= (mx_real_t *) rs_malloc(max*sizeof(mx_real_t),"data of series");
  return data_series;
}

void series_add(fextract_series_t *data_series, mx_real_t element) {
  if (data_series->maxSeries <= data_series->nSeries) {
    data_series->maxSeries+=50;
    data_series->series = (mx_real_t *) rs_realloc(data_series->series,data_series->maxSeries*sizeof(mx_real_t),"data of series");
  }
  data_series->series[data_series->nSeries++] = element;
}

void series_reset(fextract_series_t *data_series) {
  data_series->nSeries=0;
}

void series_destroy(fextract_series_t *data_series) {
  rs_free(data_series->series);
  rs_free(data_series);
}


fextract_series_t **getDerivedSeries(mx_real_t *data, int ser, int extended) {
	int i,lastExtreme=-1;
	int series_size =3*extended+2;
	fextract_series_t **serien = (fextract_series_t **) rs_malloc(series_size*sizeof(fextract_series_t *),"derived energy series");
		
	/* Energie */
    if (ser>0) {
   	
   		/* Initialisierung */
 		for (i=0;i<series_size;i++)
		    serien[i]=series_create(MAX_SERIES);
	
		for (i=1;i<ser-1;i++) {
				int isExtreme=0;
				//Maxima
			    if (data[i-1]<data[i] && data[i]>data[i+1]) {
					series_add(serien[0], data[i]); 
					isExtreme=1;
			    }
				else
					//Minima
				    if (data[i-1]>data[i] && data[i]<data[i+1]) {
						series_add(serien[1], data[i]);
						isExtreme=1;
					}
				if (isExtreme && extended) {
					if (lastExtreme != -1) {
						//distance
						series_add(serien[2],fabs(i-lastExtreme));
						//difference
						series_add(serien[3],fabs(data[i]-data[lastExtreme]));
						//slope
						series_add(serien[4],fabs((data[i]-data[lastExtreme])/(i-lastExtreme)));
					}
					lastExtreme=i;
				}
		}
    }
    return serien;
}


/* compute mean, maximum, minimum, difference between min & max, variance, median, 
   1st quartile, 3rd quartile, interquartile range of a list of real values */
int getStatistics(mx_real_t* features, mx_real_t* list,int count) {
  int i;
  mx_real_t *_list;
  mx_real_t mean=0, max=-MX_REAL_MAX, min=MX_REAL_MAX, variance=0;
  mx_real_t median, quartile_1, quartile_3;

  if (count==0) {
    for (i=0;i<STATS;i++)
      features[i]=0.0;
    return STATS;
  }


  if (count==1) {
    features[0]=features[1]=features[2]=features[5]=features[6]=features[7]=list[0];
    features[3]=features[4]=features[8]=0.0;
    return STATS;
  }

  _list= (mx_real_t *) rs_malloc(count*sizeof(mx_real_t),"statistics list");

  for (i=0;i<count;i++) {
    _list[i]=list[i];
    mean+=_list[i];
    variance+=mx_sqr(_list[i]);
    if (_list[i] <min)
      min=_list[i];
    if (_list[i] >max)
      max=_list[i];
  }
  
  mean /= count;
  variance = variance/count - mx_sqr(mean);


  qsort(_list,count,sizeof(mx_real_t),_cmp_mx_real);

  median = _list[count/2];
  quartile_1 = _list[count/4];
  quartile_3 = _list[3*count/4];

  features[0] = mean;
  features[1] = max;
  features[2] = min;
  features[3] = max - min;
  features[4] = variance;
  features[5] = median;
  features[6] = quartile_1;
  features[7] = quartile_3;
  features[8] = fabs(quartile_3 - quartile_1);

  rs_free(_list);
  return STATS;
}

static int _cmp_mx_real(const void *_big, const void *_small) {
  mx_real_t *small = (mx_real_t *) _small, *big =(mx_real_t *) _big;
  
   if (!*small && !*big)
    return(0);
  else
    if (!*small)
      return(-1);
    else
      if (!*big)
      return(1);

   if (*big == *small)
     return (0);
   else
     if (*big > *small)
       return(1);
     else 
       return (-1);
}
