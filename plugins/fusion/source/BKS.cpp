// BKS.cpp
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
#include "BKS.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *BKS::ssi_log_name = "bks_______";

BKS::BKS (const ssi_char_t *file) 
	:	_table (0),
		_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		_table_depth (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){
} 

BKS::~BKS () {
	release();
}

void BKS::release ()
{

	if(_table){
		for (ssi_size_t entry = 0; entry < _table_depth; entry++) {
			delete [] _table[entry];
		}
		delete [] _table;
		_table = 0;
	}

	_table_depth = 0;

	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
}

bool BKS::train (ssi_size_t n_models,
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
	ssi_size_t max_entries = samples.getSize();
	_table_depth = max_entries;

	_table = new ssi_size_t*[max_entries];
	for (ssi_size_t entry = 0; entry < max_entries; entry++) {
		_table[entry] = new ssi_size_t[n_models+2];		
	}
	ssi_real_t *_probs = new ssi_real_t[_n_classes];

	//train models and table
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		if (!models[n_model]->isTrained ()) { models[n_model]->train (samples, n_model); }
		samples.reset ();
		const ssi_sample_t *sample = 0;
		ssi_size_t real_index, sample_index = 0;
		while (sample = samples.next ()) {
			real_index = sample->class_id;
			models[n_model]->forward (*sample->streams[n_model], _n_classes, _probs);
			ssi_size_t max_ind = 0;
			ssi_real_t max_val = _probs[0];
			for (ssi_size_t i = 1; i < _n_classes; i++) {
				if (_probs[i] > max_val) {
					max_val = _probs[i];
					max_ind = i;
				}
			}
			_table[sample_index][n_model] = max_ind;
			_table[sample_index][n_models] = sample->class_id;
			_table[sample_index][n_models+1] = 1;
			sample_index++;
		}
	}
	
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\nOriginal Table:\n");
		printTable();
	}
	for(ssi_size_t i = 0; i < _table_depth; i++){
		for(ssi_size_t j = 0; j < _table_depth; j++){
			if( (i != j) && (checkEntry(_table[i], _table[j], _n_streams+1)) ){
				delEntry(_table, j, i);
				j--;
			}
		}
	}
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\nFinal Table:\n");
		printTable();
	}

	if(_probs){
		delete[] _probs;
		_probs = 0;
	}

	return true;
}

bool BKS::forward (ssi_size_t n_models,
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
	ssi_size_t *votes = 0;;
	votes = new ssi_size_t[n_models];
	
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		model = models[n_model];
		model->forward (*streams[n_model], n_probs, probs);

		ssi_size_t max_ind = 0;
		ssi_real_t max_val = probs[0];
		
		for (ssi_size_t i = 1; i < n_probs; i++) {
			if (probs[i] > max_val) {
				max_val = probs[i];
				max_ind = i;
			}
		}

		votes[n_model] = max_ind;

	}

	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = -1;
	}

	//fill probs with votes
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\nvotes: \n");
		for(ssi_size_t n_model = 0; n_model < n_models; n_model++){
			ssi_print("%d ", votes[n_model]);
		}ssi_print("\n");
	}

	ssi_size_t max_count = 0;
	ssi_size_t candidate_count_entry = 0;
	bool hit = false;
	for(ssi_size_t iDepth = 0; iDepth < _table_depth; iDepth++){

		if(checkEntry(_table[iDepth], votes, n_models)){
			
			hit = true;
			
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nCandidate: ");
				for(ssi_size_t n_model = 0; n_model < n_models+2; n_model++){
					ssi_print("%d ",_table[iDepth][n_model]);
				}ssi_print("\n");
			}
			
			if(_table[iDepth][n_models+1] >= candidate_count_entry){
				candidate_count_entry = _table[iDepth][n_models+1];
				max_count = _table[iDepth][n_models];
			}
		}
	}

	if(votes){
		delete[] votes;
		votes = 0;
	}

	if (hit){
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("Decision: %d\n", max_count);
		}
		probs[max_count] = 1;
		return true;
	}else{ return false; }
}

bool BKS::printTable(){

	ssi_print("\n\nBKS Table:\n\n");
	for(ssi_size_t j = 0; j < _table_depth; j++){
		ssi_print("%d:\t", j);
		for(ssi_size_t k = 0; k <= _n_models+1; k++){
			ssi_print("%d ", _table[j][k]);
		}ssi_print("\n");
	}

	return true;
}

bool BKS::checkEntry(ssi_size_t *_table_entry, ssi_size_t *_new_entry, ssi_size_t check_size){

	for(ssi_size_t index = 0; index < (check_size); index++){
		if(_table_entry[index] != _new_entry[index]){
			return false;
		}
	}
	return true;
}

bool BKS::delEntry(ssi_size_t** _table, ssi_size_t iDelIndex, ssi_size_t iRaiseIndex){

	ssi_size_t iFound = 0;
	ssi_size_t iDelBegin = iDelIndex;
	ssi_size_t iDelEnd = (iDelIndex + 1);

	for(ssi_size_t iTemp = 0; iTemp < (_table_depth); iTemp++){
		if((iTemp < iDelBegin) || (iTemp >= iDelEnd)){
			for(ssi_size_t jTemp = 0; jTemp < (_n_streams+2); jTemp++){
				_table[iTemp - iFound][jTemp] = _table[iTemp][jTemp];
			}
		}else{iFound++;}
	}
	_table[iRaiseIndex][_n_streams+1]++;
	delete [] _table[_table_depth-1];
	_table_depth--;

	return true;
}

bool BKS::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	file->write (&_table_depth, sizeof (_table_depth), 1);
	for (ssi_size_t n_entries = 0; n_entries < _table_depth; n_entries++) {
		file->write (_table[n_entries], sizeof (ssi_size_t), (_n_streams+2));
	}
	
	delete file;
	
	return true;
}

bool BKS::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	file->read (&_table_depth, sizeof (_table_depth), 1);
	_table = new ssi_size_t *[_table_depth];
	for (ssi_size_t n_entries = 0; n_entries < _table_depth; n_entries++) {
		_table[n_entries] = new ssi_size_t[_n_streams+2];
		file->read (_table[n_entries], sizeof (ssi_size_t), (_n_streams+2));
	}
	
	delete file;
	
	return true;
}

}
