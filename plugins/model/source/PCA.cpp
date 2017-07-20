// PCA.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/07
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

#include "PCA.h"
#include "AlgLibTools.h"
#include "dataanalysis.h"
#include "ioput/file/File.h"
#include "math.h"
using namespace alglib_impl;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

PCA::PCA (const ssi_char_t *file) 
	: _variance (0),
	_basis (0),
	_file (0),
	_is_build (false),
	_n(0)
	{	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

PCA::~PCA () {

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool PCA::build (ISamples &samples, ssi_size_t stream_index, ssi_size_t &n_dimension_to_keep) {

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
	AlgLibTools::Samples2matrix (samples, 0, &data, &state);

	
	_basis = new ae_matrix;
	_variance = new ae_vector;
	ae_vector_init (_variance, 0, DT_REAL, &state, ae_true);	
	ae_matrix_init (_basis, 0, 0, DT_REAL, &state, ae_true);	
	pcabuildbasis (&data, data.rows, data.cols, &info, _variance, _basis, &state);

	ae_matrix_clear (&data);

	_is_build = true;

	// to get the n dimensions to keep 
	{
		double totalVar = 0;
		double reqVar = 0;
		double sum = 0;
	
		ssi_size_t i = 0;
	    ssi_size_t dim = (ssi_size_t) _variance->cnt;

		// restricting the number of digits after the point to be 5 digits 
		for(i=0 ; i<dim ;i++){
			_variance->ptr.p_double[i]= ceill((_variance->ptr.p_double[i]*100000))/100000;		
		}

		// summing the variances 
		for(i=0; i<dim ;i++){
			totalVar += _variance->ptr.p_double[i];		  
		}
  
		reqVar = ssi_cast(ssi_real_t, (_options.percentage/100)) * totalVar;
    
		sum = 0; 
		_n = 0;
 
		 // getting the number of features to keep 
		for(i=0; sum<reqVar && i<dim ; i++){
		    sum +=  _variance->ptr.p_double[i];	
			_n++;	
		}
				
		ssi_msg (SSI_LOG_LEVEL_BASIC, "number of dimensions to keep : %i" , _n);	
		n_dimension_to_keep = _n;
	}

	return true;
}

bool PCA::transform (ssi_stream_t &stream, ssi_stream_t &stream_t) {	

	ae_state state;

	ae_vector feature_v;
	ae_vector_init (&feature_v, 0, DT_REAL, &state, ae_true);
	AlgLibTools::Stream2vector (stream, &feature_v, &state);

	ae_vector reduced_v;
	ae_vector_init (&reduced_v, _basis->rows, DT_REAL, &state, ae_true);	
	
	rmatrixmv (_basis->cols, _basis->rows, _basis, 0, 0, 1, &feature_v, 0, &reduced_v, 0, &state);

	ae_int_t cnt = reduced_v.cnt;
	
	if (_n > 0 && ssi_cast (ssi_size_t, reduced_v.cnt) >= _n) {
		reduced_v.cnt = _n;
	}
	AlgLibTools::Vector2stream (&reduced_v, stream_t, &state);
	reduced_v.cnt = cnt;

	ae_vector_clear (&feature_v);
	ae_vector_clear (&reduced_v);

	return true;
}

bool PCA::load (const ssi_char_t *filepath) {

	ae_state state;

	release ();

	FILE *file = fopen (filepath, "rb");

	_basis = new ae_matrix;
	_variance = new ae_vector;
	ae_vector_init (_variance, 0, DT_REAL, &state, ae_true);	
	ae_matrix_init (_basis, 0, 0, DT_REAL, &state, ae_true);	

	fread (&_options.percentage, sizeof (_options.percentage), 1, file);
	fread (&_n, sizeof (_n), 1, file);
	AlgLibTools::Read (file, _basis);
	AlgLibTools::Read (file, _variance);

	_is_build = true;

	fclose (file);

	return true;
}

bool PCA::save (const ssi_char_t *filepath) {

	if (!isBuild ()) {
		ssi_wrn ("not build");
		return false;
	}
	
	FILE *file = fopen (filepath, "wb");

	fwrite (&_options.percentage, sizeof (_options.percentage), 1, file);
	fwrite (&_n, sizeof (_n), 1, file);
	AlgLibTools::Write (file, _basis);
	AlgLibTools::Write (file, _variance);


	fclose (file);

	return true;
}

void PCA::release () {

	if (_is_build) {
		ae_matrix_clear (_basis);
		ae_vector_clear (_variance);
	}
	delete _basis; _basis = 0;
	delete _variance; _variance = 0;

	_is_build = false;
	_n=0;
}

void PCA::print (FILE *file) {

	if (isBuild ()) {
		fprintf (file, "base:\n");						
		AlgLibTools::Print (file, _basis);				
		fprintf (file, "variance:\n");
		AlgLibTools::Print (file, _variance);		
	}
} 

}
