// AdaBoost.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/03/03
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
#include "AdaBoost.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *AdaBoost::ssi_log_name = "adaboost__";

AdaBoost::AdaBoost (const ssi_char_t *file) 
	:	_file (0),
	_n_classes (0),
	_n_streams (0),
	_n_models (0),
	_sample (0),
	_n_samples (0),
	_niteration (0),
	_iterations (0),
	_percent (0),
	_error_default (0),
	_error_rates (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT){

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
} 

AdaBoost::~AdaBoost () { 
	
	release();

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void AdaBoost::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;

	delete _sample;

	delete [] _error_rates;
	_error_rates = 0;
}

bool AdaBoost::select (IModel **models, ISamples &samples, ssi_size_t nstrm){

	//construct intervalls (probability distribution)
	//random seed
	srand ((unsigned int) time(NULL) );
	struct inter_elem {
		ssi_size_t index;
		ssi_real_t from; ssi_real_t to;
	};
	inter_elem *inter = new inter_elem[_n_samples];
	ssi_real_t inter_counter = 0.0f;
	ssi_size_t sample_counter = 0;
	
	samples.reset();
	while( _sample = samples.next() ){
		inter_elem ie;
		ie.index = sample_counter;
		ie.from  = inter_counter;
		ie.to    = inter_counter + _sample->score;
		inter_counter = ie.to;
		inter[sample_counter] = ie;
		sample_counter++;
	}
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		for(ssi_size_t j = 0; j < _n_samples; j++){
			ssi_print("\nElement with Probability %f\t %d\t from %f to %f", samples.get(j)->score, inter[j].index, inter[j].from, inter[j].to);
		}
	}

	//select samples from intervalls
	SampleList samples_s;
	for(ssi_size_t i = 0; i < samples.getClassSize(); i++){
		samples_s.addClassName(samples.getClassName(i));
	}
	for(ssi_size_t i = 0; i < samples.getUserSize(); i++){
		samples_s.addUserName(samples.getUserName(i));
	}
	ssi_size_t toSelect = (ssi_size_t) (ssi_cast(ssi_real_t, _n_samples) / 100) * _percent;
	ssi_size_t selected = 0;
	samples.reset();
	while(selected < toSelect){
		bool sel = false;
		while(!sel){
			ssi_real_t toss = coin();
			for(ssi_size_t j = 0; j < _n_samples; j++){
				if( (toss >= inter[j].from) && (toss < inter[j].to) && (inter[j].index != -1) ){
					if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
						ssi_print("\n%f as Toss selects Intervall Element %d\t from %f to %f", toss, inter[j].index, inter[j].from, inter[j].to);
					}
					samples_s.addSample(samples.get(inter[j].index), true);
					inter[j].index = -1;
					sel = true;
					selected++;
				}
			}
		}
	}
	delete [] inter;
	inter = 0;
	
	//train model with selected samples
	if (models[(_iterations * nstrm) + _niteration]->isTrained ()) {
		models[(_iterations * nstrm) + _niteration]->release();
	}
	models[(_iterations * nstrm) + _niteration]->train(samples_s, nstrm);

	//calculate error for model based on sample->probs
	ssi_real_t* s_probs = new ssi_real_t[_n_classes];
	ssi_real_t error = 0.0f;
	samples.reset();
	while(_sample = samples.next()){
		models[(_iterations * nstrm) + _niteration]->forward(*_sample->streams[nstrm], _n_classes, s_probs);
		ssi_size_t max_ind = 0;
		ssi_real_t max_val = s_probs[0];
		for (ssi_size_t j = 1; j < _n_classes; j++) {
			if (s_probs[j] > max_val) {
				max_val = s_probs[j];
				max_ind = j;
			}
		}
		if ( max_ind != _sample->class_id ){
			error += _sample->score;
		}
	}
	delete [] s_probs;
	s_probs = 0;


	if(error > _error_default){

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\n\n");
			samples_s.printInfo();
			ssi_print("\nError:\t\t\t %f", error);
		}

		ssi_print("\nIteration %d Not Passed ...\n", _niteration);

		return false;
	}

	else
	{
		_error_rates[(_iterations * nstrm) + _niteration] = error/(1-error);
			
		//update probability distribution
		ssi_real_t norm_fac = 0.0f;
		ssi_real_t* s_probs = new ssi_real_t[_n_classes];
		for(ssi_size_t i = 0; i < _n_samples; i++){
			models[(_iterations * nstrm) + _niteration]->forward(*samples.get(i)->streams[nstrm], _n_classes, s_probs);
			ssi_size_t max_ind = 0;
			ssi_real_t max_val = s_probs[0];
			for (ssi_size_t j = 1; j < _n_classes; j++) {
				if (s_probs[j] > max_val) {
					max_val = s_probs[j];
					max_ind = j;
				}
			}
			if ( max_ind == samples.get(i)->class_id ){
				samples.get(i)->score = samples.get(i)->score * _error_rates[(_iterations * nstrm) + _niteration];
			}
			norm_fac += samples.get(i)->score;
		}
		delete [] s_probs;
		s_probs = 0;
		
		ssi_real_t sum = 0.0f;
		for(ssi_size_t i = 0; i < _n_samples; i++){
			samples.get(i)->score = samples.get(i)->score / norm_fac;
			sum += samples.get(i)->score;
		}

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\n\n");
			samples_s.printInfo();
			ssi_print("\nError:\t\t\t %f", error);
			ssi_print("\nError Rate:\t\t %f", error/(1-error));
			ssi_print("\nNormalization Factor:\t %f", norm_fac);
			ssi_print("\nSum:\t\t\t %f", sum);
		}
		
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {	
			ssi_print("\nIteration %d Passed ...\n", _niteration);
		}

		_niteration++;

		return true;
	}
}

bool AdaBoost::train (ssi_size_t n_models,
	IModel **models,
	ISamples &samples) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	/*if (samples.getStreamSize () != 1) {
		ssi_wrn ("boosting does only support one stream");
		return false;
	}*/

	/*if (_options.iter != n_models) {
		ssi_wrn ("incorrect model number");
		return false;
	}*/

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_iterations    = _options.iter;
	_percent       = _options.size;
	_error_default = _options.error;

	_n_models = n_models;
	_n_samples = samples.getSize();
	_n_classes = samples.getClassSize();
	_n_streams = samples.getStreamSize();

	_error_rates = new ssi_real_t[_n_models];
	for(ssi_size_t i = 0; i < _iterations; i++){
		_error_rates[i] = 0.0f;
	}

	SampleList samples_tmp;
	ssi::ModelTools::CopySampleList(samples, samples_tmp);

	for(ssi_size_t nstrm = 0; nstrm < _n_streams; nstrm++){

		//initialize sample probs
		samples_tmp.reset();
		ssi_size_t sample_counter = 0;
		while (sample_counter < _n_samples){
			ssi_real_t p = ssi_cast(ssi_real_t, (1.0f / ssi_cast(ssi_real_t, _n_samples)));
			samples_tmp.get(sample_counter)->score = p;
			sample_counter++;
		}
		
		//do iterations
		_niteration = 0;
		while( _niteration < _iterations ){
			this->select(models, samples_tmp, nstrm);
		}

		//reset sample probs
		samples_tmp.reset();
		sample_counter = 0;
		while (sample_counter < _n_samples){
			samples_tmp.get(sample_counter)->score = 0.0f;
			sample_counter++;
		}
	}

	return true;
}

bool AdaBoost::forward (ssi_size_t n_models,
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
	ssi_stream_t *stream = 0;
	ssi_size_t *votes = new ssi_size_t[n_models];
	ssi_real_t **decision_profile = new ssi_real_t*[n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		decision_profile[n_model] = new ssi_real_t[_n_classes];		
	}

	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		model = models[n_model];
		ssi_size_t strm = n_model / (_n_models / _n_streams);
		stream = streams[strm];
		model->forward (*stream, n_probs, probs);

		//fill decision_profile
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			decision_profile[n_model][num_probs] = probs[num_probs];
		}

		//fill votes
		ssi_size_t max_ind = 0;
		ssi_real_t max_val = probs[0];
		
		for (ssi_size_t i = 1; i < n_probs; i++) {
			if (probs[i] > max_val) {
				max_val = probs[i];
				max_ind = i;
			}
		}
		votes[n_model] = max_ind;

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				ssi_print("%f ", probs[num_probs]);
			}ssi_print("- vote: %d\n", max_ind);

		}

	}
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\n");
	}

	

	switch (_options.combination_rule) {

		case 0:
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nDefault Combination");
			}
			//clear probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				probs[num_probs] = 0.0f;
			}
			//fill probs
			for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
				probs[votes[n_model]] = probs[votes[n_model]] + log((1.0f / _error_rates[n_model]));
			}
			break;

		case 1:
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nProduct Rule");
			}
			//clear probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				probs[num_probs] = 1.0f;
			}
			//fill probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
					probs[num_probs] = probs[num_probs]*decision_profile[n_model][num_probs];
				}
			}
			break;

		case 2:
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nSum Rule");
			}
			//clear probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				probs[num_probs] = 0.0f;
			}
			//fill probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
					probs[num_probs] = probs[num_probs]+decision_profile[n_model][num_probs];
				}
			}
			break;

		case 3:
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nMax Rule");
			}
			//clear probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				probs[num_probs] = 0.0f;
			}
			//fill probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				ssi_real_t max_val = decision_profile[0][num_probs];
				for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
					if(decision_profile[n_model][num_probs] >= max_val){
						max_val = decision_profile[n_model][num_probs];
						probs[num_probs] = max_val;
					}
				}
			}
			break;

		case 4:
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nMean Rule");
			}
			//clear probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				probs[num_probs] = 0.0f;
			}
			//fill probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
					probs[num_probs] = probs[num_probs]+decision_profile[n_model][num_probs];
				}
				probs[num_probs] = probs[num_probs] / ssi_cast(ssi_real_t, n_models);
			}
			break;

		case 5:
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nMin Rule");
			}
			//clear probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				probs[num_probs] = 0.0f;
			}
			//fill probs
			for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
				ssi_real_t min_val = decision_profile[0][num_probs];
				for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
					if(decision_profile[n_model][num_probs] <= min_val){
						min_val = decision_profile[n_model][num_probs];
						probs[num_probs] = min_val;
					}
				}
			}
			break;

		default:
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nDefault Combination");
			}
			for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
				probs[votes[n_model]] = probs[votes[n_model]] + log((1.0f / _error_rates[n_model]));
			}
			break;
	}

	
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {

		ssi_print("\nVotes: ");
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			ssi_print("%f ", probs[num_probs]);
		}ssi_print("\n\n");

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

ssi_real_t AdaBoost::coin(){
	ssi_real_t scale = RAND_MAX+1.;
	ssi_real_t base = rand()/scale;
	ssi_real_t fine = rand()/scale;
	return base + fine / scale;
}

bool AdaBoost::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_options.iter, sizeof (_options.iter), 1);
	file->write (&_options.size, sizeof (_options.size), 1);
	file->write (&_options.error, sizeof (_options.error), 1);
	file->write (&_options.combination_rule, sizeof (_options.combination_rule), 1);
	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);

	file->write (_error_rates, sizeof (ssi_real_t), _n_models);

	delete file;
	
	return true;
}

bool AdaBoost::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_options.iter, sizeof (_options.iter), 1);
	file->read (&_options.size, sizeof (_options.size), 1);
	file->read (&_options.error, sizeof (_options.error), 1);
	file->read (&_options.combination_rule, sizeof (_options.combination_rule), 1);
	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);

	_error_rates = new ssi_real_t [_n_models];
	file->read (_error_rates, sizeof (ssi_real_t), _n_models);

	delete file;
	
	return true;
}

}
