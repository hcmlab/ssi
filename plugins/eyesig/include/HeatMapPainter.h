// HeatMapPainter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/06/23
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

#ifndef SSI_HEAT_MAP_PAINTER_H
#define SSI_HEAT_MAP_PAINTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "HeatmapWorker.h"

namespace ssi {

	class HeatMapPainter : public IFilter {

	public:

		class Options : public OptionList {
			public:


			Options () : minimudisplaypercentage (0.05f), decreasefactor (0.5f), smoothen(false), decreaseovertime(false), verticalregions(1), horizontalregions(1), decreasetick(2), filterrepetitions(0), opacity(0.05f), influencradius(30) {

				addOption ("influencradius", &influencradius, 1, SSI_INT, "the radius around a pixel which will be also influenced if the pixel is triggered");
				addOption ("opacity", &opacity, 1, SSI_FLOAT, "the opacity of the heatmap overlay");
				addOption ("minimudisplaypercentage", &minimudisplaypercentage, 1, SSI_FLOAT, "if a density is less then value * maximal density it will not be displayed");
				addOption ("horizontalregions", &horizontalregions, 1, SSI_INT, "specifying the total number of horizontal areas for decreasing the density-values over time. image-width/value should be an integer");	
				addOption ("verticalregions", &verticalregions, 1, SSI_INT, "specifying the total number of vertical areas for decreasing the density-values over time. image-height/value should be an integer");
				addOption ("smoothen", &smoothen, 1, SSI_BOOL, "applying a filter to smoothen the image. can lead to massive performance drop"); //not working at the moment
				addOption ("filterrepetions", &filterrepetitions, 1, SSI_INT, "specifies the number of the times the smoothening-filter will be applied. higher means smoother image and highter performancecosts");
				addOption ("decreaseovertime", &decreaseovertime, 1, SSI_BOOL, "density of untouched areas will decrease over time if set to true");
				addOption ("decreasetick", &decreasetick, 1, SSI_FLOAT, "if an area has no change in density it will decrease it values every tick in seconds");
				addOption ("decreasefactor", &decreasefactor, 1, SSI_FLOAT, "specifies the factor for decreasing areas without activity each tick");
			};

				float minimudisplaypercentage, decreasefactor,decreasetick, opacity;
				bool smoothen, decreaseovertime;
				int verticalregions, horizontalregions, filterrepetitions, influencradius;
		};

	public:

		static const ssi_char_t *GetCreateName () { return "HeatMapPainter"; };
		static IObject *Create (const ssi_char_t *file) { return new HeatMapPainter (file); };
		~HeatMapPainter ();
		HeatMapPainter::Options *getOptions () { return &_options; };
		const ssi_char_t *getName () { return GetCreateName (); };
		const ssi_char_t *getInfo () { return "creates a heatmap overlay for eyegazes features in a video stream"; };

		void setVideoFormat (ssi_video_params_t video_format) { _video_format = video_format; };
		ssi_video_params_t getVideoFormat () { return _video_format; };

		const void *getMetaData (ssi_size_t &size) { size = sizeof (_video_format); return &_video_format; };
		void setMetaData (ssi_size_t size, const void *meta) {
			if (sizeof (_video_format) != size) {
				ssi_err ("invalid meta size");
			}
			memcpy (&_video_format, meta, size);
		};

		void transform_enter (ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform (ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_flush (ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) { return sample_dimension_in; };
		ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
			if (sample_bytes_in != ssi_video_size (_video_format)) {
				ssi_err ("invalid byte size");
			}
			return ssi_video_size (_video_format); 
		};	
		ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
			if (sample_type_in != SSI_IMAGE) {
				ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			}
			return SSI_IMAGE;
		};

	protected:

		HeatMapPainter (const ssi_char_t *file = 0);
		HeatMapPainter::Options _options;
		ssi_char_t *_file;

		void paint (BYTE *image,
			ssi_size_t n_eyes,
			float *eyes);

		void paint_point (BYTE *image, 
			ssi_video_params_t &params, 
			int point_x, 
			int point_y, 
			int border, 
			int r_value, 
			int g_value, 
			int b_value);

		void paint_heatmap(BYTE *image);

		ssi_video_params_t _video_format;
		HeatmapWorker *hmWorker;

	};

}

#endif
