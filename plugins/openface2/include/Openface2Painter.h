// Openface2Painter.h
// author: Johannes Wagner <wagner@hcm-lab.de>, Björn Bittner <bittner@hcm-lab.de>
// created: 14/6/2016
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_OPENFACE2_OPENFACEPAINTER_H
#define SSI_OPENFACE2_OPENFACEPAINTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#if __gnu_linux__
#define BYTE uint8_t
#endif


namespace ssi {

	class Openface2Helper;

	class Openface2Painter : public IFilter {

	public:

		class Options : public OptionList {

		public:

			Options() {

				poscam = true;
				posworld = true;
				corrposcam = true;
				corrposworld = true;
				detectedlandmarks = true;
				gazedirections = true;

				addOption("poscam", &poscam, 1, SSI_BOOL, "option poscam");
				addOption("posworld", &posworld, 1, SSI_BOOL, "option posworld");
				addOption("corrposcam", &corrposcam, 1, SSI_BOOL, "option corrposcam");
				addOption("corrposworld", &corrposworld, 1, SSI_BOOL, "option corrposworld");
				addOption("detectedlandmarks", &detectedlandmarks, 1, SSI_BOOL, "option detectedlandmarks");
				addOption("gazedirections", &gazedirections, 1, SSI_BOOL, "option gazedirections");

			}

			bool poscam, posworld, corrposcam, corrposworld, detectedlandmarks, gazedirections;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "Openface2Painter"; };
		static IObject *Create(const ssi_char_t *file) { return new Openface2Painter(file); };
		~Openface2Painter();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "This is just a sample object."; };

		const void *getMetaData(ssi_size_t &size) { size = sizeof(_video_format); return &_video_format; };
		void setMetaData(ssi_size_t size, const void *meta) {
			if (sizeof(_video_format) != size) {
				return;
			}
			memcpy(&_video_format, meta, size);
			_stride = ssi_video_stride(_video_format);
		};

		ssi_video_params_t getVideoFormat() { return _video_format; };


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

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) { return 1; };
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sample_bytes_in;
		};

		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_IMAGE) {
				ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
				return SSI_UNDEF;
			}
			return SSI_IMAGE;
		}


	protected:

		Openface2Painter(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		ssi_video_params_t _video_format;
		ssi_size_t _stride;

		Openface2Helper *_helper;

	};

}
#endif
