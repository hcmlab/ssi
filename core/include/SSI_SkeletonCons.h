// Skeleton.h
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
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

/*
 * Provides a standardized SSI skeleton for both body and face
 * - unit: mm
 * - max users: 2
 */

#pragma once

#ifndef SSI_SKELETON_CONS_H
#define	SSI_SKELETON_CONS_H

#include "SSI_Cons.h"

#define SSI_SKELETON_MAX_USERS				2
#define SSI_SKELETON_VALUE_TYPE				ssi_real_t
#define SSI_SKELETON_VALUE_SSITYPE			SSI_REAL
#define SSI_SKELETON_VALUE_BYTES			sizeof (SSI_SKELETON_VALUE_TYPE)
#define SSI_SKELETON_INVALID_JOINT_VALUE	0.0f

struct SSI_SKELETON_JOINT
{
	enum List
	{		
		HEAD = 0,
		NECK = 1,
		TORSO = 2,
		WAIST = 3,
		LEFT_SHOULDER = 4,
		LEFT_ELBOW = 5,
		LEFT_WRIST = 6,
		LEFT_HAND = 7,
		RIGHT_SHOULDER = 8,
		RIGHT_ELBOW = 9,
		RIGHT_WRIST = 10,
		RIGHT_HAND = 11,
		LEFT_HIP = 12,
		LEFT_KNEE = 13,
		LEFT_ANKLE = 14,
		LEFT_FOOT = 15,
		RIGHT_HIP = 16,
		RIGHT_KNEE = 17,
		RIGHT_ANKLE = 18,
		RIGHT_FOOT = 19,

		FACE_NOSE = 20,
		FACE_LEFT_EAR = 21,
		FACE_RIGHT_EAR = 22,
		FACE_FOREHEAD = 23,
		FACE_CHIN = 24,

		NUM
	};
};

struct SSI_SKELETON_JOINT_VALUE
{
	enum List
	{
		POS_X = 0,
		POS_Y = 1,
		POS_Z = 2,
		POS_CONF = 3,
		
		ROT_W = 4,
		ROT_X = 5,
		ROT_Y = 6,
		ROT_Z = 7,
		ROT_CONF = 8,
		
		ROT_REL_W = 9,
		ROT_REL_X = 10,
		ROT_REL_Y = 11,
		ROT_REL_Z = 12,
		ROT_REL_CONF = 13,
			
		NUM
	};
};

struct SSI_SKELETON_TYPE
{
	enum List
	{
		SSI = 0,
		MICROSOFT_KINECT = 1,
		OPENNI_KINECT = 2,
		XSENS_MVN = 3,
		MICROSOFT_KINECT2 = 4,
			
		NUM
	};
};
extern ssi_char_t *SSI_SKELETON_TYPE_NAMES[];

struct SSI_FACE_TYPE
{
	enum List
	{
		SSI = 0,
		MICROSOFT_KINECT = 1,
		MICROSOFT_KINECT2 = 2,

		NUM
	};
};
extern ssi_char_t *SSI_FACE_TYPE_NAMES[];

#define SSI_SKELETON_META_NAME "skeleton"
struct SSI_SKELETON_META {
	ssi_char_t name[9];
	SSI_SKELETON_TYPE::List type;
	ssi_size_t num;
};

#define SSI_FACE_META_NAME "face"
struct SSI_FACE_META {
	ssi_char_t name[5];
	SSI_FACE_TYPE::List type;
	ssi_size_t num;
};

typedef SSI_SKELETON_VALUE_TYPE SSI_SKELETON[SSI_SKELETON_JOINT::NUM][SSI_SKELETON_JOINT_VALUE::NUM];

SSI_INLINE static SSI_SKELETON_META ssi_skeleton_meta(SSI_SKELETON_TYPE::List type, ssi_size_t num)
{
	SSI_SKELETON_META m;
	sprintf(m.name, SSI_SKELETON_META_NAME);
	m.type = type;
	m.num = num;
	return m;
}

SSI_INLINE static SSI_FACE_META ssi_face_meta(SSI_FACE_TYPE::List type, ssi_size_t num)
{
	SSI_FACE_META m;
	sprintf(m.name, SSI_FACE_META_NAME);
	m.type = type;
	m.num = num;
	return m;
}

#endif

