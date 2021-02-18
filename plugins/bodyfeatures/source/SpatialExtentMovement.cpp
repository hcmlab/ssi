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

#include "SpatialExtentMovement.h"
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
	SpatialExtentMovement::SpatialExtentMovement(const ssi_char_t *file)
		: _file(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	SpatialExtentMovement::~SpatialExtentMovement() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void SpatialExtentMovement::transform_enter(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		tmp_x = 0;
		tmp_y = 0;
		tmp_z = 0;
		for (int i = 0; i < SSI_SKELETON_JOINT::NUM; i++)
		{
			for (int j = 0; j < SSI_SKELETON_JOINT_VALUE::NUM; j++)
			{
				energydata_temp[i][j] = 0;
			}

			sumcount[i] = 0;
			sumdata[i] = 0;
		}
	}

	void SpatialExtentMovement::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;
		SSI_SKELETON *ss = ssi_pcast(SSI_SKELETON, stream_in.ptr);

		ssi_real_t leftx = 0;
		ssi_real_t lefty = 0;
		ssi_real_t rightx = 0;
		ssi_real_t righty = 0;
		ssi_real_t *dstptr = ssi_pcast(ssi_real_t, stream_out.ptr);

		for (ssi_size_t i = 0; i < stream_in.num; i++)
		{
			leftx = leftx + abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::LEFT_HAND][SSI_SKELETON_JOINT_VALUE::POS_X]);
			lefty = lefty + abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::LEFT_HAND][SSI_SKELETON_JOINT_VALUE::POS_Y]);
			rightx = rightx + abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][SSI_SKELETON_JOINT::RIGHT_HAND][SSI_SKELETON_JOINT_VALUE::POS_X]);
			righty = righty + abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][SSI_SKELETON_JOINT::RIGHT_HAND][SSI_SKELETON_JOINT_VALUE::POS_Y]);
		}

		leftx = leftx / stream_in.num;
		lefty = lefty / stream_in.num;
		rightx = rightx / stream_in.num;
		righty = righty / stream_in.num;
		*(dstptr) = sqrt((leftx - rightx) * (leftx - rightx) + (lefty - righty) * (lefty - righty));
	}
}