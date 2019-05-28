// Openface2Selector.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>, Björn Bittner <bittner@hcm-lab.de>
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

#include "Openface2Selector.h"

namespace ssi {

	char Openface2Selector::ssi_log_name[] = "openfacese";

	Openface2Selector::Openface2Selector(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		for (ssi_size_t i = 0; i < Openface2::FEATURE::NUM; i++)
		{
			_selection[i] = 0;
		}
	}

	Openface2Selector::~Openface2Selector() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void Openface2Selector::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

	}

	void Openface2Selector::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_real_t *src = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *dst = ssi_pcast(ssi_real_t, stream_out.ptr);

		for (ssi_size_t i = 0; i < Openface2::FEATURE::NUM; i++)
		{
			if (_selection[i] != 0)
			{
				*dst++ = *src;
			}
			src++;
		}
	}

	void Openface2Selector::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

	}

	ssi_size_t Openface2Selector::parseOptions()
	{	
		if (_options.success)
		{
			_selection[Openface2::FEATURE::DETECTION_SUCCESS] = 1;
		}

		if (_options.confidence)
		{
			_selection[Openface2::FEATURE::DETECTION_CERTAINTY] = 1;
		}

		if (_options.pose)
		{
			for (int i = Openface2::FEATURE::POSE_X; i <= Openface2::FEATURE::POSE_ROT_Z; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.landmarks)
		{
			for (int i = Openface2::FEATURE::FACIAL_LANDMARK_1_X; i <= Openface2::FEATURE::FACIAL_LANDMARK_68_Y; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.landmarks3d)
		{
			for (int i = Openface2::FEATURE::FACIAL_LANDMARK_3D_1_X; i <= Openface2::FEATURE::FACIAL_LANDMARK_3D_68_Z; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.eye)
		{
			for (int i = Openface2::FEATURE::EYE_LANDMARK_1_X; i <= Openface2::FEATURE::EYE_LANDMARK_56_Y; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.eye3d)
		{
			for (int i = Openface2::FEATURE::EYE_LANDMARK_3D_1_X; i <= Openface2::FEATURE::EYE_LANDMARK_3D_56_Z; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.gaze)
		{
			for (int i = Openface2::FEATURE::GAZE_LEFT_EYE_X; i <= Openface2::FEATURE::GAZE_ANGLE_Y; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.auclass_success)
		{
			_selection[Openface2::FEATURE::AU_REG_DETECTION_SUCCESS] = 1;
		}

		if (_options.auclass)
		{
			for (int i = Openface2::FEATURE::AU01_c; i <= Openface2::FEATURE::AU45_c; i++)
			{
				_selection[i] = 1;
			}
		}

		if (_options.aureg_success)
		{
			_selection[Openface2::FEATURE::AU_CLASS_DETECTION_SUCCESS] = 1;
		}

		if (_options.aureg)
		{
			for (int i = Openface2::FEATURE::AU01_r; i <= Openface2::FEATURE::AU45_r; i++)
			{
				_selection[i] = 1;
			}
		}

		ssi_size_t n = 0;

		for (ssi_size_t i = 0; i < Openface2::FEATURE::NUM; i++)
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
