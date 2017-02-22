// ev_efeatures.h
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

#include "ev_emo.h"

#define MAXIMUM_FEATURES 2000
#define MAX_SEGMENTS     100

int feature_output1(char *filename, mx_real_t *features, int n_features, int binary, char delim, char *class);
int feature_append_output1(char *filename, mx_real_t *features, int n_features, int binary, char delim, char *class);
int feature_outputN(char *filename,mx_real_t **feature_vector,int n_segments,int n_features,int binary,char delim,char *class_info);
void printRawXML(mx_real_t *values, int nvalues, char *type);
