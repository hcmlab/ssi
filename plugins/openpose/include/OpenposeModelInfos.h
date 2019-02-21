// OpenposeModelInfos.h
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

#pragma once
namespace ssi {
	class OpenposeModelInfos {

	public:
		struct SKELETON_JOINT_BODY {
			enum Value {
				NOSE,
				NECK,
				RSHOULDER,
				RELBOW,
				RWRIST,
				LSHOULDER,
				LELBOW,
				LWRIST,
				MIDHIP,
				RHIP,
				RKNEE,
				RANKLE,
				LHIP,
				LKNEE,
				LANKLE,
				REYE,
				LEYE,
				REAR,
				LEAR,
				LBIGTOE,
				LSMALLTOE,
				LHEEL,
				RBIGTOE,
				RSMALLTOE,
				RHEEL,
				NUM,
			};
		};

		struct SKELETON_JOINT_MPI {
			enum Value {
				HEAD,
				NECK,
				RSHOULDER,
				RELBOW,
				RWRIST,
				LSHOULDER,
				LWRIST,
				RHIP,
				RKNEE,
				RANKLE,
				LHIP,
				LKNEE,
				LANKLE,
				CHEST,
				NUM
			};
		};


		struct SKELETON_JOINT_VALUE {
			enum Value {
				POS_X,
				POS_Y,
				POS_CONF,
				NUM,
			};
		};
		//at first only high model
		typedef float SKELETON_BODY[SKELETON_JOINT_BODY::NUM][SKELETON_JOINT_VALUE::NUM];
		typedef float SKELETON_MPI[SKELETON_JOINT_MPI::NUM][SKELETON_JOINT_VALUE::NUM];


		//FACE : Size: #people x #face parts (70) x 3 ((x,y) coordinates + score)
		struct SKELETON_FACE_MODEL {
			enum Value {
				PARTS = 70,
			    DIM = 3
			};
		};

		//HAND : Size each Array: #people x #hand parts (21) x 3 ((x,y) coordinates + score)
		struct SKELETON_HAND_MODEL {
			enum Value {
				PARTS = 21,
				DIM = 3
			};
		};
	};
}