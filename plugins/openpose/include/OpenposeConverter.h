// OpenposeConverter.h
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

#ifndef SSI_OPENPOSE_CONVERTER_H
#define SSI_OPENPOSE_CONVERTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

#include "SSI_SkeletonCons.h"
#include "Openpose.h"

namespace ssi {

	class OpenposeConverter : public IFilter {


	public:

		class Options : public OptionList {

		public:

			Options() : n_skeletons(1), skeleton_type(-1) {

				addOption("numskel", &n_skeletons, 1, SSI_SIZE, "number of skeletons tracked");
				addOption("bodytype", &skeleton_type, 1, SSI_INT, "body skeleton type (SSI = 0, MICROSOFT_KINECT = 1, OPENNI_KINECT = 2, XSENS_MVN = 3, MICROSOFT_KINECT2 = 4)");
			};

			ssi_size_t n_skeletons = 1;
			int skeleton_type;
		};

	public:
		static const ssi_char_t *GetCreateName() { return "OpenposeConverter"; };
		static IObject *Create(const ssi_char_t *file) { return new OpenposeConverter(file); };
		~OpenposeConverter();
		OpenposeConverter::Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "converts different types of skeleton streams to the ssi skeleton"; };

		void transform_enter(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform(ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_flush(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
			parseMetaData();
			return ssi_cast(ssi_size_t, _skeleton_meta_in.num * SSI_SKELETON_JOINT::NUM * SSI_SKELETON_JOINT_VALUE::NUM);
		};
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			if (sample_bytes_in != SSI_SKELETON_VALUE_BYTES) {
				ssi_err("invalid struct size");
			}
			return SSI_SKELETON_VALUE_BYTES;
		}
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_SKELETON_VALUE_SSITYPE) {
				ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
				return SSI_UNDEF;
			}
			return SSI_SKELETON_VALUE_SSITYPE;
		}

		void setMetaData(ssi_size_t size, const void *meta) {
			_meta_received++;
			memcpy(_meta_in_ptr, meta, size);
			_meta_in_ptr += size; //if there are multiple meta's, we will concatenate them
		}

		const void *getMetaData(ssi_size_t &size) {
			parseMetaData();
			_meta_out = ssi_skeleton_meta(SSI_SKELETON_TYPE::SSI, _skeleton_meta_in.num);
			size = sizeof(_meta_out);
			return &_meta_out;
		}

	protected:

		OpenposeConverter(const ssi_char_t *file = 0);
		OpenposeConverter::Options _options;
		ssi_char_t *_file;

		SSI_SKELETON_META _skeleton_meta_in;

		SSI_SKELETON_META _meta_out;
		ssi_byte_t _meta_in[256];
		ssi_byte_t *_meta_in_ptr;
		ssi_size_t _meta_received;

		void parseMetaData() {
			if (_meta_received > 0)
			{
				//ssi_print("covnerter : meta received detected %d : \n", _meta_received);

				ssi_byte_t *ptr = _meta_in;
				SSI_SKELETON_META *m = ssi_pcast(SSI_SKELETON_META, ptr);
				_skeleton_meta_in = *m;
				//ssi_print("covnerter : meta out %d : \n", _meta_out.num);

			}
			else
			{
				_skeleton_meta_in.type = ssi_cast(SSI_SKELETON_TYPE::List, _options.skeleton_type);
				_skeleton_meta_in.num = _options.n_skeletons;
			}

		}

		void convertOpenpose(void* fromPtr, SSI_SKELETON &to, int k);

	};

}

#endif
