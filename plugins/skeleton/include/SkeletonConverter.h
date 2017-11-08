// SkeletonConverter.h
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

#pragma once

#ifndef SSI_SKELETON_CONVERTER_H
#define SSI_SKELETON_CONVERTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

#include "SSI_SkeletonCons.h"

namespace ssi {

class SkeletonConverter : public IFilter {


public:

class Options : public OptionList {

	public:

		Options () : n_skeletons (1), n_faces(1), skeleton_type(-1), face_type(1) {

			addOption ("numskel", &n_skeletons, 1, SSI_SIZE, "number of skeletons tracked");
			//addOption ("numface", &n_faces, 1, SSI_SIZE, "number of faces tracked");
			addOption ("bodytype", &skeleton_type, 1, SSI_INT, "body skeleton type (SSI = 0, MICROSOFT_KINECT = 1, OPENNI_KINECT = 2, XSENS_MVN = 3, MICROSOFT_KINECT2 = 4)");
			//addOption ("facetype", &face_type, 1, SSI_INT, "face type (SSI = 0, MICROSOFT_KINECT = 1)");
		};

		ssi_size_t n_skeletons;
		ssi_size_t n_faces;
		int skeleton_type;
		int face_type;
	};

public:
	static const ssi_char_t *GetCreateName () { return "SkeletonConverter"; };
	static IObject *Create (const ssi_char_t *file) { return new SkeletonConverter (file); };
	~SkeletonConverter ();
	SkeletonConverter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "converts different types of skeleton streams to the ssi skeleton"; };

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
		parseMetaData();
		return ssi_cast (ssi_size_t, _skeleton_meta_in.num * SSI_SKELETON_JOINT::NUM * SSI_SKELETON_JOINT_VALUE::NUM); 
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != SSI_SKELETON_VALUE_BYTES) {
			ssi_err ("invalid struct size");
		}
		return SSI_SKELETON_VALUE_BYTES; 
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_SKELETON_VALUE_SSITYPE) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_SKELETON_VALUE_SSITYPE;
	}
	
	void setMetaData (ssi_size_t size, const void *meta) {
		_meta_received++;		
		memcpy (_meta_in_ptr, meta, size);
		_meta_in_ptr += size; //if there are multiple meta's, we will concatenate them
	}

	const void *getMetaData (ssi_size_t &size) { 		
		_meta_out = ssi_skeleton_meta(SSI_SKELETON_TYPE::SSI, _skeleton_meta_in.num);
		size = sizeof(_meta_out);
		return &_meta_out;
	}

protected:

	SkeletonConverter (const ssi_char_t *file = 0);
	SkeletonConverter::Options _options;
	ssi_char_t *_file;
	
	SSI_SKELETON_META _skeleton_meta_in;
	bool _faceTracking;
	SSI_FACE_META _face_meta_in;

	SSI_SKELETON_META _meta_out;
	ssi_byte_t _meta_in[256];
	ssi_byte_t *_meta_in_ptr;
	ssi_size_t _meta_received;

	void parseMetaData () {
		if(_meta_received > 0)
		{
			ssi_byte_t *ptr = _meta_in;
			SSI_SKELETON_META *m = ssi_pcast (SSI_SKELETON_META, ptr);
			_skeleton_meta_in = *m;			
			if(_meta_received == 2)
			{
				ptr += sizeof(SSI_SKELETON_META);
				SSI_FACE_META *fm = ssi_pcast(SSI_FACE_META, ptr);
				_face_meta_in = *fm;
			}
		}
		else
		{	
			if(_options.skeleton_type < 0) 
			{
				ssi_wrn("Skeleton type not set. Assuming MICROSOFT_KINECT.");
				_skeleton_meta_in.type = SSI_SKELETON_TYPE::MICROSOFT_KINECT;
			}
			else
			{
				_skeleton_meta_in.type = ssi_cast (SSI_SKELETON_TYPE::List, _options.skeleton_type);
			}

			_skeleton_meta_in.num = _options.n_skeletons;
			_face_meta_in.type = ssi_cast (SSI_FACE_TYPE::List, _options.face_type);
			_face_meta_in.num = _options.n_faces;
		}
	}

	void convertMicrosoftKinect(void* fromPtr, SSI_SKELETON &to);
	void convertMicrosoftKinect2(void* fromPtr, SSI_SKELETON &to);
	void convertMicrosoftKinectOld(void* fromPtr, SSI_SKELETON &to);
	void convertOpenNIKinect(void* fromPtr, SSI_SKELETON &to);
	void convertXsensMVN(void* fromPtr, SSI_SKELETON &to);

	void convertFaceMicrosoftKinect(void* facePointsPtr, void* headPosePtr, SSI_SKELETON &to);
	void convertFaceMicrosoftKinect2(void* facePointsPtr, void* headPosePtr, SSI_SKELETON &to);

	/*
	* This is all we need from MicrosoftKinect2.h
	* No need to include the whole SDK 2
	*/
	struct KINECT2_JOINT
	{
		enum _JointType
		{
			JointType_SpineBase = 0,
			JointType_SpineMid = 1,
			JointType_Neck = 2,
			JointType_Head = 3,
			JointType_ShoulderLeft = 4,
			JointType_ElbowLeft = 5,
			JointType_WristLeft = 6,
			JointType_HandLeft = 7,
			JointType_ShoulderRight = 8,
			JointType_ElbowRight = 9,
			JointType_WristRight = 10,
			JointType_HandRight = 11,
			JointType_HipLeft = 12,
			JointType_KneeLeft = 13,
			JointType_AnkleLeft = 14,
			JointType_FootLeft = 15,
			JointType_HipRight = 16,
			JointType_KneeRight = 17,
			JointType_AnkleRight = 18,
			JointType_FootRight = 19,
			JointType_SpineShoulder = 20,
			JointType_HandTipLeft = 21,
			JointType_ThumbLeft = 22,
			JointType_HandTipRight = 23,
			JointType_ThumbRight = 24,
			JointType_Count = (JointType_ThumbRight + 1)
		};
	};

	struct KINECT2_JOINT_VALUE
	{
		enum List
		{
			POS_X = 0,
			POS_Y,
			POS_Z,
			POS_CONF,

			ROT_W,
			ROT_X,
			ROT_Y,
			ROT_Z,
			ROT_CONF,

			RELROT_W,
			RELROT_X,
			RELROT_Y,
			RELROT_Z,
			RELROT_CONF,

			NUM
		};
	};

	

	typedef float KINECT2_SKELETON[KINECT2_JOINT::JointType_Count][KINECT2_JOINT_VALUE::NUM];

	/*
	 * Legacy
	 */
	struct MSOLD_JOINT
	{
		enum List
		{
		
			HIP_CENTER	= 0,
			SPINE,
			SHOULDER_CENTER,
			HEAD,
			SHOULDER_LEFT,
			ELBOW_LEFT,
			WRIST_LEFT,
			HAND_LEFT,
			SHOULDER_RIGHT,
			ELBOW_RIGHT,
			WRIST_RIGHT,
			HAND_RIGHT,
			HIP_LEFT,
			KNEE_LEFT,
			ANKLE_LEFT,
			FOOT_LEFT,
			HIP_RIGHT,
			KNEE_RIGHT,
			ANKLE_RIGHT,
			FOOT_RIGHT,

			NUM
		};
	};

	struct MSOLD_JOINT_VALUE
	{
		enum List
		{
			POS_W = 0,
			POS_X,
			POS_Y,
			POS_Z,

			ROT_W,
			ROT_X,
			ROT_Y,
			ROT_Z,

			RELROT_W,
			RELROT_X,
			RELROT_Y,
			RELROT_Z,

			NUM
		};
	};

	typedef float MSOLD[MSOLD_JOINT::NUM][MSOLD_JOINT_VALUE::NUM];
};

}

#endif
