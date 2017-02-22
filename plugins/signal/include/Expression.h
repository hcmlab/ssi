// Expression.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/04
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

#ifndef SSI_SIGNAL_EXPRESSION_H
#define SSI_SIGNAL_EXPRESSION_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

class ExpParser;

namespace ssi {

class Expression : public IFilter {

public:

	struct JOIN {
		enum List {
			OFF = 0,
			MULT = 1,
			SUM = 2
		};
	};

public:

	class Options : public OptionList {

	public:

		Options () : single (false), join (JOIN::OFF), cast(SSI_FLOAT) {
			
			setExpression ("d + d");			
			
			addOption ("single", &single, 1, SSI_BOOL, "apply a single expression to all dimensions otherwise expression is applied to each dimension (use d0, d1, ... to address specific dimension or just d otherwise)");		
			addOption ("expression", expression, SSI_MAX_CHAR, SSI_CHAR, "expression string (e.g. 'd+d' doubles each dimension, 'd0+d1+d2..' sums up dimensions)");					
			addOption ("join", &join, 1, SSI_INT, "only applies if single==false : join dimensions after applying expression (0=off, 1=multiply, 2=sum up");
			addOption ("cast", &cast, 1, SSI_SIZE, "cast result after expression was applied (CHAR = 1, UCHAR = 2, SHORT = 3, USHORT = 4, INT = 5, UINT = 6, LONG = 7, ULONG = 8, FLOAT = 9, DOUBLE = 10, LDOUBLE = 11, STRUCT = 12, IMAGE = 13, BOOL = 14");
		};

		void setExpression (const ssi_char_t *string) {
			strcpy (expression, string);
		}

		bool single;
		ssi_char_t expression[SSI_MAX_CHAR];
		JOIN::List join;
		ssi_size_t cast;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Expression"; };
	static IObject *Create (const ssi_char_t *file) { return new Expression (file); };
	~Expression ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Applies mathematical expressions to a stream (see http://www.speqmath.com/tutorials/expression_parser_cpp/index.html for supported operators)."; };

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

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		return _options.single ? 1 : _options.join == JOIN::OFF ? sample_dimension_in : 1;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		ssi_type_t type = ssi_type_t(_options.cast);
		return ssi_type2bytes(type);
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return ssi_type_t (_options.cast);
	}

protected:

	Expression (const ssi_char_t *file = 0);
	Expression::Options _options;
	ssi_char_t *_file;

	bool _single;
	JOIN::List _join;
	ExpParser *_parser;
	ssi_char_t _expression[SSI_MAX_CHAR];

	void insert (ssi_char_t *src, ssi_char_t *dst, ssi_size_t current, ssi_real_t *sample); // returns 0 if nothing to insert
};

}

#endif
