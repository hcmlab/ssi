// SkeletonSelector.h
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

#ifndef SSI_SKELETON_SELECTOR_H
#define SSI_SKELETON_SELECTOR_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class SkeletonSelector : public IFilter {


public:

class Options : public OptionList {

	public:

		Options () {

			joints[0] = '\0';
			values[0] = '\0';
			addOption ("joints", joints, SSI_MAX_CHAR, SSI_CHAR, "indices of selected joints (HIP_CENTER = 0, TORSO = 1, SHOULDER_CENTER = 2, HEAD = 3, SHOULDER_LEFT = 4, ELBOW_LEFT = 5, WRIST_LEFT = 6, HAND_LEFT = 7, SHOULDER_RIGHT = 8, ELBOW_RIGHT = 9, WRIST_RIGHT = 10, HAND_RIGHT = 11, HIP_LEFT = 12, KNEE_LEFT = 13, ANKLE_LEFT = 14, FOOT_LEFT = 15, HIP_RIGHT = 16, KNEE_RIGHT = 17, ANKLE_RIGHT = 18, FOOT_RIGHT = 19)");
			addOption ("values", values, SSI_MAX_CHAR, SSI_CHAR, "indices of selected joint values (POS_X = 0, POS_Y = 1, POS_Z = 2, POS_CONF = 3, ROT_W = 4, ROT_X = 5, ROT_Y = 6, ROT_Z = 7, ROT_CONF = 8, ROT_REL_W = 9, ROT_REL_X = 10, ROT_REL_Y = 11, ROT_REL_Z = 12)");
		};

		void setJoints (ssi_size_t index) {
			setJoints (1, &index);
		}

		void setValues (ssi_size_t index) {
			setValues (1, &index);
		}

		void setJoints (ssi_size_t n_inds, ssi_size_t *inds) {
			joints[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%u", inds[0]);
				strcat (joints, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%u", inds[i]);
					strcat (joints, s);
				}
			}
		}

		void setValues (ssi_size_t n_inds, ssi_size_t *inds) {
			values[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%u", inds[0]);
				strcat (values, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%u", inds[i]);
					strcat (values, s);
				}
			}
		}

		ssi_char_t joints[SSI_MAX_CHAR];
		ssi_char_t values[SSI_MAX_CHAR];
	};

public:
	static const ssi_char_t *GetCreateName () { return "SkeletonSelector"; };
	static IObject *Create (const ssi_char_t *file) { return new SkeletonSelector (file); };
	~SkeletonSelector ();
	SkeletonSelector::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "selects specific elements from a ssi skeleton stream"; };

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

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) { 
		if (_options.joints[0] != '\0') {
			releaseSelection ();
			parseSelection ();
		}
		_n_skeletons = sample_dimension_in / (SSI_SKELETON_JOINT_VALUE::NUM * SSI_SKELETON_JOINT::NUM);
		return ssi_cast (ssi_size_t, _n_skeletons * _n_selected); 
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != SSI_SKELETON_VALUE_BYTES) {
			ssi_err ("invalid struct size");
		}
		return SSI_SKELETON_VALUE_BYTES; 
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_SKELETON_VALUE_SSITYPE) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_SKELETON_VALUE_SSITYPE;
	}

	void select(SSI_SKELETON_JOINT::List j, SSI_SKELETON_JOINT_VALUE::List v)
	{
		_selector[j][v] = true;
		++_n_selected;
	};

protected:

	SkeletonSelector (const ssi_char_t *file = 0);
	SkeletonSelector::Options _options;
	ssi_char_t *_file;

	bool _selector[SSI_SKELETON_JOINT::NUM][SSI_SKELETON_JOINT_VALUE::NUM];
	ssi_size_t _n_selected;
	ssi_size_t _n_skeletons;

	void parseSelection ();
	void releaseSelection ();
};

}

#endif
