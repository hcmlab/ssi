// JointTransfTrigger.h
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2012/02/12
// Copyright (C) 2007-12 University of Augsburg, Ionut Damian
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

#ifndef SSI_TRIGGER_JOINTTRANSFTRIGGER_H
#define SSI_TRIGGER_JOINTTRANSFTRIGGER_H

#include "base/IFilter.h"
#include "OpenNIKinect.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class JointTransfTrigger : public IFilter {

public:
	class Options : public OptionList {

	public:

		Options ()
			: joint_value (OpenNIKinect::JOINT_VALUES::ROT_Y), joint (OpenNIKinect::SkeletonJoint::TORSO), joint_value_min (-0.1f), joint_value_max (0.1f), min_conf(0.9f), negative (false) {
			
			addOption ("joint", &joint, 1, SSI_SIZE, "index of the joint to watch (head = 0, neck = 1, torso = 2, waist = 3, left_shoulder = 4, left_elbow = 5, left_wrist = 6, left_hand = 7, right_shoulder = 8, right_elbow = 9, right_wrist = 10, right_hand = 11, left_hip = 12, left_knee = 13, left_ankle = 14, left_foot = 15, right_hip = 16, right_knee = 17, right_ankle = 18, right_foot = 19)");
			addOption ("value", &joint_value, 1, SSI_SIZE, "index of the joint value of interest (pos_x = 0, pos_y = 1; pos_z = 2, pos_conf = 3, rot_x = 4, rot_y = 5, rot_z = 6, rot_conf = 7, loc_rot_x = 8, loc_rot_y = 9, loc_rot_z = 10, loc_rot_conf = 11)");
			addOption ("min", &joint_value_min, 1, SSI_REAL, "minimum value the joint is allowed to have");
			addOption ("max", &joint_value_max, 1, SSI_REAL, "maximum value the joint is allowed to have");
			addOption ("minConf", &min_conf, 1, SSI_REAL, "minimum confidence value of the joint for its transformation to be valid");
			addOption ("negative", &negative, 1, SSI_BOOL, "if true, trigger fires when joint is NOT between min and max");	
		}

		ssi_size_t joint_value;
		ssi_size_t joint;
		ssi_real_t joint_value_min;
		ssi_real_t joint_value_max;
		ssi_real_t min_conf;
		bool negative;
	};

public:

	static const ssi_char_t *GetCreateName () { return "JointTransfTrigger"; };
	static IObject *Create (const ssi_char_t *file) { return new JointTransfTrigger (file); };
	~JointTransfTrigger ();
	JointTransfTrigger::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "joint transformation trigger"; };

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
		ssi_size_t num_users = sample_dimension_in / (OpenNIKinect::JOINT_VALUES::NUM * OpenNIKinect::SkeletonJoint::NUM_JOINTS);
		return num_users;
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != sizeof (float)) {
			ssi_err ("invalid struct size");
		}
		return sizeof (short); 
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_FLOAT) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_SHORT;
	}

protected:

	JointTransfTrigger (const ssi_char_t *file = 0);
	JointTransfTrigger::Options _options;
	ssi_char_t *_file;

	static ssi_char_t ssi_log_name[];

	ssi_real_t getSkeletonValue (OpenNIKinect::SKELETON &s, ssi_size_t joint, ssi_size_t joint_value);
};

}

#endif
