// OpenfaceSelector.h
// author: Johannes Wagner <wagner@hcm-lab.de>
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

#ifndef SSI_OPENFACE_OPENFACESELECTOR_H
#define SSI_OPENFACE_OPENFACESELECTOR_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "Openface.h"

namespace ssi {

	class OpenfaceSelector : public IFilter {

	public:

		class Options : public OptionList {

		public:

			Options() {

				success = false;
				confidence = false;
				poscam = false;
				posworld = false;
				corrposcam = false;
				corrposworld = false;
				landmarks = false;
				gaze = false;
				pupil = false;
				auclass = false;
				auclass_success = false;
				aureg = false;			
				aureg_success = false;

				addOption("success", &success, 1, SSI_BOOL, "Select detection success (1 value)");
				addOption("confidence", &confidence, 1, SSI_BOOL, "Select detection confidence (1 value)");
				addOption("poscam", &poscam,1, SSI_BOOL, "Select camera position (x,y,z,rot_x,rot_y,rot_z)");
				addOption("posworld", &posworld, 1, SSI_BOOL, "Select world position (x,y,z,rot_x,rot_y,rot_z)");
				addOption("corrposcam", &corrposcam, 1, SSI_BOOL, "Select corrected camera position (x,y,z,rot_x,rot_y,rot_z)");
				addOption("corrposworld", &corrposworld, 1, SSI_BOOL, "Select correted world position (x,y,z,rot_x,rot_y,rot_z)");
				addOption("landmarks", &landmarks, 1, SSI_BOOL, "Select landmarks (x_1,y_1,...,x_68,y_68)");
				addOption("gaze", &gaze, 1, SSI_BOOL, "Select gaze direction (x_left,y_left,_z_left,y_right,y_right,_z_right)");
				addOption("pupil", &pupil, 1, SSI_BOOL, "Calculate pupil position (x_left,y_left,z_left,y_right,y_right,z_right)");
				addOption("auclass_success", &auclass_success, 1, SSI_BOOL, "Select action units success (classification)");
				addOption("auclass", &auclass, 1, SSI_BOOL, "Select action units (classification -> AU01=InnerBrowRaiser,AU02=OuterBrowRaiser,AU04=BrowLowerer,AU05=UpperLidRaiser,AU06=CheekRaiser,AU09=NoseWrinkler,AU10=UpperLipRaiser,AU12=LipCornerPuller,AU14=Dimpler,AU15=LipCornerDepressor,AU17=ChinRaiser,AU20=LipStretcher,AU25=LipsPart,AU26=JawDrop)");
				addOption("aureg_success", &aureg_success, 1, SSI_BOOL, "Select action units success (regression)");
				addOption("aureg", &aureg, 1, SSI_BOOL, "Select action units (regression -> AU04=BrowLowerer,AU12=LipCornerPuller,AU15=LipCornerDepressor,AU23=LipTightener,AU28=LipSuck,AU45=Blink)");
			}

			bool success, confidence, poscam, posworld, corrposcam, corrposworld, landmarks, gaze, pupil, auclass_success, auclass, aureg_success, aureg;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "OpenfaceSelector"; };
		static IObject *Create(const ssi_char_t *file) { return new OpenfaceSelector(file); };
		~OpenfaceSelector();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Select feature groups."; };

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
			return parseOptions();
		}

		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
			return sizeof(SSI_REAL);
		}

		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
			return SSI_REAL;
		}

	protected:

		OpenfaceSelector(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		ssi_size_t parseOptions();		
		int _selection[Openface::FEATURE::NUM];

	};

}

#endif
