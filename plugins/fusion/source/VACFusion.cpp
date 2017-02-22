// VACFusion.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2011/1/27
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
#include "VACFusion.h"
#include "Evaluation.h"
#include "ISMissingData.h"
#include "ISHotClass.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	ssi_char_t *VACFusion::ssi_log_name = "vac_______";

VACFusion::VACFusion (const ssi_char_t *file) 
	:	_file (0),
		_n_classes (0),
		_n_streams (0),
		_n_models (0),
		_n_modalities (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT){

} 

VACFusion::~VACFusion () { 
	release();
}

void VACFusion::release ()
{
	_n_classes = 0;
	_n_streams = 0;
	_n_models  = 0;
	_n_modalities = 0;
}

bool VACFusion::train (ssi_size_t n_models,
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

	if (samples.getClassSize() != 4) {
		ssi_wrn ("wrong number of classes (%u)", _n_classes);
		return false;
	}

	_n_streams = samples.getStreamSize ();
	_n_classes = samples.getClassSize ();
	_n_models  = n_models;
	_n_modalities = _n_streams / 4;

	Evaluation eval;

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\n\nSAMPLES Properties:\n");
		ssi_print("\nClasses:\n");
		for(ssi_size_t nclass = 0; nclass < _n_classes; nclass++){
			ssi_print("%d: %s\n", nclass, samples.getClassName(nclass));			
		}ssi_print("\n");
	}

	if (samples.hasMissingData ()) {
		ISMissingData samples_h (&samples);
		ssi_size_t position = 0;
		for(ssi_size_t nmod = 0; nmod < _n_modalities; nmod++){
			samples_h.setStream(position);
			if (!models[position]->isTrained ()) {
				models[position]->train(samples_h, position);
			}
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nDirect Classification: \n\n");
				eval.eval(*models[position], samples_h, position);
				eval.print();
			}
			position++;
			for(ssi_size_t comb = 1; comb <= 3; comb++){
				ISHotClass hot (&samples_h);
				ssi_size_t hotties[] = { 0, comb };
				samples_h.setStream(position);
				hot.setHotClass(2, hotties, "hot");
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print("\nCombination: %d & %d\n\n", 0, comb);
					ssi::ModelTools::PrintInfo(hot);
					ssi_print("\n");
				}
				if (!models[position]->isTrained ()) {
					models[position]->train(hot, position);
				}
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					eval.eval(*models[position], hot, position);
					eval.print();
				}
				position++;
			}
		}
	}else{
		ssi_size_t position = 0;
		for(ssi_size_t nmod = 0; nmod < _n_modalities; nmod++){
			if (!models[position]->isTrained ()) {
				models[position]->train(samples, position);
			}
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nDirect Classification: \n\n");
				eval.eval(*models[position], samples, position);
				eval.print();
			}
			position++;
			for(ssi_size_t comb = 1; comb <= 3; comb++){
				ISHotClass hot (&samples);
				ssi_size_t hotties[] = { 0, comb };
				hot.setHotClass(2, hotties, "hot");
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print("\nCombination: %d & %d\n\n", 0, comb);
					ssi::ModelTools::PrintInfo(hot);
					ssi_print("\n");
				}
				if (!models[position]->isTrained ()) {
					models[position]->train(hot, position);
				}
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					eval.eval(*models[position], hot, position);
					eval.print();
				}
				position++;
			}
		}
	}

	return true;

}

bool VACFusion::forward (ssi_size_t n_models,
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

	if (_n_classes != 4) {
		ssi_wrn ("wrong number of classes (%u)", _n_classes);
		return false;
	}

	IModel			*model = 0;
	ssi_stream_t	*stream = 0;

	ssi_size_t max_ind = 0;
	ssi_real_t max_val = 0.0f;

	bool found_data = false;

	//prepare probs
	ssi_real_t ***all_probs_hot = new ssi_real_t **[3];
	for(ssi_size_t ind = 0; ind < 3; ind++){
		all_probs_hot[ind] = new ssi_real_t *[_n_modalities];
		for(ssi_size_t ind2 = 0; ind2 < _n_modalities; ind2++){
			all_probs_hot[ind][ind2] = new ssi_real_t [2];
		}
	}
	ssi_real_t **all_probs_direct = new ssi_real_t*[_n_modalities];
	for(ssi_size_t ind = 0; ind < _n_modalities; ind++){
		all_probs_direct[ind] = new ssi_real_t[_n_classes];
	}
	
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\n\n-----------------------------\n\n");
	}
	
	ssi_size_t position = 0;
	for(ssi_size_t nmod = 0; nmod < _n_modalities; nmod++){
				
		ssi_real_t *direct_probs = new ssi_real_t[_n_classes];
		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print("\nDirect Classification: \n\n");
		}
		model = models[position];
		stream = streams[position];
		if(stream->num > 0){
			found_data = true;
			model->forward(*stream, _n_classes, direct_probs);
			for(ssi_size_t nprob = 0; nprob < _n_classes; nprob++){
				all_probs_direct[nmod][nprob] = direct_probs[nprob];
			}
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("Probs:\t");
				for(ssi_size_t nprob = 0; nprob < _n_classes; nprob++){
					ssi_print("%f ", all_probs_direct[nmod][nprob]);
				}
				ssi_print("\n");
			}
		}else{
			for(ssi_size_t nprob = 0; nprob < _n_classes; nprob++){
				all_probs_direct[nmod][nprob] = -1.0f;
			}
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("Probs:\t");
				for(ssi_size_t nprob = 0; nprob < _n_classes; nprob++){
					ssi_print("%f ", all_probs_direct[nmod][nprob]);
				}
				ssi_print("\n");
			}
		}
		position++;
		delete [] direct_probs;

		ssi_size_t combo = 0;
		for(ssi_size_t comb = 1; comb <= 3; comb++){
			ssi_real_t *hot_probs = new ssi_real_t[2];
			ssi_size_t hotties[] = { 0, comb };
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print("\nCombination: %d & %d\n\n", 0, comb);
			}
			model = models[position];
			stream = streams[position];
			if(stream->num > 0){
				model->forward(*stream, 2, hot_probs);
				for(ssi_size_t nprob = 0; nprob < 2; nprob++){
					all_probs_hot[combo][nmod][nprob] = hot_probs[nprob];
				}
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print("Probs:\t");
					for(ssi_size_t nprob = 0; nprob < 2; nprob++){
						ssi_print("%f ", all_probs_hot[combo][nmod][nprob]);
					}
					ssi_print("\n");
				}
			}else{
				for(ssi_size_t nprob = 0; nprob < 2; nprob++){
					all_probs_hot[combo][nmod][nprob] = -1.0f;
				}
				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print("Probs:\t");
					for(ssi_size_t nprob = 0; nprob < 2; nprob++){
						ssi_print("%f ", all_probs_hot[combo][nmod][nprob]);
					}
					ssi_print("\n");
				}
			}
			position++;
			combo++;
			delete [] hot_probs;
		}
					
	}

	//clear probs
	ssi_size_t decision = 0;
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = 0.0f;
	}

	//Step1
	for(ssi_size_t comb = 1; comb <= 3; comb++){
		decision = productRule(all_probs_hot[comb - 1], 2);
		if( comb == 1 ){
			if( decision == 0 ){
				probs[0] += 1.0f;
				probs[1] += 1.0f;
			}else{
				probs[2] += 1.0f;
				probs[3] += 1.0f;
			}
		}
		if( comb == 2 ){
			if( decision == 0 ){
				probs[0] += 1.0f;
				probs[2] += 1.0f;
			}else{
				probs[1] += 1.0f;
				probs[3] += 1.0f;
			}
		}
		if( comb == 3 ){
			if( decision == 0 ){
				probs[0] += 1.0f;
				probs[3] += 1.0f;
			}else{
				probs[1] += 1.0f;
				probs[2] += 1.0f;
			}
		}
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\n\nCombination Steps:\n\n");
		ssi_print("\nCombo Votes:\t");
		for(ssi_size_t nprob = 0; nprob < _n_classes; nprob++){
			ssi_print("%f ", probs[nprob]);
		}
		ssi_print("\n");
	}
	//check
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		if (probs[num_probs] == 3.0f) {
			//clean up
			if(all_probs_hot){
				for(ssi_size_t ind = 0; ind < 3; ind++){
					for(ssi_size_t ind2 = 0; ind2 < _n_modalities; ind2++){
						delete [] all_probs_hot[ind][ind2];
					}
					delete [] all_probs_hot[ind];
				}
				delete [] all_probs_hot;
				all_probs_hot = 0;
			}
			if(all_probs_direct){
				for(ssi_size_t ind = 0; ind < _n_modalities; ind++){
					delete [] all_probs_direct[ind];
				}
				delete [] all_probs_direct;
				all_probs_direct = 0;
			}
			return found_data;
		}
	}

	//Step2
	decision = productRule(all_probs_direct, _n_classes);

	probs[decision] += 1.0f;

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\nDirect Votes added:\t");
		for(ssi_size_t nprob = 0; nprob < _n_classes; nprob++){
			ssi_print("%f ", probs[nprob]);
		}
		ssi_print("\n");
	}
	//check
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		if (probs[num_probs] == 3.0f) {
			//clean up
			if(all_probs_hot){
				for(ssi_size_t ind = 0; ind < 3; ind++){
					for(ssi_size_t ind2 = 0; ind2 < _n_modalities; ind2++){
						delete [] all_probs_hot[ind][ind2];
					}
					delete [] all_probs_hot[ind];
				}
				delete [] all_probs_hot;
				all_probs_hot = 0;
			}
			if(all_probs_direct){
				for(ssi_size_t ind = 0; ind < _n_modalities; ind++){
					delete [] all_probs_direct[ind];
				}
				delete [] all_probs_direct;
				all_probs_direct = 0;
			}
			return found_data;
		}
	}

	//Step3
	//clear probs
	for (ssi_size_t num_probs = 0; num_probs < n_probs; num_probs++){
		probs[num_probs] = 0.0f;
	}
	probs[decision] += 1.0f;
	
	//clean up
	if(all_probs_hot){
		for(ssi_size_t ind = 0; ind < 3; ind++){
			for(ssi_size_t ind2 = 0; ind2 < _n_modalities; ind2++){
				delete [] all_probs_hot[ind][ind2];
			}
			delete [] all_probs_hot[ind];
		}
		delete [] all_probs_hot;
		all_probs_hot = 0;
	}
	if(all_probs_direct){
		for(ssi_size_t ind = 0; ind < _n_modalities; ind++){
			delete [] all_probs_direct[ind];
		}
		delete [] all_probs_direct;
		all_probs_direct = 0;
	}
	return found_data;
}

bool VACFusion::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_streams, sizeof (_n_streams), 1);
	file->write (&_n_models, sizeof (_n_models), 1);
	file->write (&_n_modalities, sizeof (_n_modalities), 1);
	
	delete file;
	
	return true;
}

bool VACFusion::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&ssi_log_level, sizeof (ssi_log_level), 1);

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_streams, sizeof (_n_streams), 1);
	file->read (&_n_models, sizeof (_n_models), 1);
	file->read (&_n_modalities, sizeof (_n_modalities), 1);
	
	delete file;
	
	return true;
}

ssi_size_t VACFusion::productRule(ssi_real_t **probs, ssi_size_t n_probs){

	ssi_real_t *fusion_probs = new ssi_real_t[n_probs];
	for(ssi_size_t i = 0; i < n_probs; i++){
		fusion_probs[i] = 1.0f;
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\nProductRule:\n");
		for(ssi_size_t nmod = 0; nmod < _n_modalities; nmod++){
			for(ssi_size_t nprob = 0; nprob < n_probs; nprob++){
				ssi_print("%f ", probs[nmod][nprob]);
			}
			ssi_print("\n");
		}
		ssi_print("\n");
	}

	for(ssi_size_t nmod = 0; nmod < _n_modalities; nmod++){
		for(ssi_size_t nprob = 0; nprob < n_probs; nprob++){
			if(probs[nmod][nprob] != -1.0f){
				fusion_probs[nprob] *= probs[nmod][nprob];
			}
		}
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		for(ssi_size_t nprob = 0; nprob < n_probs; nprob++){
			ssi_print("%f ", fusion_probs[nprob]);
		}
		ssi_print("\n");
	}

	ssi_size_t max_ind = 0;
	ssi_real_t max_val = fusion_probs[0];
	for (ssi_size_t i = 1; i < n_probs; i++) {
		if (fusion_probs[i] > max_val) {
			max_val = fusion_probs[i];
			max_ind = i;
		}
	}
	
	delete [] fusion_probs;

	return max_ind;
}

ssi_size_t VACFusion::sumRule(ssi_real_t **probs, ssi_size_t n_probs){

	ssi_real_t *fusion_probs = new ssi_real_t[n_probs];
	for(ssi_size_t i = 0; i < n_probs; i++){
		fusion_probs[i] = 0.0f;
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print("\nSumRule:\n");
		for(ssi_size_t nmod = 0; nmod < _n_modalities; nmod++){
			for(ssi_size_t nprob = 0; nprob < n_probs; nprob++){
				ssi_print("%f ", probs[nmod][nprob]);
			}
			ssi_print("\n");
		}
		ssi_print("\n");
	}

	for(ssi_size_t nmod = 0; nmod < _n_modalities; nmod++){
		for(ssi_size_t nprob = 0; nprob < n_probs; nprob++){
			if(probs[nmod][nprob] != -1.0f){
				fusion_probs[nprob] += probs[nmod][nprob];
			}
		}
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		for(ssi_size_t nprob = 0; nprob < n_probs; nprob++){
			ssi_print("%f ", fusion_probs[nprob]);
		}
		ssi_print("\n");
	}

	ssi_size_t max_ind = 0;
	ssi_real_t max_val = fusion_probs[0];
	for (ssi_size_t i = 1; i < n_probs; i++) {
		if (fusion_probs[i] > max_val) {
			max_val = fusion_probs[i];
			max_ind = i;
		}
	}
	
	delete [] fusion_probs;

	return max_ind;
}

}
