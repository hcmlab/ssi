// SkeletonSelector.cpp
// author: Ionut Damian <damian@hcm-lab.de>
// created: 15.11.2012
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

#include "SkeletonSelector.h"

namespace ssi {

SkeletonSelector::SkeletonSelector (const ssi_char_t *file) 
	: _file (0),
	_n_selected (0) {

	for (ssi_size_t a = 0; a < SSI_SKELETON_JOINT::NUM; a++) {
		for (ssi_size_t b = 0; b < SSI_SKELETON_JOINT_VALUE::NUM; b++) {
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

SkeletonSelector::~SkeletonSelector () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void SkeletonSelector::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_options.joints[0] != '\0') {
		releaseSelection ();
		parseSelection ();
	}

	_n_skeletons = stream_in.dim / (SSI_SKELETON_JOINT_VALUE::NUM * SSI_SKELETON_JOINT::NUM);
}

void SkeletonSelector::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	SSI_SKELETON *ss = ssi_pcast (SSI_SKELETON, stream_in.ptr);
	ssi_real_t *outptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	for (ssi_size_t i = 0; i < stream_in.num * _n_skeletons; i++) {		
		ssi_size_t cur_selected = 0;
		for (ssi_size_t a = 0; a < SSI_SKELETON_JOINT::NUM; a++) {
			for (ssi_size_t b = 0; b < SSI_SKELETON_JOINT_VALUE::NUM; b++) {
				if((_selector[a][b]))
				{
					*outptr++ = (*ss)[a][b];
					++cur_selected;
				}
			}
			if(cur_selected >= _n_selected) //stop searching if we found all needed values
				break;
		}
		ss++;
	}
}

void SkeletonSelector::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void SkeletonSelector::parseSelection () {

	ssi_size_t n_joints = 0;
	ssi_size_t n_values = 0;
	int *joints = ssi_parse_indices (_options.joints, n_joints);
	int *values = ssi_parse_indices (_options.values, n_values);
	for (ssi_size_t i = 0; i < n_joints; i++) {
		for (ssi_size_t j = 0; j < n_values; j++) {
			if(joints[i] < 0 || joints[i] > SSI_SKELETON_JOINT::NUM)
				ssi_wrn ("index out of range '%d'", joints[i]);
			if(values[j] < 0 || values[j] > SSI_SKELETON_JOINT_VALUE::NUM)
				ssi_wrn ("index out of range '%d'", values[j]);

			select((SSI_SKELETON_JOINT::List)joints[i], (SSI_SKELETON_JOINT_VALUE::List)values[j]);
		}
	}
	delete[] joints;
	delete[] values;
}

void SkeletonSelector::releaseSelection () 
{
	for (ssi_size_t a = 0; a < SSI_SKELETON_JOINT::NUM; a++) {
		for (ssi_size_t b = 0; b < SSI_SKELETON_JOINT_VALUE::NUM; b++) {
			_selector[a][b] = false;
		}
	}
	_n_selected = 0;
}

}
