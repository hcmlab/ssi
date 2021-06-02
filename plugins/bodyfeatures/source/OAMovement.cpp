// KinectFeatures.cpp
// author: Tobias Baur <tobias.baur@informatik.uni-augsburg.de>
// created: 2012/08/10
// Copyright (C) 2007-12 University of Augsburg, Johannes Wagner
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

#include "OAMovement.h"
#include "signal/mathext.h"
#include <math.h>
#include "ssi.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	OAMovement::OAMovement(const ssi_char_t *file)
		: _file(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	OAMovement::~OAMovement() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void OAMovement::transform_enter(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
	}

	void OAMovement::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		SSI_SKELETON *ss = ssi_pcast(SSI_SKELETON, stream_in.ptr);
		ssi_real_t *dstptr = ssi_pcast(ssi_real_t, stream_out.ptr);

		ssi_size_t cnt = 0;
		ssi_real_t sum_left = 0.0f;
		ssi_real_t sum_right = 0.0f;

		for (ssi_size_t i = 0; i < stream_in.num; i++)
		{
			float normalizeFactor = 1;
			if (_options.normalizeByArmSpan)
			{
				float armSpan = computeArmSpan(&ss[i]);
				normalizeFactor = _options.stdArmSpan / armSpan;
			}

			if (ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_CONF] >= 0.5f && ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_CONF] >= 0.5f)
			{
				Vec3f left_new;
				left_new.x = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X]);
				left_new.y = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y]);
				left_new.z = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z]);

				if (_left.x > 0 && _left.y > 0 && _left.z > 0)
				{
					Vec3f delta;
					delta.x = abs(left_new.x - _left.x) * normalizeFactor;
					delta.y = abs(left_new.y - _left.y) * normalizeFactor;
					delta.z = abs(left_new.z - _left.z) * normalizeFactor;
					sum_left += delta.x + delta.y + delta.z;
				}

				//store value for next frame computation
				_left = left_new;
			}
			else
			{
				_left.x = _left.y = _left.z = 0.0f;
			}

			if (ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_CONF] >= 0.5f && ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_CONF] >= 0.5f)
			{
				Vec3f right_new;
				right_new.x = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X]);
				right_new.y = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y]);
				right_new.z = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z]);

				if (_right.x > 0 && _right.y > 0 && _right.z > 0)
				{
					Vec3f delta;
					delta.x = abs(right_new.x - _right.x) * normalizeFactor;
					delta.y = abs(right_new.y - _right.y) * normalizeFactor;
					delta.z = abs(right_new.z - _right.z) * normalizeFactor;
					sum_right += delta.x + delta.y + delta.z;
				}

				//store value for next frame computation
				_right = right_new;
			}
			else
			{
				_right.x = _right.y = _right.z = 0.0f;
			}

			cnt++;
		}

		ssi_real_t result = (cnt > 0) ? (sum_left + sum_right) / cnt : 0.0;

		if (_options.normalizeByMinMax == true)
		{
			result = normalizevalue(result, _options.normalizeminval, _options.normalizemaxval);
		}

		*(dstptr++) = result;
	}

	ssi_real_t OAMovement::normalizevalue(ssi_real_t value, ssi_real_t min, ssi_real_t max)
	{
		ssi_real_t result = (ssi_real_t)((value - min)*(1 / (max - min)));

		if (result < 0.0) return 0.0;
		else if (result > 1.0) return 1.0;
		else return result;
	}

	ssi_real_t OAMovement::computeArmSpan(SSI_SKELETON* skel)
	{
		ssi_real_t span = 0;

		Vec3f left_wrist((*skel)[SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X], (*skel)[SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y], (*skel)[SSI_SKELETON_JOINT::LEFT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z]);
		Vec3f left_elbow((*skel)[SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_X], (*skel)[SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Y], (*skel)[SSI_SKELETON_JOINT::LEFT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Z]);
		Vec3f left_shoulder((*skel)[SSI_SKELETON_JOINT::LEFT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_X], (*skel)[SSI_SKELETON_JOINT::LEFT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_Y], (*skel)[SSI_SKELETON_JOINT::LEFT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_Z]);
		Vec3f neck((*skel)[SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X], (*skel)[SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y], (*skel)[SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Z]);
		Vec3f right_wrist((*skel)[SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_X], (*skel)[SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Y], (*skel)[SSI_SKELETON_JOINT::RIGHT_WRIST][SSI_SKELETON_JOINT_VALUE::POS_Z]);
		Vec3f right_elbow((*skel)[SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_X], (*skel)[SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Y], (*skel)[SSI_SKELETON_JOINT::RIGHT_ELBOW][SSI_SKELETON_JOINT_VALUE::POS_Z]);
		Vec3f right_shoulder((*skel)[SSI_SKELETON_JOINT::RIGHT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_X], (*skel)[SSI_SKELETON_JOINT::RIGHT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_Y], (*skel)[SSI_SKELETON_JOINT::RIGHT_SHOULDER][SSI_SKELETON_JOINT_VALUE::POS_Z]);

		span += left_wrist.getDisance(&left_elbow) + left_elbow.getDisance(&left_shoulder) + left_shoulder.getDisance(&neck)
			+ neck.getDisance(&right_shoulder) + right_shoulder.getDisance(&right_elbow) + right_elbow.getDisance(&right_wrist);

		return span;
	}
}