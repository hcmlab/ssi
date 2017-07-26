// Chain.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/08/29
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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

#pragma once

#ifndef SSI_FRAME_CHAIN_H
#define SSI_FRAME_CHAIN_H

#include "base/ITransformer.h"
#include "base/IFilter.h"
#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "ioput/xml/tinyxml.h"
#include "base/IXMLPipeline.h"

namespace ssi {

/**
 * \brief Puts multiple filter and feature objects in series.
 *
 * A chain consists of a set of Filter and Feature transformer.
 * First, incoming data is successively run through all filters
 * (in order given by the filter array). Then features are
 * calculated from the filter output and concatenated (again in
 * order given by the feature array).
 *
 * Either the filter or the feature set may be empty.
 *
 * @author Johannes Wagner
 * @date  Feb 2009
 */
class Chain : public ITransformer {

public:

	enum VERSION {
		V1 = 1,		// original format
	};
	static Chain::VERSION DEFAULT_VERSION;

public:

	class Options : public OptionList {

	public:

		Options () {

			path[0] = '\0';
			addOption("file", path, SSI_MAX_CHAR, SSI_CHAR, "deprecated, see path");
			addOption("path", path, SSI_MAX_CHAR, SSI_CHAR, "path to file with chain definition");
		};

		void set (const ssi_char_t *f) {
			ssi_strcpy(path, f);
		};

		ssi_char_t path[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "Chain"; };
	static IObject *Create (const ssi_char_t *file) { return new Chain (file); };
	virtual ~Chain ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Combines a series of filter and feature components in a single transformer."; };

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

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in);
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in);

	virtual void set (ssi_size_t n_filters,
		IFilter **filters,
		ssi_size_t n_features,
		IFeature **features) {

		release ();
		_n_filters = n_filters;
		_filters = filters;
		_n_features = n_features;
		_features = features;
		_load_from_file = false;
	}
	virtual void parse();
	virtual void release();

	const void *getMetaData (ssi_size_t &size) {
		size = _meta_size;
		return _meta_data;
	};

	void setMetaData (ssi_size_t size, const void *meta) {
		ssi_size_t meta_size = size;
		const void *meta_data = meta;

		parse();

		for (ssi_size_t i = 0; i < _n_filters; i++) {
			_filters[i]->setMetaData (meta_size, meta_data);
			meta_data = _filters[i]->getMetaData (meta_size);
		}
		if (_n_features > 0) {
			for (ssi_size_t i = 0; i < _n_features; i++) {
				_features[i]->setMetaData (meta_size, meta_data);
			}
		} else if (meta_size > 0) {
			_meta_size = meta_size;
			_meta_data = new ssi_byte_t[_meta_size];
			memcpy (_meta_data, meta_data, _meta_size);
		}

	};

protected:

	Chain (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	bool getPath();	
	ssi_char_t *_chain_path;
	ssi_char_t *_chain_dir;
	ssi_char_t *_work_dir;

	void changeWorkDir();
	void resetWorkDir();

	ssi_size_t calc_sample_dimension_out (ssi_size_t sample_dimension_in);
	ssi_size_t calc_sample_number_out (ssi_size_t sample_number_in);
	ssi_size_t calc_sample_bytes_out (ssi_size_t sample_bytes_in);
	ssi_type_t calc_sample_type_out (ssi_type_t sample_type_in);

	static ssi_char_t *ssi_log_name;

	bool parseFilter (TiXmlElement *element);
	bool parseFeature (TiXmlElement *element);

	bool _parsed;
	bool _load_from_file;
	ssi_size_t _n_filters;
	IFilter **_filters;
	ssi_size_t _n_features;
	IFeature **_features;

	ssi_stream_t **_stream_tmp;
	ssi_size_t _stream_tmp_num;
	ssi_size_t *_feature_dim_out;
	ssi_size_t _feature_dim_out_tot;

	ssi_size_t _meta_size;
	ssi_byte_t *_meta_data;	
};

}

#endif
