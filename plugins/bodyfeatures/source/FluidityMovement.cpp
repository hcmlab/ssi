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

#include "FluidityMovement.h"
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
	FluidityMovement::FluidityMovement(const ssi_char_t *file)
		: _file(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	FluidityMovement::~FluidityMovement() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void FluidityMovement::transform_enter(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		tmp_x = 0;
		tmp_y = 0;
		tmp_z = 0;

		tmp2_x = 0;
		tmp2_y = 0;
		tmp2_z = 0;

		tmpdiff_x = 0;
		tmpdiff_y = 0;
		tmpdiff_z = 0;

		sumdataleft = new ssi_real_t[stream_in.num];
		sumdataright = new ssi_real_t[stream_in.num];

		for (int i = 0; i < stream_in.num; i++)
		{
			sumdataleft[i] = 0.0;
			sumdataright[i] = 0.0;
		}

		
	}

	void FluidityMovement::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;
		SSI_SKELETON *ss = ssi_pcast(SSI_SKELETON, stream_in.ptr);

		ssi_real_t *dstptr = ssi_pcast(ssi_real_t, stream_out.ptr);
		*dstptr = 0;

		int joint_center = _options.legs ? SSI_SKELETON_JOINT::WAIST : SSI_SKELETON_JOINT::NECK;
		int joint_left = _options.legs ? SSI_SKELETON_JOINT::LEFT_FOOT : SSI_SKELETON_JOINT::LEFT_HAND;
		int joint_right = _options.legs ? SSI_SKELETON_JOINT::RIGHT_FOOT : SSI_SKELETON_JOINT::RIGHT_HAND;


		if (!_options.alternativeAlgorithm){
			for (ssi_size_t i = 0; i < stream_in.num - 1; i++)
			{
				tmp_x = abs(abs(ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][joint_left][SSI_SKELETON_JOINT_VALUE::POS_X]) - abs(ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 1][joint_left][SSI_SKELETON_JOINT_VALUE::POS_X]));
				tmp_y = abs(abs(ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Y]) - abs(ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 1][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Y]));
				tmp_z = abs(abs(ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Z]) - abs(ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 1][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Z]));
				sumdataleft[i] = std::sqrt(tmp_x * tmp_x + tmp_y * tmp_y + tmp_z * tmp_z);

				tmp_x = abs(abs(ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][joint_right][SSI_SKELETON_JOINT_VALUE::POS_X]) - abs(ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_X]));
				tmp_y = abs(abs(ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Y]) - abs(ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Y]));
				tmp_z = abs(abs(ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Z]) - abs(ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Z]));
				sumdataright[i] = std::sqrt(tmp_x * tmp_x + tmp_y * tmp_y + tmp_z * tmp_z);
			}

			ssi_real_t *dstleft = new ssi_real_t();
			ssi_real_t *dstright = new ssi_real_t();
			ssi_var(stream_in.num, 1, sumdataleft, dstleft);
			ssi_var(stream_in.num, 1, sumdataright, dstright);
			ssi_real_t result = (*dstleft + *dstright) / 2;
			if (result < 10000) *dstptr = result;

			
		}
		else{
			for (ssi_size_t i = 0; i < stream_in.num - 2; i++)
			{
				tmp_x = ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - (ss[i][joint_left][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X]);
				tmp_y = ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - (ss[i][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y]);
				tmp_z = ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - (ss[i][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z]);

				tmp2_x = ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - (ss[i + 1][joint_left][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X]);
				tmp2_y = ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - (ss[i + 1][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y]);
				tmp2_z = ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - (ss[i + 1][joint_left][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z]);
				
				tmpdiff_x = tmp2_x - tmp_x;
				tmpdiff_y = tmp2_y - tmp_y;
				tmpdiff_z = tmp2_z - tmp_z;

				sumdataleft[i] = std::sqrt(tmpdiff_x * tmpdiff_x + tmpdiff_y * tmpdiff_y + tmpdiff_z * tmpdiff_z);


				tmp_x = ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - (ss[i][joint_right][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X]);
				tmp_y = ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - (ss[i][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y]);
				tmp_z = ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - (ss[i][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z]);

				tmp2_x = ss[i + 2][joint_right][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X] - (ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_X]);
				tmp2_y = ss[i + 2][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y] - (ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Y]);
				tmp2_z = ss[i + 2][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 2][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z] - (ss[i + 1][joint_right][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i + 1][joint_center][SSI_SKELETON_JOINT_VALUE::POS_Z]);
				
				tmpdiff_x = tmp2_x - tmp_x;
				tmpdiff_y = tmp2_y - tmp_y;
				tmpdiff_z = tmp2_z - tmp_z;

				sumdataright[i] = std::sqrt(tmpdiff_x * tmpdiff_x + tmpdiff_y * tmpdiff_y + tmpdiff_z * tmpdiff_z);

			}

			ssi_real_t *dstleft = new ssi_real_t();
			ssi_real_t *dstright = new ssi_real_t();
			ssi_var(stream_in.num, 1, sumdataleft, dstleft);
			ssi_var(stream_in.num, 1, sumdataright, dstright);
			ssi_real_t result = (*dstleft + *dstright) / 2;
			if (result < 10000) *dstptr = result;
			

		}
		
		for (int i = 0; i < stream_in.num; i++)
		{
			sumdataleft[i] = 0.0;
			sumdataright[i] = 0.0;
		}
		
	}
}