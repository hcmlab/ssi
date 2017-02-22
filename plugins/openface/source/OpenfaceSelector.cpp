// OpenfaceSelector.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 14/6/2016
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

#include "OpenfaceSelector.h"

namespace ssi {

	char OpenfaceSelector::ssi_log_name[] = "openfacese";

	OpenfaceSelector::OpenfaceSelector(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}

		for (ssi_size_t i = 0; i < Openface::FEATURE::NUM; i++)
		{
			_selection[i] = 0;
		}
	}

	OpenfaceSelector::~OpenfaceSelector() {

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}
	}

	void OpenfaceSelector::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

	}

	void OpenfaceSelector::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_real_t *src = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *dst = ssi_pcast(ssi_real_t, stream_out.ptr);

		for (ssi_size_t i = 0; i < Openface::FEATURE::NUM; i++)
		{
			if (_selection[i] != 0)
			{
				*dst++ = *src;
			}
			src++;
		}
	}

	void OpenfaceSelector::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

	}

	ssi_size_t OpenfaceSelector::parseOptions()
	{	
		if (_options.success)
		{
			_selection[Openface::FEATURE::DETECTION_SUCCESS] = 1;
		}

		if (_options.confidence)
		{
			_selection[Openface::FEATURE::DETECTION_CERTAINTY] = 1;
		}

		if (_options.poscam)
		{
			for (int i = Openface::FEATURE::POSE_CAMERA_X; i <= Openface::FEATURE::POSE_CAMERA_ROT_Z; i++)
			{
				_selection[i] = 1;
			}
		}
		
		if (_options.posworld)
		{
			for (int i = Openface::FEATURE::POSE_WORLD_X; i <= Openface::FEATURE::POSE_WORLD_ROT_Z; i++)
			{
				_selection[i] = 1;
			}
		}
			
		if (_options.corrposcam)
		{
			for (int i = Openface::FEATURE::CORRECTED_POSE_CAMERA_X; i <= Openface::FEATURE::CORRECTED_POSE_CAMERA_ROT_Z; i++)
			{
				_selection[i] = 1;
			}
		}
		
		if (_options.corrposworld)
		{
			for (int i = Openface::FEATURE::CORRECTED_POSE_WORLD_X; i <= Openface::FEATURE::CORRECTED_POSE_WORLD_ROT_Z; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.landmarks)
		{
			for (int i = Openface::FEATURE::FACIAL_LANDMARK_1_X; i <= Openface::FEATURE::FACIAL_LANDMARK_68_Y; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.gaze)
		{
			for (int i = Openface::FEATURE::GAZE_LEFT_EYE_X; i <= Openface::FEATURE::GAZE_RIGHT_EYE_Z; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.pupil)
		{
			for (int i = Openface::FEATURE::PUPIL_LEFT_EYE_X; i <= Openface::FEATURE::PUPIL_RIGHT_EYE_Z; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.auclass_success)
		{
			_selection[Openface::FEATURE::AU_REG_DETECTION_SUCCESS] = 1;
		}

		if (_options.auclass)
		{
			for (int i = Openface::FEATURE::AU04_c; i <= Openface::FEATURE::AU45_c; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.aureg_success)
		{
			_selection[Openface::FEATURE::AU_CLASS_DETECTION_SUCCESS] = 1;
		}

		if (_options.aureg)
		{
			for (int i = Openface::FEATURE::AU01_r; i <= Openface::FEATURE::AU26_r; i++)
			{
				_selection[i] = 1;
			}
		}

		ssi_size_t n = 0;

		for (ssi_size_t i = 0; i < Openface::FEATURE::NUM; i++)
		{
			if (_selection[i] != 0)
			{
				n++;
			}
		}

		if (n == 0)
		{
			ssi_err("empty selection");
		}

		return n;
	}
}
