// ev_serien.h
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

#include "ev_real.h"

#ifndef SERIEN_H_
#define SERIEN_H_

#define MAX_SERIES      256
#define STATS           9        // mean, maximum, minimum, difference between min & max, variance, median, 
                                 // 1st quartile, 3rd quartile, interquartile range

typedef struct {
  mx_real_t *series;
  int maxSeries;
  int nSeries;
} fextract_series_t;

fextract_series_t *series_create(int max);
void series_add(fextract_series_t *data_series, mx_real_t element);
void series_reset(fextract_series_t *data_series);
void series_destroy(fextract_series_t *data_series);

int getStatistics(mx_real_t* features, mx_real_t* list,int count);

fextract_series_t **getDerivedSeries(mx_real_t *data, int ser, int extended);


#endif /*SERIEN_H_*/
