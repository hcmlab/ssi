// ITransformer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/06/23
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

#include "OpenNIKinectSelector.h"

namespace ssi {

OpenNIKinectSelector::OpenNIKinectSelector (const ssi_char_t *file) 
	: _file (0),
	_n_selected (0) {

	for (ssi_size_t a = 0; a < OpenNIKinect::SkeletonJoint::NUM_JOINTS; a++) {
		for (ssi_size_t b = 0; b < OpenNIKinect::JOINT_VALUES::NUM; b++) {
			_selector[a][b] = false;
		}
	}

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

OpenNIKinectSelector::~OpenNIKinectSelector () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void OpenNIKinectSelector::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_options.joints[0] != '\0') {
		releaseSelection ();
		parseSelection ();
	}
}

void OpenNIKinectSelector::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	OpenNIKinect::SKELETON *ss = ssi_pcast (OpenNIKinect::SKELETON, stream_in.ptr);
	ssi_real_t *outptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	size_t num_users = stream_in.dim / (OpenNIKinect::JOINT_VALUES::NUM * OpenNIKinect::SkeletonJoint::NUM_JOINTS);

	for (ssi_size_t i = 0; i < stream_in.num; i++) {
		for (ssi_size_t j = 0; j < num_users; j++) 
		{
			ssi_size_t cur_selected = 0;
			for (ssi_size_t a = 0; a < OpenNIKinect::SkeletonJoint::NUM_JOINTS; a++) {
				for (ssi_size_t b = 0; b < OpenNIKinect::JOINT_VALUES::NUM; b++) {
					if((_selector[a][b]))
					{
						*outptr++ = ss[j][a][b];
						++cur_selected;
					}
				}
				if(cur_selected >= _n_selected) //stop searching if we found all needed values
					break;
			}
		}
		ss += num_users;
	}
}

void OpenNIKinectSelector::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void OpenNIKinectSelector::parseSelection () {

	ssi_size_t n_joints = 0;
	ssi_size_t n_values = 0;
	int *joints = ssi_parse_indices (_options.joints, n_joints);
	int *values = ssi_parse_indices (_options.values, n_values);
	for (ssi_size_t i = 0; i < n_joints; i++) {
		for (ssi_size_t j = 0; j < n_values; j++) {
			if(joints[i] < 0 || joints[i] >= OpenNIKinect::SkeletonJoint::NUM_JOINTS)
				ssi_wrn ("index out of range '%d'", joints[i]);
			if(values[j] < 0 || values[j] >= OpenNIKinect::JOINT_VALUES::NUM)
				ssi_wrn ("index out of range '%d'", values[j]);

			select((OpenNIKinect::SkeletonJoint::Joint)joints[i], (OpenNIKinect::JOINT_VALUES::List)values[j]);
		}
	}
	delete[] joints;
	delete[] values;
}

void OpenNIKinectSelector::releaseSelection () 
{
	for (ssi_size_t a = 0; a < OpenNIKinect::SkeletonJoint::NUM_JOINTS; a++) {
		for (ssi_size_t b = 0; b < OpenNIKinect::JOINT_VALUES::NUM; b++) {
			_selector[a][b] = false;
		}
	}
	_n_selected = 0;
}

}
