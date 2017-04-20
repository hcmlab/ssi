// GSRClean.cpp
// author: Frank Gaibler <gaibler@hcm-lab.de>
// created: 2017/01/17
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

#include "GSRClean.h"
char GSRClean::ssi_log_name[] = "GSRCleanFeatures";

/**
* constructor
*/
GSRClean::GSRClean(const ssi_char_t *file)
{
}

/**
* destructor
*/
GSRClean::~GSRClean()
{
}

/**
* init global variables
*/
void GSRClean::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[])
{
	init = true;
	ssi_msg(SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

/**
* cleanup data
*/
void GSRClean::transform(ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[])
{
	ssi_real_t* ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	ssi_real_t* ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	if (init) {
		init = false;
		valLast = *ptr_in;
		valLastOut = *ptr_in;
	}
	//ignore values wich are larger than the threshold or zero
	for (int i = 0; i < stream_in.num; i++) {
		//filter zeros
		ssi_real_t valIn = *(ptr_in + i);
		if (valIn == 0.0) {
			valIn = valLast;
		}
		//difference of real values
		ssi_real_t difVal = valIn - valLast;
		//threshold correction
		if (difVal < -_options.threshold || difVal > _options.threshold) {
			difVal = 0;
		}
		//new value
		ssi_real_t newVal = valLastOut + difVal;
		*(ptr_out + i) = newVal;
		//save old values
		valLastOut = newVal;
		valLast = valIn;
	}
}

/**
* flush
*/
void GSRClean::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[])
{
	ssi_msg(SSI_LOG_LEVEL_BASIC, "flush()..ok");
}
