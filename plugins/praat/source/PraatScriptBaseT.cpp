// PraatScriptBaseT.cpp
// author: Andreas Seiderer
// created: 2013/09/16
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
// version 3 of the License, or any later version.
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

#include "PraatScriptBaseT.h"
#include "PraatTools.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	char *PraatScriptBaseT::ssi_log_name = "praat____t";

	PraatScriptBaseT::PraatScriptBaseT (const ssi_char_t *file)
		: ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
		_ready (false),
		_parser (0),
		_file (0){

		if (file) {
			if (!OptionList::LoadXML (file, _options)) {
				OptionList::SaveXML (file, _options);
			}
			_file = ssi_strcpy (file);
		}
	}

	PraatScriptBaseT::~PraatScriptBaseT () {

		if (_file) {
			OptionList::SaveXML (_file, _options);
			delete[] _file;
		}
	}

	void PraatScriptBaseT:: transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {		

		_ready = false;

		if (!(_parser = getParser ())) {
			ssi_wrn ("could not initialize parser");	
			return;
		}
		if (!ssi_exists (_options.script)) {
			ssi_wrn ("script not found '%s'", _options.script);
			return;
		}
		if (!ssi_exists (_options.exe)) {
			ssi_wrn ("exe not found '%s'", _options.exe);
			return;
		}

		_ready = true;
	}

	void PraatScriptBaseT::transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		if (!_ready) {
			return;
		}

		std::string praatOutput = PraatTools::RunPraat (stream_in, _options.exe, _options.script, _options.script_args, _options.tmpwav);

		if (_parser->parseValues (praatOutput)) {
			
			ssi_size_t n_values = _parser->getNumberOfValues ();
			
			if (stream_out.dim != n_values) {
				ssi_wrn ("dimensions do not match (%u != %u)", stream_out.dim, n_values);
			} else {

				ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream_out.ptr);
				for (ssi_size_t i = 0; i < stream_out.dim; i++) { 
					*ptr++ = _parser->getValue (i);			
				}

				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sucessfully parsed \"praat\" result");		
			}
		
		} else {
			ssi_wrn ("could not parse praat result");
		}
		
	}

	void PraatScriptBaseT::transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

		_ready = false;
	}

}
