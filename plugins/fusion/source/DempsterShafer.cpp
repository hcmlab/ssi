// DempsterShafer.cpp
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
#include "DempsterShafer.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *DempsterShafer::ssi_log_name = "dempshaf__";

DempsterShafer::DempsterShafer (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		_proximities (0),
		_decision_template (0),
		_decision_profile (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

DempsterShafer::~DempsterShafer () {
	release();
}

void DempsterShafer::release ()
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
	if (_proximities) {
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete[] _proximities[n_model];
		}
		delete[] _proximities;
		_proximities = 0;
	}

	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
}

bool DempsterShafer::train (ssi_size_t n_models,
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
	ssi_size_t* sample_cardinality = new ssi_size_t[_n_classes];
	//initialize with zeros
	for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
		sample_cardinality[n_class] = 0;
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
		sample_cardinality[c_id] = sample_cardinality[c_id]++;
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

	if(_probs){
		delete [] _probs;
		_probs = 0;
	}
	if(sample_cardinality){
		delete [] sample_cardinality;
		sample_cardinality = 0;
	}
		
	return true;
}

bool DempsterShafer::forward (ssi_size_t n_models,
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
	
	//Proximities between unknown sample's DP and #(_n_classes) DTs from training
	_proximities = new ssi_real_t*[n_models];
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		_proximities[n_model] = new ssi_real_t[_n_classes];
		//initialize with zeros
		for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
			_proximities[n_model][n_class] = 0;
		}
	}
	
	//printDT();
		
	//CALC Proximities
	// ||p - q|| = sqrt( ||p||^2 + ||q||^2 - 2(p*q) ), with * is dotProd
	ssi_real_t p  = 0;
	ssi_real_t q  = 0;
	ssi_real_t pq = 0;
	ssi_real_t num = 0.0f;
	ssi_real_t denom = 0.0f;
	for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
		//ssi_print("\n");
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {

			//num
			p   = dotProd(_decision_template[n_class][n_model], _decision_template[n_class][n_model], _n_classes);
			q   = dotProd(_decision_profile[n_model], _decision_profile[n_model], _n_classes);
			pq  = dotProd(_decision_template[n_class][n_model], _decision_profile[n_model], _n_classes);
			num = pow( ( 1.0f + (p + q - 2*pq) ), -1.0f );
			//ssi_print("num:\t%f\n", num);

			//denom
			denom = 0.0f;
			for (ssi_size_t n_class2 = 0; n_class2 < _n_classes; n_class2++) {
				p   = dotProd(_decision_template[n_class2][n_model], _decision_template[n_class2][n_model], _n_classes);
				q   = dotProd(_decision_profile[n_model], _decision_profile[n_model], _n_classes);
				pq  = dotProd(_decision_template[n_class2][n_model], _decision_profile[n_model], _n_classes);
				ssi_real_t tmp = pow( ( 1.0f + (p + q - 2*pq) ), -1.0f );
				denom = denom + tmp;
			}
			//ssi_print("denom:\t%f\n", denom);

			//proximity
			_proximities[n_model][n_class] = num / denom;
			//ssi_print("prox:\t%f\n\n", _proximities[n_model][n_class]);

		}
	}

	//Calc Belief for each classifier
	ssi_real_t **belief = new ssi_real_t*[n_models];
	for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
		belief[n_model] = new ssi_real_t[_n_classes];
		for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
			ssi_real_t mult_term = 1;
			for(ssi_size_t k = 0; k < _n_classes; k++){
				if(k != n_class){
					mult_term = mult_term * ( 1.0f - _proximities[n_model][k] );
				}
			}
			belief[n_model][n_class] = ( _proximities[n_model][n_class] * mult_term ) / ( 1.0f - ( _proximities[n_model][n_class] * ( 1.0f - mult_term ) ) );
			//ssi_print("\nbelief in model %d, class %d: %f", n_model, n_class, belief[n_model][n_class]);
		}
	}

	ssi_real_t min, max;
	min = belief[0][0];
	max = belief[0][0];
	for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			if(belief[n_model][n_class] > max){ max = belief[n_model][n_class];}
			if(belief[n_model][n_class] < min){ min = belief[n_model][n_class];}
		}
	}
	for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			belief[n_model][n_class] = (belief[n_model][n_class] - min) * ((1.0f - 0.0f) / (max - min)) + 0.0f;
		}
	}
	/*for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
		for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
			ssi_print("\nbelief:\t%f\n", belief[n_model][n_class]);
		}
	}*/

	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = 0;
	}

	//ssi_print("\n\nDempster Combination: \n\n");
	for(ssi_size_t n_class = 0; n_class < _n_classes; n_class++){
		ssi_real_t support = 1;
		for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
			support = support * belief[n_model][n_class];
		}
		//probs[n_class] = norm_const[n_class] * support;
		probs[n_class] = support;
		//ssi_print("%f ", probs[n_class]);
	}//ssi_print("\n\n");/*getchar();*/

	//clean up
	if(_decision_profile){
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete [] _decision_profile[n_model];
		}
		delete[] _decision_profile;
		_decision_profile = 0;
	}
	if (_proximities) {
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete[] _proximities[n_model];
		}
		delete[] _proximities;
		_proximities = 0;
	}
	if (belief) {
		for (ssi_size_t n_model = 0; n_model < _n_models; n_model++) {
			delete[] belief[n_model];
		}
		delete[] belief;
		belief = 0;
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

ssi_real_t DempsterShafer::dotProd(ssi_real_t *arrayA, ssi_real_t *arrayB, ssi_size_t length){
	
	ssi_real_t prod = 0;
	ssi_real_t temp = 0;

	for(ssi_size_t i = 0; i < length; i++){
		temp = arrayA[i] * arrayB[i];
		prod = prod + temp;
	}

	return prod;
}
bool DempsterShafer::printDT(){

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

bool DempsterShafer::save (const ssi_char_t *filepath) {

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

bool DempsterShafer::load (const ssi_char_t *filepath) {

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
