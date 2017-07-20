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

#include "JointTransfTrigger.h"

namespace ssi {

ssi_char_t JointTransfTrigger::ssi_log_name[] = "kinectjtr_";

JointTransfTrigger::JointTransfTrigger (const ssi_char_t *file) 
	: _file (0) 
{
	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

JointTransfTrigger::~JointTransfTrigger () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void JointTransfTrigger::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void JointTransfTrigger::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	OpenNIKinect::SKELETON *ss = ssi_pcast (OpenNIKinect::SKELETON, stream_in.ptr);
	short *outptr = ssi_pcast (short, stream_out.ptr);
	size_t num_users = stream_in.dim / (OpenNIKinect::JOINT_VALUES::NUM * OpenNIKinect::SkeletonJoint::NUM_JOINTS);

	short decision = 0;
	for (ssi_size_t i = 0; i < stream_in.num; i++) {
		for (ssi_size_t j = 0; j < num_users; j++) 
		{
			//check confidence
			ssi_real_t conf = getSkeletonValue (ss[j], _options.joint, (_options.joint_value < 4)? OpenNIKinect::JOINT_VALUES::POS_CONF : OpenNIKinect::JOINT_VALUES::ROT_CONF);
			
			decision = 0;
			if(conf > _options.min_conf)
			{
				ssi_real_t x = getSkeletonValue (ss[j], _options.joint, _options.joint_value);
				
				if(!_options.negative)
					decision = (x >= _options.joint_value_min && x <= _options.joint_value_max);
				else
					decision = (x <= _options.joint_value_min && x <= _options.joint_value_max);
			}
			*outptr++ = decision;
		}
		ss += num_users;
	}
}

void JointTransfTrigger::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

ssi_real_t JointTransfTrigger::getSkeletonValue (OpenNIKinect::SKELETON &s, ssi_size_t joint, ssi_size_t joint_value) 
{
	if(joint < 0 || joint >= OpenNIKinect::SkeletonJoint::NUM_JOINTS)
	{
		ssi_wrn ("index out of range '%d'", joint);
		return 0;
	}
	if(joint_value < 0 || joint_value >= OpenNIKinect::JOINT_VALUES::NUM)
	{
		ssi_wrn ("index out of range '%d'", joint_value);
		return 0;
	}

	return s[joint][joint_value];
}

}

