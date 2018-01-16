// OpenfaceNormalizer.cpp
// author: Jorrit Posor
// created: 04/12/2017
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "OpenfaceNormalizer.h"
#include "Openface.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	OpenfaceNormalizer::OpenfaceNormalizer(const ssi_char_t *file)
		:
		_file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	OpenfaceNormalizer::~OpenfaceNormalizer() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void OpenfaceNormalizer::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

	}

	void OpenfaceNormalizer::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;

		ssi_real_t *in = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *out = ssi_pcast(ssi_real_t, stream_out.ptr);
		
		double normalized_value = 0.0;
		double visualisation_boundary = 0.2;

		/**
		Iterates through stream_in and subtracts X and Y values of pose
		from facial landmarks to make further feature calculation more resistent 
		to movements of the head
		*/
		if (in[Openface::FEATURE::DETECTION_SUCCESS] && in[Openface::FEATURE::DETECTION_CERTAINTY] < visualisation_boundary) {
			// normalize facial landmarks
			for (ssi_size_t i = 0; i < sample_number; i++)
			{
				for (ssi_size_t j = 0; j < sample_dimension; j++)
				{
					int current = i* sample_dimension + j;
					if (25 < j < 162) {
						// X-landmarks are even values
						if (j % 2 == 0) {
							normalized_value = (double)in[current] - ((double)in[Openface::FEATURE::CORRECTED_POSE_WORLD_X] + 320.0);
							out[current] = normalized_value;
						}
						// Y-landmarks are odd values
						else {
							normalized_value = (double)in[current] - ((double)in[Openface::FEATURE::CORRECTED_POSE_WORLD_Y] + 220.0);
							out[current] = normalized_value;
						}
					}
					else {
						out[current] = in[current];
					}
				}
			}
		}
	}

	void OpenfaceNormalizer::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
	}
}