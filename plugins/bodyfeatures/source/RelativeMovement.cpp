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

#include "RelativeMovement.h"
#include "signal/mathext.h"
#include <math.h>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {
	RelativeMovement::RelativeMovement(const ssi_char_t *file)
		: _file(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	RelativeMovement::~RelativeMovement() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void RelativeMovement::transform_enter(
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


	void RelativeMovement::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;

		memcpy(stream_out.ptr, stream_in.ptr, stream_in.tot);

		SSI_SKELETON *src = ssi_pcast(SSI_SKELETON, stream_in.ptr);
		SSI_SKELETON *dst = ssi_pcast(SSI_SKELETON, stream_out.ptr);
		
		for (ssi_size_t i = 0; i < stream_in.num; i++)
		{
			for (int l = 0; l < SSI_SKELETON_JOINT::NUM; l++)
			{
				dst[i][l][SSI_SKELETON_JOINT_VALUE::POS_X] -= src[i][_options.joint][SSI_SKELETON_JOINT_VALUE::POS_X];
				dst[i][l][SSI_SKELETON_JOINT_VALUE::POS_Y] -= src[i][_options.joint][SSI_SKELETON_JOINT_VALUE::POS_Y];
				dst[i][l][SSI_SKELETON_JOINT_VALUE::POS_Z] -= src[i][_options.joint][SSI_SKELETON_JOINT_VALUE::POS_Z];
			}
		}

	}
}