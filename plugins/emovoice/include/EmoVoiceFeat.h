// EmoVoiceFeat.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2007/10/26
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

//! \brief VAD calculation using Esmeralda.

#ifndef SSI_SIGNAL_EMOVOICEFEAT_H
#define SSI_SIGNAL_EMOVOICEFEAT_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class EmoVoiceFeat : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options () {

			delim = ';';
			binary = 1;
			fselect = features_out = confidences = 0;
			fselection_def[0] = '\0'; ffile[0] = '\0'; cl_file[0] = '\0';
			strcpy (m_e_params, "desklab");
			maj = 2;
			min = 0;

			addOption ("delim", &delim, 1, SSI_CHAR, "delim");
			addOption ("fselect", &fselect, 1, SSI_INT, "fselect");
			addOption ("features_out", &features_out, 1, SSI_INT, "features_out");
			addOption ("binary", &binary, 1, SSI_INT, "binary");
			addOption ("maj", &maj, 1, SSI_INT, "major version number (1=1316 features, 2=1451)");
			addOption ("min", &min, 1, SSI_INT, "minor version number");
			addOption ("confidences", &confidences, 1, SSI_INT, "confidences");
			addOption ("cl_file", cl_file, SSI_MAX_CHAR, SSI_CHAR, "cl_file");
			addOption ("fselection_def", fselection_def, SSI_MAX_CHAR, SSI_CHAR, "fselection_def");
			addOption ("ffile", ffile, SSI_MAX_CHAR, SSI_CHAR, "ffile");
			addOption ("m_e_params", m_e_params, SSI_MAX_CHAR, SSI_CHAR, "m_e_params");
		};

		char delim;
		int fselect;
		int features_out;
		int binary;
		int maj;
		int min;
		int confidences;
		char cl_file[SSI_MAX_CHAR];
		char fselection_def[SSI_MAX_CHAR];
		char ffile[SSI_MAX_CHAR];
		char m_e_params[SSI_MAX_CHAR];
	};


public:

	static const ssi_char_t *GetCreateName () { return "EmoVoiceFeat"; };
	static IObject *Create (const ssi_char_t *file) { return new EmoVoiceFeat (file); };
	EmoVoiceFeat::Options *getOptions () { return &_options; };
	~EmoVoiceFeat ();
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Emo Voice Feature Set"; };

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in);

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

protected:

	EmoVoiceFeat (const ssi_char_t *file = 0);
	EmoVoiceFeat::Options _options;
	ssi_char_t *_file;

	ssi_stream_t _stream_short;

	void *_fex;
	void *_fselection;
	void *_feature_vector;
	int _n_features, _n_features_reduced;
};

}

#endif
