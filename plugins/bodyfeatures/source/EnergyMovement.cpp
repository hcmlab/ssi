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

#include "EnergyMovement.h"
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
	EnergyMovement::EnergyMovement(const ssi_char_t *file)
		: _file(0) {
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	EnergyMovement::~EnergyMovement() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void EnergyMovement::transform_enter(
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[])
	{
		tmp_x = 0;
		tmp_y = 0;
		tmp_z = 0;
		for (int i = 0; i < _options.maxdim; i++)
		{
			for (int j = 0; j < SSI_SKELETON_JOINT_VALUE::NUM; j++)
			{
				energydata_temp[i][j] = 0;
			}

			sumcount[i] = 0;
			sumdata[i] = 0;
		}
	}

	ssi_real_t calculateEnergy(ssi_real_t x, ssi_real_t y, ssi_real_t z)
	{
		return ((x * x + y * y + z * z) / 3);
	}

	void EnergyMovement::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;
		SSI_SKELETON *ss = ssi_pcast(SSI_SKELETON, stream_in.ptr);
		ssi_real_t *dstptr = ssi_pcast(ssi_real_t, stream_out.ptr);

		for (int i = 0; i < _options.maxdim; i++)
		{
			sumcount[i] = 0;
			sumdata[i] = 0;
		}



		for (ssi_size_t i = 0; i < stream_in.num; i++)
		{
			for (int l = 0; l < _options.maxdim; l++)
			{
				if (ss[i][l][SSI_SKELETON_JOINT_VALUE::POS_CONF] > 0.5f && ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_CONF] > 0.5f)
				{
					if (energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_X] > 0 && energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Y] > 0 && energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Z] > 0)
					{
						tmp_x = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][l][SSI_SKELETON_JOINT_VALUE::POS_X]) - energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_X];
						tmp_y = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][l][SSI_SKELETON_JOINT_VALUE::POS_Y]) - energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Y];
						tmp_z = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][l][SSI_SKELETON_JOINT_VALUE::POS_Z]) - energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Z];
						sumdata[l] += calculateEnergy(tmp_x, tmp_y, tmp_z);
						sumcount[l] += 1;
					}
					energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_X] = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_X] - ss[i][l][SSI_SKELETON_JOINT_VALUE::POS_X]);
					energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Y] = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Y] - ss[i][l][SSI_SKELETON_JOINT_VALUE::POS_Y]);
					energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Z] = abs(ss[i][SSI_SKELETON_JOINT::NECK][SSI_SKELETON_JOINT_VALUE::POS_Z] - ss[i][l][SSI_SKELETON_JOINT_VALUE::POS_Z]);
				}
				else
				{
					energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_X] = energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Y] = energydata_temp[l][SSI_SKELETON_JOINT_VALUE::POS_Z] = 0.0f;
				}
			}
		}





		for (int i = 0; i < _options.maxdim; i++)
		{
			ssi_real_t val = sumcount[i] > 0 ? sqrt(sumdata[i] / sumcount[i]) : 0;

			if (_options.norm){
				*(dstptr++) = normalizevalue(val, 0, _options.normmax);
			}
			else {
				*(dstptr++) = val;
			}
			
		}
	}


	ssi_real_t EnergyMovement::normalizevalue(ssi_real_t value, ssi_real_t min, ssi_real_t max) {
		ssi_real_t result = (ssi_real_t)((value - min)*(1 / (max - min)));
		if (result < 0.0) return 0.0;
		else if (result > 1.0) return 1.0;
		else return result;
	}
}