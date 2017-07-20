// LDA.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/12/23
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

#include "LDA.h"
#include "AlgLibTools.h"
#include "statistics.h"
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

namespace ssi {

LDA::LDA (const ssi_char_t *file) 
	: _n_classes (0),
	_n_features (0),
	_covinv (0),
	_meanc (0),
	_min (0),
	_max (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

LDA::~LDA () {

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool LDA::train (ISamples &samples, ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	ae_state state;

	_n_classes = samples.getClassSize ();
	_n_features = samples.getStream (stream_index).dim;

	_covinv = new ae_matrix;
	ae_matrix_init (_covinv, 0, 0, DT_REAL, &state, true);	
	_meanc = new ae_vector *[_n_classes];

	ae_matrix data;
	ae_matrix *datac = new ae_matrix[_n_classes];	
	ae_matrix *covc = new ae_matrix[_n_classes];

	ae_matrix_init (&data, 0, 0, DT_REAL, &state, ae_true);	
	AlgLibTools::Samples2matrix (samples, 0, &data, &state);

	if (_options.scale) {
		AlgLibTools::CreateScaling (&data, &_min, &_max);
		AlgLibTools::ApplyScaling (&data, _min, _max);		
	}

	/*FILE *file = fopen ("data.txt", "w");
	AlgLibTools::Print (file, &data);
	fclose (file);
	ssi_sample_t *sample = 0;
	samples.reset ();
	file = fopen ("labels.txt", "w");
	while (sample = samples.next ()) {
		ssi_fprint (file, "%u\n", sample->class_id);
	}
	fclose (file);
	*/

	for (ssi_size_t c = 0; c < _n_classes; c++) {		

		ae_matrix_init (&datac[c], 0, 0, DT_REAL, &state, ae_true);
		AlgLibTools::Samples2matrix (samples, 0, c, &datac[c], &state);

		if (_options.scale) {			
			AlgLibTools::ApplyScaling (&datac[c], _min, _max);
		}

		_meanc[c] = new ae_vector;
		ae_vector_init (_meanc[c], 0, DT_REAL, &state, ae_true);
		AlgLibTools::Meanm (&datac[c], _meanc[c], &state);

		ae_matrix_init (&covc[c], 0, 0, DT_REAL, &state, ae_true);	
		covm (&datac[c], datac[c].rows, datac[c].cols, &covc[c], &state);

		//ssi_print ("meanc[%u]\n", c);
		//AlgLibTools::Print (ssiout, _meanc[c]);
		//ssi_print ("covc[%u]\n", c);
		//AlgLibTools::Print (ssiout, &covc[c]);
	}

	ae_matrix_init (_covinv, 0, 0, DT_REAL, &state, ae_true);	
	AlgLibTools::Meanms (_n_classes, covc, _covinv, &state);
	//ssi_print ("covinv\n");
	//AlgLibTools::Print (ssiout, _covinv);

	ae_int_t info;
    matinvreport rep;
	spdmatrixinverse (_covinv, _covinv->cols, ae_true, &info, &rep, &state);
	//ssi_print ("pinv (covinv)\n");
	//AlgLibTools::Print (ssiout, _covinv);

	/*FILE *file = fopen ("conv.txt", "w");
	AlgLibTools::Print (file, _covinv);
	fclose (file);*/

	ae_matrix_clear (&data);	
	for (ssi_size_t c = 0; c < _n_classes; c++) {		
		ae_matrix_clear (&datac[c]);	
		ae_matrix_clear (&covc[c]);	
	}	
	delete[] datac;
	delete[] covc;

	return true;
}

bool LDA::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {	

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_probs != _n_classes) {
		ssi_wrn ("#classes differs");
		return false;
	}

	if (stream.type != SSI_REAL) {
		ssi_wrn ("type differs");
		return false;
	}

	if (stream.dim != _n_features) {
		ssi_wrn ("feature dimension differs");
		return false;
	}

	ssi_size_t sample_dimension = stream.dim;
	ssi_size_t sample_number = stream.num;
	
	ae_state state;
	ae_int_t dim = _covinv->cols;
	ae_int_t i = 0;
	ae_int_t j = 0;
	
	ae_vector samplev;
	ae_vector_init (&samplev, 0, DT_REAL, &state, ae_true);
	AlgLibTools::Stream2vector (stream, &samplev, &state);

	if (_options.scale) {
		AlgLibTools::ApplyScaling (&samplev, _min, _max);
	}

	ae_vector tmp, tmp2;
	ae_vector_init (&tmp, 0, DT_REAL, &state, ae_true);
	ae_vector_init (&tmp2, dim, DT_REAL, &state, ae_true);
	double sum = 0;
	double result = 0;
	for (ssi_size_t c = 0; c < n_probs; c++) {
		AlgLibTools::Subv (&tmp, &samplev, _meanc[c], &state);
		//ssi_print ("tmp\n");
		//AlgLibTools::Print (ssiout, &tmp);
		for (i=0; i < dim; i++) {
			sum = 0;
			for (j=0; j < dim; j++) {
				if (i > j) {
					sum += 	tmp.ptr.p_double[j] * _covinv->ptr.pp_double[j][i];
				} else {
					sum += 	tmp.ptr.p_double[j] * _covinv->ptr.pp_double[i][j];
				}
			}
			tmp2.ptr.p_double[i] = sum;
		}
		result = 0;
		for (i=0; i < dim; i++) {
			result += tmp.ptr.p_double[i] * tmp2.ptr.p_double[i];
		}
		probs[c] = - ssi_cast (ssi_real_t, result);
	}

	if (_options.norm) {
		ssi_real_t sum = 0;
		for (ssi_size_t j = 0; j < n_probs; j++) {
			sum += probs[j];
		}
		for (ssi_size_t j = 0; j < n_probs; j++) {
			probs[j] = 1.0f - (probs[j] / sum);
		}
	}

	ae_vector_clear (&samplev);
	ae_vector_clear (&tmp);
	ae_vector_clear (&tmp2);

	return true;
}

bool LDA::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);
	
	file->read (&_options.norm, sizeof (_options.norm), 1);
	file->read (&_options.scale, sizeof (_options.scale), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_features, sizeof (_n_features), 1);

	ae_state state;

	_covinv = new ae_matrix;
	ae_matrix_init (_covinv, _n_features, _n_features, DT_REAL, &state, true);	
	_meanc = new ae_vector *[_n_classes];

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_meanc[i] = new ae_vector;
		ae_vector_init (_meanc[i], _n_features, DT_REAL, &state, true);	
		file->read (_meanc[i]->ptr.p_double, sizeof (double), _n_features);
	}
	for (ssi_size_t i = 0; i < _n_features; i++) {
		file->read (_covinv->ptr.pp_double[i], sizeof (double), _n_features);
	}

	if (_options.scale) {
		_min = new double[_n_features];
		file->read (_min, sizeof (double), _n_features);
		_max = new double[_n_features];
		file->read (_max, sizeof (double), _n_features);
	}

	return true;
}

bool LDA::save (const ssi_char_t *filepath) {

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}
	
	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&_options.norm, sizeof (_options.norm), 1);
	file->write (&_options.scale, sizeof (_options.scale), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_features, sizeof (_n_features), 1);

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		file->write (_meanc[i]->ptr.p_double, sizeof (double), _n_features);
	}
	for (ssi_size_t i = 0; i < _n_features; i++) {
		file->write (_covinv->ptr.pp_double[i], sizeof (double), _n_features);
	}

	if (_options.scale) {
		file->write (_min, sizeof (double), _n_features);
		file->write (_max, sizeof (double), _n_features);
	}

	delete file;

	return true;
}

void LDA::release () {

	for (ssi_size_t c = 0; c < _n_classes; c++) {		
		if (_meanc[c]) {
			ae_vector_clear (_meanc[c]);
		}
		delete _meanc[c];
	}	
	delete[] _meanc;
	_meanc = 0;
	if (_covinv) {
		ae_matrix_clear (_covinv);
	}
	delete _covinv;
	_covinv = 0;

	delete[] _min;
	_min = 0;
	delete[] _max;
	_max = 0;

	_n_classes = 0;
	_n_features = 0;
}

void LDA::print (FILE *file) {

	if (isTrained ()) {

		for (ssi_size_t i = 0; i < _n_classes; i++) {			
			fprintf (file, "class-mean#%d:\n", i);				
			AlgLibTools::Print (file, _meanc[i]);
					
		}		
		fprintf (file, "inverse-mean-covariance:\n");
		AlgLibTools::Print (file, _covinv);		
	}
} 


}
