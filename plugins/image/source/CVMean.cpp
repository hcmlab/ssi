// CVMean.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/07/19
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

#include "CVMean.h"
#include "ssiocv.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *CVMean::ssi_log_name = "ocvmean___";	

CVMean::CVMean () 
{
}

CVMean::~CVMean ()
{
}

void CVMean::transform(ssi_time_t frame_rate,
	IplImage *image_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) 
{
	ssi_size_t dim = stream_out.dim;
	ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream_out.ptr);

	cv::Mat image = cv::cvarrToMat(image_in, false);

	std::vector<cv::Mat> channels;	
	cv::split(image, channels);

	for (ssi_size_t i = 0; i < dim; i++)
	{
		cv::Scalar m = mean(channels[i]);
		*ptr++ = (ssi_real_t) (_options.scale ? m.val[0] / 255.0 : m.val[0]);		
	}
}

void CVMean::setFormat (ssi_video_params_t format) 
{
	ICVFeature::setFormatIn (format);	
}

ssi_size_t CVMean::getSampleDimensionOut(ssi_size_t sample_dimension_in) 
{
	return (ssi_size_t) _format_in.numOfChannels;
}

ssi_size_t CVMean::getSampleBytesOut(ssi_size_t sample_bytes_in)
{
	return sizeof(ssi_real_t);
}

ssi_type_t CVMean::getSampleTypeOut(ssi_type_t sample_type_in)
{
	return SSI_REAL;
}

}
