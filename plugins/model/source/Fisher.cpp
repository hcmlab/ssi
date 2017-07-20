// Fisher.cpp
// author: Amira Elshimy
// created: 2012/03/07
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

#include "Fisher.h"
#include "AlgLibTools.h"
#include "dataanalysis.h"
#include "ioput/file/File.h"
using namespace alglib_impl;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif



namespace ssi{

Fisher::Fisher (const ssi_char_t *file) 
	: _basis (0),
	_file (0),
	_is_build (false) {	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Fisher::~Fisher () {

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool Fisher::build (ISamples &samples, ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isBuild ()) {
		ssi_wrn ("already trained");
		return false;
	}

	ae_state state;
	ae_int_t info; 


	ae_matrix data;
	ae_matrix_init (&data, 0, 0, DT_REAL, &state, ae_true);	
	// convert the samples to a matrix where the last column holds the class number to which the sample belongs
	AlgLibTools::Samples2MatrixWithClass(samples, 0, &data);

	_basis = new ae_matrix;
	ae_matrix_init (_basis, 0, 0, DT_REAL, &state, ae_true);
	fisherldan(&data,data.rows,data.cols-1 , samples.getClassSize(),&info,_basis,&state);

	ae_matrix_clear (&data);

	_is_build = true;

	return true;

}

bool Fisher::transform (ssi_stream_t &stream, ssi_stream_t &stream_t) {	

	ae_state state;

	ae_vector feature_v;
	ae_vector_init (&feature_v, 0, DT_REAL, &state, ae_true);
	AlgLibTools::Stream2vector (stream, &feature_v, &state);

	ae_vector reduced_v;
	ae_vector_init (&reduced_v, _basis->rows, DT_REAL, &state, ae_true);
	
	
	rmatrixmv (_basis->cols, _basis->rows, _basis, 0, 0, 1, &feature_v, 0, &reduced_v, 0, &state);

	ae_int_t cnt = reduced_v.cnt;
	if (_options.n > 0 && ssi_cast (ssi_size_t, reduced_v.cnt) >= _options.n) {
		reduced_v.cnt = _options.n;
	}
	AlgLibTools::Vector2stream (&reduced_v, stream_t, &state);
	reduced_v.cnt = cnt;

	ae_vector_clear (&feature_v);
	ae_vector_clear (&reduced_v);

	return true;
}
bool Fisher::load (const ssi_char_t *filepath) {

	ae_state state;

	release ();

	FILE *file = fopen (filepath, "rb");

	_basis = new ae_matrix;
	
		
	ae_matrix_init (_basis, 0, 0, DT_REAL, &state, ae_true);	

	fread (&_options.n, sizeof (_options.n), 1, file);
	AlgLibTools::Read (file, _basis);
	

	_is_build = true;

	fclose (file);

	return true;
}


bool Fisher::save (const ssi_char_t *filepath) {

	if (!isBuild ()) {
		ssi_wrn ("not build");
		return false;
	}
	
	FILE *file = fopen (filepath, "wb");

	fwrite (&_options.n, sizeof (_options.n), 1, file);
	AlgLibTools::Write (file, _basis);


	fclose (file);

	return true;
}


void Fisher::release () {

	if (_is_build) {
		ae_matrix_clear (_basis);
	
	}
	delete _basis; _basis = 0;
	
	_is_build = false;

}

void Fisher::print (FILE *file) {

	if (isBuild ()) {
		fprintf (file, "base:\n");						
		AlgLibTools::Print (file, _basis);				
		fprintf(file,"Number of dimensions to keep:%i\n",Fisher::getOptions()->n);		
	}
} 


		}
		
