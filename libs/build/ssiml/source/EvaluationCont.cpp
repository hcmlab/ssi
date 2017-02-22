// EvaluationCont.cpp
// author: Ionut Damian <damian@informatik.uni-augsburg.de>
// created: 2012/10/02
// Copyright (C) 2007-12 University of Augsburg, Ionut Damian
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

#include "EvaluationCont.h"
#include "model/ModelTools.h"
#include "ioput/file/File.h"
#include "ISSelectUser.h"
#include "ISSelectSample.h"
#include "base/IContinuousModel.h"
#include "Trainer.h"
#if __gnu_linux__
#include <unistd.h>
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *EvaluationCont::ssi_log_name = "evaluation";
int EvaluationCont::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

EvaluationCont::Counter::Counter()
{
	sum = 0;
	min = 0;
	max = 0;
}

EvaluationCont::EvaluationCont () : _n_correct(0), _annos(0), _repetitions(0)
{
	_anno_max_delay = 1.0;
	_chunk_size = 2;
}

EvaluationCont::~EvaluationCont () 
{	
	delete[] _nonevent_reco;
	_nonevent_reco = 0;
}

void EvaluationCont::init_conf_mat (ISamples &samples) {

	_n_classes = samples.getClassSize ();

	// store class names
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy (samples.getClassName (i));
	}

	// allocate confussion matrix
	_conf_mat_ptr = new ssi_size_t *[_n_classes];
	_conf_mat_data = new ssi_size_t[_n_classes * _n_classes];
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		_conf_mat_ptr[i] = _conf_mat_data + i*_n_classes;
	}

	// set all elements in the confussion matrix to zero
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		for (ssi_size_t j = 0; j < _n_classes; ++j) {
			_conf_mat_ptr[i][j] = 0;
		}
	}

	// allocate non-event recognition vector
	_nonevent_reco = new ssi_size_t [_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		_nonevent_reco[i] = 0;
	}

	_n_unclassified = 0;
	_n_classified = 0;	
}

void EvaluationCont::eval (ssi_stream_t *stream, Annotation* anno, ssi_real_t fps) 
{	
	// test trainer against whole stream frame by frame
	ssi_size_t wait_time = (fps > 0) ? ssi_cast(ssi_size_t, ssi_cast(ssi_real_t, _chunk_size / fps) * 1000) : 1; //in ms
	ssi_size_t num = 0;
	ssi_size_t index, real_index, old_index = 0;
	ssi_real_t* probs = new ssi_real_t[_n_classes];

	IContinuousModel *model = (IContinuousModel *)_trainer->getModel(0);
	ssi_time_t out_time;
	Annotation::Entry* entry;
	Annotation::Entry* old_entry = 0;

	//this represents the time between the first annotation and the last
	//we assume that anything coming before or after this interval is not part of the evaluation
	anno->reset();
	ssi_time_t start = anno->next()->start; 
	ssi_time_t end = anno->last()->stop;

	while( num < stream->num )
	{
		ssi_stream_t chunk;
		ssi_stream_copy(*stream, chunk, num, num + _chunk_size);
		chunk.time = num * (1.0 / chunk.sr); //relative time counter
		
		_trainer->forward_probs (chunk, _n_classes, 0);

		//check for recognitions	
		while(model->getOutput(_n_classes, probs, &out_time, 0))
		{
			//first we check whether this output is valid
			if(out_time < start || out_time > end)
				continue;

			ssi_size_t max_ind = 0;
			ssi_real_t max_val = probs[0];
			for (ssi_size_t i = 1; i < _n_classes; i++) {
				if (probs[i] > max_val) {
					max_val = probs[i];
					max_ind = i;
				}
			}
			index = max_ind;

			entry = anno->getEntryAt(out_time, _anno_max_delay);

			if(entry == old_entry && index == old_index)
				continue; //do not count duplicates

			if(entry != 0)
			{
				real_index = entry->label_index;
				_conf_mat_ptr[real_index][index]++;
				_n_classified++;
								
				if(real_index == index)
				{
					_n_correct++;

					//measure eagerness of correct recognitions
					ssi_time_t delay_abs = out_time - entry->stop;
					ssi_time_t delay_rel = delay_abs / (entry->stop - entry->start);

					_eager_abs.sum += delay_abs;
					if(delay_abs < _eager_abs.min) _eager_abs.min = delay_abs;
					if(delay_abs > _eager_abs.max) _eager_abs.max = delay_abs;

					_eager_rel.sum += delay_rel;
					if(delay_rel < _eager_rel.min) _eager_rel.min = delay_rel;
					if(delay_rel > _eager_rel.max) _eager_rel.max = delay_rel;
				}

				old_entry = entry;
				old_index = index;
			}
			else
				_nonevent_reco[index]++;
		}
		
		num += _chunk_size;
		ssi_stream_destroy(chunk);
		#if _WIN32|_WIN64
		timeBeginPeriod(1); //increase sleep accuracy
		Sleep(wait_time); //simulate "framrate"
		timeEndPeriod(1);
		#else
		usleep(wait_time);
		#endif
	}

	delete[] probs;
}

void EvaluationCont::evalLOUO (Trainer &trainer, ISamples &samples, std::vector<ssi_stream_t*>* streams, std::vector<Annotation*>* annos, ssi_real_t fps, ssi_size_t reps){
	
	ssi_msg(SSI_LOG_LEVEL_BASIC, "Starting leave-one-user-out (continuous) evaluation with chunkSize=%d fps=%.2f reps=%d", _chunk_size, fps, reps);

	ssi_time_t dur;
	ssi_time_t cfps;	
	_annos = annos;
	_repetitions = reps;

	init (samples, &trainer, IModel::TASK::CLASSIFICATION);

	ssi_size_t n_users = samples.getUserSize ();

	ssi_size_t itest  = 0;
	ssi_size_t *itrain = new ssi_size_t[n_users - 1];
	for (ssi_size_t nuser = 0; nuser < n_users - 1; ++nuser) {
		itrain[nuser] = nuser+1;
	}
	
	ISSelectUser strain (&samples);

	for (ssi_size_t nuser = 0; nuser < n_users; ++nuser) {
		
		if(nuser != 0) itrain[nuser-1] = nuser-1;
		itest = nuser;
		strain.setSelection  (n_users-1, itrain);
		
		for(ssi_size_t i=0; i< reps; ++i)
		{		
			_trainer->release ();
			if (_fselmethod) {
				_trainer->setSelection (strain, _fselmethod, _pre_fselmethod, _n_pre_feature);
			}
			if (_preproc_mode) {
				_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
			}
		
			_trainer->train (strain);

			printf("- starting eval: user %d (rep %d) ...", itest, i);
			ssi_msg(SSI_LOG_LEVEL_BASIC, "- starting eval: user %d (rep %d) ...", itest, i);
			clock_t start = clock();

			eval ((*streams)[itest], (*annos)[itest], fps);

			clock_t end = clock();
			dur = ssi_cast(ssi_time_t, (end - start)) / CLOCKS_PER_SEC;
			cfps = ssi_cast(ssi_time_t, (*streams)[itest]->num) / dur;
			printf("done in %.2fs (%.2f fps) with %d classifications\n", dur, cfps, _n_classified);
			ssi_msg(SSI_LOG_LEVEL_BASIC, "done in %.2fs (%.2f fps) with %d classifications", dur, cfps, _n_classified);
		}
	}

	delete [] itrain;
}

void EvaluationCont::evalFull (Trainer &trainer, ISamples &samples, std::vector<ssi_stream_t*>* streams, std::vector<Annotation*>* annos, ssi_real_t fps, ssi_size_t reps){

	ssi_msg(SSI_LOG_LEVEL_BASIC, "Starting full (continuous) evaluation with chunkSize=%d fps=%.2f reps=%d", _chunk_size, fps, reps);

	ssi_time_t dur;
	ssi_time_t cfps;
	_annos = annos;
	_repetitions = reps;
	
	init (samples, &trainer, IModel::TASK::CLASSIFICATION);
		
	ssi_size_t n_users = samples.getUserSize ();
	for (ssi_size_t nuser = 0; nuser < n_users; ++nuser) 
	{
		for(ssi_size_t i=0; i< reps; ++i)
		{		
			_trainer->release ();
			if (_preproc_mode) {
				_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
			}
		
			_trainer->train (samples);

			printf("- starting eval: user %d (rep %d) ...", nuser, i);
			ssi_msg(SSI_LOG_LEVEL_BASIC, "- starting eval: user %d (rep %d) ...", nuser, i);
			clock_t start = clock();

			eval ((*streams)[nuser], (*annos)[nuser], fps);

			clock_t end = clock();
			dur = ssi_cast(ssi_time_t, (end - start)) / CLOCKS_PER_SEC;
			cfps = ssi_cast(ssi_time_t, (*streams)[nuser]->num) / dur;
			printf("done in %.2fs (%.2f fps) with %d classifications\n", dur, cfps, _n_classified);
			ssi_msg(SSI_LOG_LEVEL_BASIC, "done in %.2fs (%.2f fps) with %d classifications", dur, cfps, _n_classified);
		}
	}
}

void EvaluationCont::print (FILE *file) {

	if (!_conf_mat_ptr) {
		ssi_wrn ("nothing to print");
		return;
	}
		
	///////////////////////
	/// PREPARE RESULTS ///
	///////////////////////

	ssi_size_t max_label_len = 0;
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		ssi_size_t len = ssi_cast (ssi_size_t, strlen (_class_names[i]));
		if (len > max_label_len) {
			max_label_len = len;
		}
	}

	//compute how many class occurences were present in the stream
	ssi_size_t *class_occurences = new ssi_size_t[_n_classes];
	ssi_size_t n_occurences = 0;
	for(ssi_size_t i=0; i< _n_classes; ++i)
	{
		class_occurences[i] = 0;
	}
	for(ssi_size_t i=0; i< _annos->size(); ++i)
	{
		(*_annos)[i]->reset();
		Annotation::Entry *e = (*_annos)[i]->next();
		while( e != 0 )
		{
			class_occurences[e->label_index] += 1 * _repetitions;
			n_occurences += 1 * _repetitions;
			e = (*_annos)[i]->next();
		}
	}

	///////////////////////
	///  PRINT RESULTS  ///
	///////////////////////

	File *tmp = File::Create (File::ASCII, File::WRITE, 0, file);
	tmp->setType (SSI_UINT);
	tmp->setFormat (" ", "6");	
	
	ssi_real_t prob = 0;
	ssi_real_t prob_sum = 0;
	
	ssi_fprint (file, "#classes:      %u\n", _n_classes);
	ssi_fprint (file, "#total:        %u\n", n_occurences);
	ssi_fprint (file, "#classified:   %u\n", _n_classified);
	ssi_fprint (file, "#unclassified: %u\n", _n_unclassified);
	for (ssi_size_t i = 0; i < _n_classes; ++i) {		
		ssi_fprint (file, "%*s: ", max_label_len, _class_names[i]);
		tmp->write (_conf_mat_ptr[i], 0, _n_classes);

		prob = ssi_cast (ssi_real_t, _conf_mat_ptr[i][i]) / ssi_cast (ssi_real_t, class_occurences[i]);
		if(prob > 1.0f) prob = 1.0f;
		ssi_fprint (file, "   -> %8.2f%%\n", 100* prob);
		prob_sum += prob;
	}
	ssi_fprint (file, "   %*s  => %8.2f%% | %.2f%%\n", max_label_len + _n_classes * 7, "", 100* (prob_sum / ssi_cast (ssi_real_t, _n_classes)), 100*get_accuracy_prob ());

	
	ssi_fprint (file, "\n== Non-event recognitions ==\n");
	ssi_size_t *ptr = _nonevent_reco;
	ssi_size_t sum = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {	
		sum += _nonevent_reco[i];
	}
		
	ssi_fprint (file, "%*s: ", max_label_len, "#");
	tmp->write (_nonevent_reco, 0, _n_classes);
	ssi_fprint (file, "   -> %u\n", sum);

	ssi_fprint (file, "\n== Recognition Time Differences (Eagerness) ==\n");
	ssi_fprint (file, "avg: %8.4f (%8.4fs)\n", _eager_rel.sum / _n_correct, _eager_abs.sum / _n_correct);
	ssi_fprint (file, "min: %8.4f (%8.4fs)\n", _eager_rel.min, _eager_abs.min);
	ssi_fprint (file, "max: %8.4f (%8.4fs)\n", _eager_rel.max, _eager_abs.max);


	delete tmp;
	fflush (file);
}

}
