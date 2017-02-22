// OpenNIKinectSelector.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/08
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

#pragma once

#ifndef SSI_OPENNIKINECT_SELECTOR_H
#define SSI_OPENNIKINECT_SELECTOR_H

#include "base/IFilter.h"
#include "OpenNIKinect.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OpenNIKinectSelector : public IFilter {


public:

class Options : public OptionList {

	public:

		Options () {

			joints[0] = '\0';
			values[0] = '\0';
			addOption ("joints", joints, SSI_MAX_CHAR, SSI_CHAR, "indices of selected joints (head = 0, neck = 1, torso = 2, waist = 3, left_shoulder = 4, left_elbow = 5, left_wrist = 6, left_hand = 7, right_shoulder = 8, right_elbow = 9, right_wrist = 10, right_hand = 11, left_hip = 12, left_knee = 13, left_ankle = 14, left_foot = 15, right_hip = 16, right_knee = 17, right_ankle = 18, right_foot = 19)");
			addOption ("values", values, SSI_MAX_CHAR, SSI_CHAR, "indices of selected joint values (pos_x = 0, pos_y = 1; pos_z = 2, pos_conf = 3, rot_x = 4, rot_y = 5, rot_z = 6, rot_conf = 7, locrot_x = 8, locrot_y = 9, locrot_z = 10, locrot_conf = 11)");
		};

		void setJoints (ssi_size_t index) {
			setJoints (1, &index);
		}

		void setValues (ssi_size_t index) {
			setValues (1, &index);
		}

		void setJoints (ssi_size_t n_inds, ssi_size_t *inds) {
			joints[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%u", inds[0]);
				strcat (joints, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%u", inds[i]);
					strcat (joints, s);
				}
			}
		}

		void setValues (ssi_size_t n_inds, ssi_size_t *inds) {
			values[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%u", inds[0]);
				strcat (values, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%u", inds[i]);
					strcat (values, s);
				}
			}
		}

		ssi_char_t joints[SSI_MAX_CHAR];
		ssi_char_t values[SSI_MAX_CHAR];
	};

public:
	static const ssi_char_t *GetCreateName () { return "OpenNIKinectSelector"; };
	static IObject *Create (const ssi_char_t *file) { return new OpenNIKinectSelector (file); };
	~OpenNIKinectSelector ();
	OpenNIKinectSelector::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "selects properties from a kinect skeleton stream"; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) { 
		if (_options.joints[0] != '\0') {
			releaseSelection ();
			parseSelection ();
		}
		size_t num_users = sample_dimension_in / (OpenNIKinect::JOINT_VALUES::NUM * OpenNIKinect::SkeletonJoint::NUM_JOINTS);
		return ssi_cast (ssi_size_t, num_users * _n_selected); 
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != sizeof (float)) {
			ssi_err ("invalid struct size");
		}
		return sizeof (ssi_real_t); 
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_FLOAT) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	void select(OpenNIKinect::SkeletonJoint::Joint j, OpenNIKinect::JOINT_VALUES::List v)
	{
		_selector[j][v] = true;
		++_n_selected;
	};

protected:

	OpenNIKinectSelector (const ssi_char_t *file = 0);
	OpenNIKinectSelector::Options _options;
	ssi_char_t *_file;

	bool _selector[OpenNIKinect::SkeletonJoint::NUM_JOINTS][OpenNIKinect::JOINT_VALUES::NUM];
	ssi_size_t _n_selected;

	void parseSelection ();
	void releaseSelection ();
};

}

#endif
