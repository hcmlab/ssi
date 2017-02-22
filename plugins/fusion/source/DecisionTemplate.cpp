// DecisionTemplate.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/4/6
// Copyright (C) 2007-12 University of Augsburg, Florian Lingenfelser
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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
#include "DecisionTemplate.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *DecisionTemplate::ssi_log_name = "dectemp___";

DecisionTemplate::DecisionTemplate (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		_decision_template (0),
		_decision_profile (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

DecisionTemplate::~DecisionTemplate () { 
	release();
}

void DecisionTemplate::release ()
{
	if(_decision_profile){
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete [] _decision_profile[n_model];
		}
		delete[] _decision_profile;
		_decision_profile = 0;
	}

	if(_decision_template){
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
			for(ssi_size_t n_model = 0; n_model < _n_models; n_model++){
				delete [] _decision_template[n_class][n_model];
			}
			delete [] _decision_template[n_class];
		}
		delete[] _decision_template;
		_decision_template = 0;
	}

	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
}

bool DecisionTemplate::train (ssi_size_t n_models,
	IModel **models,
	ISamples &samples) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (samples.getStreamSize () != n_models) {
		ssi_wrn ("#models (%u) differs from #streams (%u)", n_models, samples.getStreamSize ());
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_streams = samples.getStreamSize ();
	_n_classes = samples.getClassSize ();
	_n_models  = n_models;
	
	const ssi_sample_t *sample = 0;
	ssi_real_t* _probs = new ssi_real_t[_n_classes];
	ssi_real_t* sample_cardinality = new ssi_real_t[_n_classes];
	//initialize with zeros
	for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
		sample_cardinality[n_class] = 0.0f;
	}
	_decision_template = new ssi_real_t**[_n_classes];
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
		_decision_template[n_class] = new ssi_real_t*[n_models];
		for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
			_decision_template[n_class][n_model] = new ssi_real_t[_n_classes];
			//initialize with zeros
			for (ssi_size_t n = 0; n < _n_classes; n++){
				_decision_template[n_class][n_model][n] = 0;
			}
		}
	}
	
	//determine sample cardinality
	samples.reset ();
	ssi_size_t c_id = 0;
	while (sample = samples.next ()) {
		c_id = sample->class_id;
		sample_cardinality[c_id] = sample_cardinality[c_id] + 1.0f;
	}
	//train models and build decision template
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		if (!models[n_model]->isTrained ()) { models[n_model]->train (samples, n_model); }
		samples.reset ();
		ssi_size_t c_id = 0;
		while (sample = samples.next ()) {
			models[n_model]->forward (*sample->streams[n_model], _n_classes, _probs);
			c_id = sample->class_id;
			for(ssi_size_t n_prob = 0; n_prob < _n_classes; n_prob++)
			{
				_decision_template[c_id][n_model][n_prob] = _decision_template[c_id][n_model][n_prob] + (_probs[n_prob] / sample_cardinality[c_id]);
				//_decision_template[c_id][n_model][n_prob] = _decision_template[c_id][n_model][n_prob] + _probs[n_prob];
			}
		}
	}
	/*for(ssi_size_t iclass = 0; iclass < _n_classes; iclass++){
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++){
			for(ssi_size_t n_prob = 0; n_prob < _n_classes; n_prob++){
				_decision_template[iclass][n_model][n_prob] = _decision_template[iclass][n_model][n_prob] / sample_cardinality[iclass];
			}
		}
	}*/


	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		printDT();
	}

	if(_decision_profile){
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete [] _decision_profile[n_model];
		}
		delete[] _decision_profile;
		_decision_profile = 0;
	}
	if(sample_cardinality){
		delete [] sample_cardinality;
		sample_cardinality = 0;
	}
	if(_probs){
		delete [] _probs;
		_probs = 0;
	}
	
	return true;
}

bool DecisionTemplate::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t *streams[],
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (n_streams != _n_streams) {
		ssi_wrn ("#streams (%u) differs from #streams (%u)", n_streams, _n_streams);
		return false;
	}

	if (_n_streams != n_models) {
		ssi_wrn ("#models (%u) differs from #streams (%u)", n_models, _n_streams);
		return false;
	}

	if (_n_classes != n_probs) {
		ssi_wrn ("#probs (%u) differs from #classes (%u)", n_probs ,_n_classes);
		return false;
	}

	IModel *model = 0;
	
	//Decision Profile for unknown sample
	_decision_profile = new ssi_real_t*[n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		_decision_profile[n_model] = new ssi_real_t[_n_classes];
		//initialize with zeros
		for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
			_decision_profile[n_model][n_class] = 0;
		}
	}
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		model = models[n_model];
		model->forward (*streams[n_model], n_probs, probs);

		//fill decision_profile DP
		for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
			_decision_profile[n_model][num_probs] = probs[num_probs];
		}	
	}
	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = -1;
	}

	for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
		probs[n_class] = similarityDT(_decision_template[n_class], _decision_profile);
	}

	if(_decision_profile){
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete [] _decision_profile[n_model];
		}
		delete[] _decision_profile;
		_decision_profile = 0;
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

ssi_real_t DecisionTemplate::similarityDT(ssi_real_t **dt, ssi_real_t **dp){

	ssi_real_t similarity = 0;
	ssi_real_t temp = 0;

	for(ssi_size_t n_model = 0; n_model < _n_models; n_model++){
		for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
			temp = dt[n_model][n_class] - dp[n_model][n_class];
			temp = temp*temp;
			similarity = similarity + temp;
		}
	}
	
	similarity = 1.0f - sqrt(similarity);

	return similarity;
}

bool DecisionTemplate::printDT(){

	ssi_print("\n\nDT:\n\n");
	for(ssi_size_t i = 0; i < _n_classes; i++){
		for(ssi_size_t j = 0; j < _n_models; j++){
			for(ssi_size_t k = 0; k < _n_classes; k++){
				ssi_print("%f ", _decision_template[i][j][k]);
			}ssi_print("\n");
		}ssi_print("\n");
	}

	return true;
}

bool DecisionTemplate::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);

	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			file->write (_decision_template[n_class][n_model], sizeof (ssi_real_t), _n_classes);
		}
	}
	
	delete file;
	
	return true;
}

bool DecisionTemplate::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	
	_decision_template = new ssi_real_t **[_n_classes];
	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		_decision_template[n_class] = new ssi_real_t*[_n_models];
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			_decision_template[n_class][n_model] = new ssi_real_t[_n_classes];
			file->read (_decision_template[n_class][n_model], sizeof (ssi_real_t), _n_classes);
		}
	}
	
	delete file;
	
	return true;
}

}
