// Expression.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/03
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

#include "Expression.h"
#include "signal/MatrixOps.h"

#include "expparser.h"


#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif



namespace ssi {
Expression::Expression (const ssi_char_t *file) 
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_parser = new ExpParser ();
}

Expression::~Expression () {

	delete _parser;

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void Expression::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	
	_single = _options.single;
	_join = _options.join;
}

void Expression::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {	

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	double result;

	if (_single) {
		for (ssi_size_t i = 0; i < stream_in.num; i++) {
			insert (_options.expression,_expression, 0, srcptr);
			const ssi_char_t *err = _parser->parse (_expression, result);
			if (err) {
				ssi_err ("%s", err);
			}
			ssi_cast2type (1, &result, dstptr++, ssi_type_t (_options.cast));
			srcptr += stream_in.dim;
		}
	} else {
		if (_join == JOIN::OFF) {
			for (ssi_size_t i = 0; i < stream_in.num; i++) {
				for (ssi_size_t j = 0; j < stream_in.dim; j++) {
					insert (_options.expression,_expression, j, srcptr);
					const ssi_char_t *err = _parser->parse (_expression, result);
					if (err) {
						ssi_err ("%s", err);
					}
					ssi_cast2type(1, &result, dstptr++, ssi_type_t(_options.cast));
				}
				srcptr += stream_in.dim;
			}
		} else {
			double tmp;
			for (ssi_size_t i = 0; i < stream_in.num; i++) {
				tmp = _join == JOIN::SUM ? 0.0f : 1.0f;
				for (ssi_size_t j = 0; j < stream_in.dim; j++) {
					insert (_options.expression,_expression, j, srcptr);
					const ssi_char_t *err = _parser->parse (_expression, result);
					if (err) {
						ssi_err ("%s", err);
					}
					switch (_join) {
						case JOIN::SUM:
							tmp += result;
							break;
						case JOIN::MULT:
							tmp *= result;
							break;
					}
				}
				ssi_cast2type(1, &tmp, dstptr++, ssi_type_t(_options.cast));
				srcptr += stream_in.dim;
			}
		}
	}
}

void Expression::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {
}


void Expression::insert (ssi_char_t *src, ssi_char_t *dst, ssi_size_t current, ssi_real_t *sample) {
	
	while (*src != '\0') {
		if (*src == 'd') {
			if (*(src+1) != '\0' && isdigit (*(src+1))) {
				int d = atoi (src+1);
				sprintf (dst, "%f", sample[d]);
				dst = dst + strlen(dst) - 2;
				while (isdigit (*(src+1))) {
					src++;
				}
			} else {
				sprintf (dst, "%f", sample[current]);
				dst = dst + strlen(dst) - 2;
			}
		} else {
			*dst = *src;
		}
		src++;
		dst++;
	}
	*dst = '\0';
}

}
