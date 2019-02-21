// OpenposeConverter.cpp
// author: Felix Kickstein felix.kickstein@student.uni-augsburg.de
// created: 14/05/2018
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

#include "OpenposeConverter.h"
#include "OpenposeModelInfos.h"



namespace ssi {

	OpenposeConverter::OpenposeConverter(const ssi_char_t *file)
		: _file(0), _meta_received(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		_meta_in_ptr = _meta_in;
	}

	OpenposeConverter::~OpenposeConverter() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void OpenposeConverter::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		parseMetaData();
	}

	void OpenposeConverter::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		//ssi_print("stream_meta_in num: %d stream in num: %d  \n", _skeleton_meta_in.num , stream_in.num);
		//_skeleton_meta_in.num = 1;

		SSI_SKELETON *dstptr = ssi_pcast(SSI_SKELETON, stream_out.ptr);
		for (ssi_size_t k = SSI_SKELETON_INVALID_JOINT_VALUE; k < stream_in.num * _skeleton_meta_in.num; ++k) {
			for (ssi_size_t i = SSI_SKELETON_INVALID_JOINT_VALUE; i < SSI_SKELETON_JOINT::NUM; i++) {
				for (ssi_size_t j = SSI_SKELETON_INVALID_JOINT_VALUE; j < SSI_SKELETON_JOINT_VALUE::NUM; ++j) {
					(*dstptr)[i][j] = SSI_SKELETON_INVALID_JOINT_VALUE;
				}
			}
		}
		dstptr = ssi_pcast(SSI_SKELETON, stream_out.ptr);

		/*
		* Convert skeleton
		*/

		OpenposeModelInfos::SKELETON_BODY *srcptr = ssi_pcast(OpenposeModelInfos::SKELETON_BODY, stream_in.ptr);

		for (ssi_size_t k = SSI_SKELETON_INVALID_JOINT_VALUE; k < stream_in.num * _skeleton_meta_in.num; k++) {
			convertOpenpose(srcptr++, *dstptr++, k);
		}


	}


	void OpenposeConverter::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
	}

	/*
		Generally openpose provides only x,y and conf values.
		The other values (z, rotation and rel_rotation) of the ssi_skeleton will be set to 0
	*/
	void OpenposeConverter::convertOpenpose(void* fromPtr, SSI_SKELETON &to, int k) {

		//Openpose::SKELETON& from = *(ssi_pcast(Openpose::SKELETON, fromPtr));
		OpenposeModelInfos::SKELETON_BODY& from = *(ssi_pcast(OpenposeModelInfos::SKELETON_BODY, fromPtr));
		/*
		* HEAD
		* there's no head position in the coco model, the idea is to calculate head as  the mid of the eyes (LEYE and REYE)
		*/

		//ssi_print("person : %d head x : %f  \n", k, from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X]);
		to[SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::HEAD][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];

		/*
		* NECK
		*/
		to[SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* WAIST

		*/


		to[SSI_SKELETON_JOINT::WAIST][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::MIDHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::WAIST][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::MIDHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::WAIST][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::MIDHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];

		/*
		* TORSO
		  the torso calculation is inspired by the caluclation from the kinect to ssi skeleton
		*/

		to[SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X];
		if (from[OpenposeModelInfos::SKELETON_JOINT_BODY::MIDHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] == 0.0f)
			to[SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		else
			to[SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_Y] = (from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] - (from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] - from[OpenposeModelInfos::SKELETON_JOINT_BODY::MIDHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y]) / 3)* -1.0f;;
		to[SSI_SKELETON_JOINT::TORSO][SSI_SKELETON_JOINT_VALUE::POS_CONF] = (from[OpenposeModelInfos::SKELETON_JOINT_BODY::NECK][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF] + from[OpenposeModelInfos::SKELETON_JOINT_BODY::MIDHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF]) / 2;

		/*
		* LEFT_SHOULDER
		*/
		to[SSI_SKELETON_JOINT::LEFT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LSHOULDER][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LSHOULDER][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LSHOULDER][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* LEFT_ELBOW
		*/
		to[SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LELBOW][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LELBOW][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LELBOW][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];

		/*
		* LEFT_WRIST
		*/
		to[SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* LEFT_HAND
		* no information of the left hand
		*/
		to[SSI_SKELETON_JOINT::LEFT_HAND][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_HAND][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_HAND][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* RIGHT_SHOULDER
		*/
		to[SSI_SKELETON_JOINT::RIGHT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RSHOULDER][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RSHOULDER][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RSHOULDER][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];

		/*
		* RIGHT_ELBOW
		*/
		to[SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RELBOW][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RELBOW][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RELBOW][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* RIGHT_WRIST
		*/
		to[SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* RIGHT_HAND
		  no informaiton of the right hand, for now the wrist is taken
		*/
		to[SSI_SKELETON_JOINT::RIGHT_HAND][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_HAND][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_HAND][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RWRIST][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];

		/*
		* LEFT_HIP
		*/
		to[SSI_SKELETON_JOINT::LEFT_HIP][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_HIP][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* LEFT_KNEE
		*/
		to[SSI_SKELETON_JOINT::LEFT_KNEE][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LKNEE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_KNEE][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LKNEE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_KNEE][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LKNEE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* LEFT_ANKLE
		*/
		to[SSI_SKELETON_JOINT::LEFT_ANKLE][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LANKLE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_ANKLE][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LANKLE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_ANKLE][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LANKLE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];

		///*
		//* LEFT_FOOT
		//*/
		to[SSI_SKELETON_JOINT::LEFT_FOOT][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LHEEL][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::LEFT_FOOT][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LHEEL][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::LEFT_FOOT][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LHEEL][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* RIGHT_HIP
		*/
		to[SSI_SKELETON_JOINT::RIGHT_HIP][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_HIP][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_HIP][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RHIP][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];



		/*
		* RIGHT_KNEE
		*/
		to[SSI_SKELETON_JOINT::RIGHT_KNEE][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RKNEE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_KNEE][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RKNEE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_KNEE][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RKNEE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		/*
		* RIGHT_ANKLE
		*/
		to[SSI_SKELETON_JOINT::RIGHT_ANKLE][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RANKLE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_ANKLE][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RANKLE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_ANKLE][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RANKLE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];

		///*
		//* RIGHT_FOOT
		//*/
		to[SSI_SKELETON_JOINT::RIGHT_FOOT][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RHEEL][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_FOOT][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RHEEL][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::RIGHT_FOOT][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::RHEEL][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		///*
		//
		//FACE
		//
		//*/

		//// face

		to[SSI_SKELETON_JOINT::FACE_LEFT_EAR][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LEAR][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::FACE_LEFT_EAR][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LEAR][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::FACE_LEFT_EAR][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LEAR][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		to[SSI_SKELETON_JOINT::FACE_RIGHT_EAR][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::REAR][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::FACE_RIGHT_EAR][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::REAR][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::FACE_RIGHT_EAR][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::REAR][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];


		to[SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::POS_X] = (from[OpenposeModelInfos::SKELETON_JOINT_BODY::LEYE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] + from[OpenposeModelInfos::SKELETON_JOINT_BODY::REYE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X]) / 2 * 1.0f;
		to[SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::POS_Y] = (from[OpenposeModelInfos::SKELETON_JOINT_BODY::LEYE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] + from[OpenposeModelInfos::SKELETON_JOINT_BODY::REYE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y]) / 2 * -1.0f;
		to[SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::POS_CONF] = (from[OpenposeModelInfos::SKELETON_JOINT_BODY::LEYE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF] + from[OpenposeModelInfos::SKELETON_JOINT_BODY::REYE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF]) / 2;


		to[SSI_SKELETON_JOINT::FACE_NOSE][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::FACE_NOSE][SSI_SKELETON_JOINT_VALUE::POS_Y] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f;
		to[SSI_SKELETON_JOINT::FACE_NOSE][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];
		

		to[SSI_SKELETON_JOINT::FACE_CHIN][SSI_SKELETON_JOINT_VALUE::POS_X] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_X] * 1.0f;
		to[SSI_SKELETON_JOINT::FACE_CHIN][SSI_SKELETON_JOINT_VALUE::POS_Y] = (from[OpenposeModelInfos::SKELETON_JOINT_BODY::NOSE][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_Y] * -1.0f) + to[SSI_SKELETON_JOINT::FACE_FOREHEAD][SSI_SKELETON_JOINT_VALUE::POS_Y] / 2;
		to[SSI_SKELETON_JOINT::FACE_CHIN][SSI_SKELETON_JOINT_VALUE::POS_CONF] = from[OpenposeModelInfos::SKELETON_JOINT_BODY::LEAR][OpenposeModelInfos::SKELETON_JOINT_VALUE::POS_CONF];
	}
}


	