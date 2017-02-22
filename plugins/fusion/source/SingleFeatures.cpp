// SingleFeatures.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/09/26
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
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

#include "SingleFeatures.h"
#include "ISSelectDim.h"

//#include <vld.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *SingleFeatures::ssi_log_name = "singlef___";

SingleFeatures::SingleFeatures (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		_sample (0),
		_n_samples (0),
		_n_feats (0),
		_error_rates (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){

			if (file) {
				if (!OptionList::LoadXML (file, _options)) {
					OptionList::SaveXML (file, _options);
				}
				_file = ssi_strcpy (file);
			}
} 

SingleFeatures::~SingleFeatures () { 
	
	release();

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void SingleFeatures::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models = 0;
	_n_samples = 0;

	delete _sample;

	if ( _n_feats != 0){
		delete [] _n_feats;
		_n_feats = 0;
	}

	if ( _error_rates != 0){
		delete [] _error_rates;
		_error_rates = 0;
	}

}

bool SingleFeatures::train (ssi_size_t n_models,
	IModel **models,
	ISamples &samples) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_models = n_models;
	_n_samples = samples.getSize();
	_n_classes = samples.getClassSize();
	_n_streams = samples.getStreamSize();

	_n_feats = new ssi_size_t[_n_streams];

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\n\n");
		ModelTools::PrintInfo(samples);
		ssi_print("\n\n");
	}
	
	samples.reset();
	for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){
		_n_feats[nstrm] = samples.get(0)->streams[nstrm]->dim;
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\n#feats at stream#%d: %d", nstrm, _n_feats[nstrm]);
		}
	}
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\n\n");
	}
	
	_error_rates = new ssi_real_t [_n_models];
	for(ssi_size_t i = 0; i < _n_models; i++){
		_error_rates[i] = 0.0f;
	}

	SampleList samples_tmp;
	ssi::ModelTools::CopySampleList(samples, samples_tmp);
	//initialize sample probs
	samples_tmp.reset();
	ssi_size_t sample_counter = 0;
	while (sample_counter < _n_samples){
		ssi_real_t p = ssi_cast(ssi_real_t, (1.0f / ssi_cast(ssi_real_t, _n_samples)));
		samples_tmp.get(sample_counter)->score = p;
		sample_counter++;
	}

	//train models
	ssi_size_t pos = 0;
	ssi_size_t *dims = new ssi_size_t[1];
	for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){
		for(ssi_size_t nfeat = 0; nfeat < _n_feats[nstrm]; nfeat++){
			dims[0] = nfeat;

			ISSelectDim samples_sel (&samples_tmp);
			samples_sel.setSelection(nstrm, 1, dims);

			if (models[pos]->isTrained()) {	models[pos]->release(); }
			models[pos]->train(samples_sel, nstrm);

			pos++;
		}
	}
	//calc weights
	pos = 0;
	for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){
		for(ssi_size_t nfeat = 0; nfeat < _n_feats[nstrm]; nfeat++){
			dims[0] = nfeat;

			ISSelectDim samples_sel (&samples_tmp);
			samples_sel.setSelection(nstrm, 1, dims);
			
			ssi_real_t* f_probs = new ssi_real_t[_n_classes];
			ssi_real_t error = 0.0f;
			samples_sel.reset();
			while(_sample = samples_sel.next()){
				models[pos]->forward(*_sample->streams[nstrm], _n_classes, f_probs);
				ssi_size_t max_ind = 0;
				ssi_real_t max_val = f_probs[0];
				for (ssi_size_t j = 1; j < _n_classes; j++) {
					if (f_probs[j] > max_val) {
						max_val = f_probs[j];
						max_ind = j;
					}
				}
				if ( max_ind != _sample->class_id ){
					error += _sample->score;
				}
			}

			_error_rates[pos] = error;

			pos++;

			//clean up
			if (f_probs != 0){
				delete [] f_probs;
				f_probs = 0;
			}
		}
	}
	//clean up
	if (dims != 0){
		delete [] dims;
		dims = 0;
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		for(ssi_size_t i = 0; i < _n_models; i++){
			ssi_print("\nError of Classifier for Feature %d:\t%f ", i, _error_rates[i]);
		}
	}

	return true;
}

bool SingleFeatures::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t *streams[],
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (n_streams != _n_streams) {
		ssi_wrn ("#streams (%u) differs from #streams (%u)", n_streams, _n_streams);
		return false;
	}

	if (_n_classes != n_probs) {
		ssi_wrn ("#probs (%u) differs from #classes (%u)", n_probs ,_n_classes);
		return false;
	}

	IModel *model = 0;
	const ssi_stream_t *stream = 0;
	ssi_stream_t stream_s;
	ssi_size_t *votes = new ssi_size_t[n_models];
	ssi_real_t **decision_profile = new ssi_real_t*[n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		decision_profile[n_model] = new ssi_real_t[_n_classes];		
	}

	ssi_size_t pos = 0;
	ssi_size_t *dims = new ssi_size_t[1];
	for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){
		for(ssi_size_t nfeat = 0; nfeat < _n_feats[nstrm]; nfeat++){
			
			dims[0] = nfeat;

			model = models[pos];
			stream = streams[nstrm];
			
			ssi_stream_select(*stream, stream_s, 1, dims);
			model->forward (stream_s, n_probs, probs);

			//fill decision_profile
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				decision_profile[pos][num_probs] = probs[num_probs];
			}
			
			pos++;
			ssi_stream_destroy(stream_s);
		}
	}
	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = 0.0f;
	}
	//fill probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
			probs[num_probs] = probs[num_probs] + (decision_profile[n_model][num_probs] * (1.0f - _error_rates[n_model]));
		}
		probs[num_probs] = probs[num_probs] / ssi_cast(ssi_real_t, n_models);
	}
	//print
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		print_DP(decision_profile, probs);
	}
	//clean up
	if (dims != 0){
		delete [] dims;
		dims = 0;
	}
	if(votes){
		delete[] votes;
		votes = 0;
	}
	if(decision_profile){
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete [] decision_profile[n_model];
		}
		delete[] decision_profile;
		decision_profile = 0;
	}

	/// is there a draw ? ///
	ssi_size_t max_ind = 0;
	ssi_size_t max_ind_draw = 0;
	ssi_real_t max_val = probs[0];
	bool draw = false;

	for (ssi_size_t i = 1; i < n_probs; i++) {
		if (probs[i] >= max_val) {
			if(probs[i] == max_val){
				draw = true;
				max_ind_draw = i;
			}
			max_val = probs[i];
			max_ind = i;
		}
	}
	
	if(draw && (max_ind == max_ind_draw)){
		return false;
	}else{
		return true;
	}
}

void SingleFeatures::print_DP(ssi_real_t ** decision_profile, ssi_real_t* probs){

	ssi_print("DP:\n");
	for (ssi_size_t n_model = 0; n_model < _n_models; n_model++){
		for (ssi_size_t num_probs = 0; num_probs < _n_classes; num_probs++){
			ssi_print("%f ", decision_profile[n_model][num_probs]);
		}ssi_print("\n");
	}ssi_print("\n");
	ssi_print("Combination:\n");
	for (ssi_size_t num_probs = 0; num_probs < _n_classes; num_probs++){
		ssi_print("%f ", probs[num_probs]);
	}ssi_print("\n\n");

}

bool SingleFeatures::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_options.iter, sizeof (_options.iter), 1);
		
	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	file->write (&_n_samples, sizeof (_n_samples), 1);

	file->write (_n_feats, sizeof (ssi_size_t), _n_streams);

	file->write (_error_rates, sizeof (ssi_real_t), _n_models);

	delete file;
	
	return true;
}

bool SingleFeatures::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_options.iter, sizeof (_options.iter), 1);
		
	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	file->read (&_n_samples, sizeof (_n_samples), 1);

	_n_feats = new ssi_size_t [_n_streams];
	file->read (_n_feats, sizeof (ssi_size_t), _n_streams);

	_error_rates = new ssi_real_t [_n_models];
	file->read (_error_rates, sizeof (ssi_real_t), _n_models);

	delete file;
	
	return true;
}

}
