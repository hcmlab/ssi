// FloatingSearch.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/03
// Copyright (C) University of Augsburg
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

#include "FloatingSearch.h"
#include "Trainer.h"
#include "Evaluation.h"
#include "ISSelectDim.h"
#include "model/ModelTools.h"
#include "thread/ThreadPool.h"

namespace ssi {

FloatingSearch::FloatingSearch (const ssi_char_t *file) :
	_model (0),
	_file (0),
	_stream_index (0),
	_n_dims (0),
	_n_scores (0),
	_scores (0),
	_samples (0) {


	ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
	ssi_log_name = ssi_strcpy ("fsearch___");

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}
	
FloatingSearch::~FloatingSearch () {

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	delete[] ssi_log_name;
}

bool FloatingSearch::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (!_model) {
		ssi_wrn ("a model has not been set yet");
		return false;
	}

	release ();

	_n_dims = samples.getStream (stream_index).dim;	
	_n_scores = _options.nfirst == 0 ? _n_dims : _options.nfirst;
	_scores = new score[_n_dims];
	_stream_index = stream_index;	
	_samples = &samples;

	switch (_options.method) {
		case SFS:
			lr_search (_n_scores, 1, 0);
			break;
		case SBS:
			lr_search (_n_scores, 0, 1);
			break;
		case LR:
			lr_search (_n_scores, _options.l, _options.r);
			break;
		case SFFS:
			sffs (_n_scores);
			break;
		default:
			ssi_wrn ("unkown method");
			return false;
	}

	return true;
}

void FloatingSearch::print (FILE *file) {
		
	for (ssi_size_t i = 0; i < _n_scores; i++) {
		ssi_fprint (file, "%u: %.2f\n", _scores[i].index, _scores[i].value);
	}
}

void FloatingSearch::release () {

	delete[] _scores; _scores = 0;
	_n_scores = 0;
	_samples = 0;
}


bool FloatingSearch::lr_search (ssi_size_t n_keep, ssi_size_t l, ssi_size_t r) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "plus-l-minus-r: n=%u k=%u l=%u r=%u", _n_dims, n_keep, l, r);

	if (n_keep > _n_dims) {
		ssi_wrn ("#dimension to select exceeds #dimensions");
		return false;
	}	

	ssi_size_t k;

	// Initialisierung
	if ( l > r ) {
		k = 0;
		if (_options.nthread > 0) {
			tp_inclusion (n_keep, k, l, r);
		} else {
			inclusion (n_keep, k, l, r);
		}
	}
	else if ( l < r ) {
		k = _n_dims;
		for ( ssi_size_t i = 0; i < _n_dims; i++ ) {
			_scores[i].index = i;			
		}
		exclusion (n_keep, k, l, r);
		for ( ssi_size_t i = 1; i < _n_scores; i++ ) {		
			_scores[i].value = _scores[0].value;
		}
	}
	else {
		ssi_wrn ("l cannot be equals r");
		return false;
	}

	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print ("\nfinal: [");
		for (ssi_size_t c = 0; c < n_keep; c++) {
			ssi_print (" %u=%.2f", _scores[c].index, _scores[c].value);
		}
		ssi_print (" ]\n");
	}

	return true;
}

void FloatingSearch::inclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r) {

	if (l > 0) {

		ssi_real_t *probs = new ssi_real_t[_n_dims];
		Evaluation eval;
		Trainer trainer (_model, _stream_index);

		bool exists; 

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "inclusion: k=%u l=%u", k, l);

		// Füge l neue Features hinzu
		ssi_size_t i = 0;
		while( (k != n_keep) && i < l ) 
		{
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print ("selected: [");
				for (ssi_size_t c = 0; c < k; c++) {
					ssi_print (" %u", _scores[c].index);
				}
				ssi_print (" ]\n");
			}

			// Neues Feature hinzufügen
			ssi_size_t *sel = new ssi_size_t[k+1];
			for (ssi_size_t x = 0; x < k; x++) {
				sel[x] = _scores[x].index;
			}

			for (ssi_size_t ndim = 0; ndim < _n_dims; ndim++) { 
				
				// Bereits vorhandene Features überspringen
				exists = false;
				for (ssi_size_t j = 0; j < k; j++) {
					if (_scores[j].index == ndim) {
						exists = true;
						probs[ndim] = -FLT_MAX;
					}

				}
				
				// Feature noch nicht enthalten
				if (!exists) {

					sel[k] = ndim;
					ISSelectDim samples_s (_samples);
					samples_s.setSelection (_stream_index, k + 1, sel);
					probs[ndim] = eval_h (&eval, &trainer, samples_s);					

					if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
						ssi_print ("+%u=%.2f\n", ndim, probs[ndim]);
					}
				}
			}
			delete[] sel;

			// Wähle das beste Feature
			ssi_real_t max_val = probs[0];
			ssi_size_t max_ind = 0;
			for (ssi_size_t ndim = 1; ndim < _n_dims; ndim++) {
				if (probs[ndim] > max_val) {
					max_val = probs[ndim];
					max_ind = ndim;
				}
			}
			_scores[k].index = max_ind;
			_scores[k].value = max_val;
			probs[max_ind] = -FLT_MAX;

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "add: %u=%.2f", max_ind, max_val);

			k++;
			i++;
		}

		delete[] probs;
	}

	if (k != n_keep)
		exclusion (n_keep, k, l, r);
}

void FloatingSearch::tp_inclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r) {

	if (l > 0) {

		ssi::ThreadPool tp ("inclusion", _options.nthread);

		ssi_real_t *probs = new ssi_real_t[_n_dims];
		Trainer trainer (_model, _stream_index);

		bool exists; 

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "inclusion (tp): k=%u l=%u", k, l);

		// Füge l neue Features hinzu
		ssi_size_t i = 0;
		while( (k != n_keep) && i < l ) 
		{
			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print ("selected: [");
				for (ssi_size_t c = 0; c < k; c++) {
					ssi_print (" %u", _scores[c].index);
				}
				ssi_print (" ]\n");
			}

			// Neues Feature hinzufügen
			ssi_size_t *sel = new ssi_size_t[k+1];
			for (ssi_size_t x = 0; x < k; x++) {
				sel[x] = _scores[x].index;
			}

			for (ssi_size_t ndim = 0; ndim < _n_dims; ndim++) { 
				
				// Bereits vorhandene Features überspringen
				exists = false;
				for (ssi_size_t j = 0; j < k; j++) {
					if (_scores[j].index == ndim) {
						exists = true;
						probs[ndim] = -FLT_MAX;
					}

				}
				
				// Feature noch nicht enthalten
				if (!exists) {

					sel[k] = ndim;
					ISSelectDim samples_s (_samples);
					samples_s.setSelection (_stream_index, k + 1, sel);
					//probs[ndim] = eval_h (eval, trainer, samples_s);		

					if (!trainer.isTrained ()) {
						trainer.train (samples_s);
						trainer.save ("~tp_inclusion");
					}

					tp_eval_h_in_s *in = new tp_eval_h_in_s;
					in->dim = ndim;
					in->options = &_options;
					SampleList *samples = new SampleList ();
					ModelTools::CopySampleList (samples_s, *samples);
					in->samples = samples;
					Trainer *trainer = new Trainer ();
					if (!Trainer::Load (*trainer, "~tp_inclusion")) {
						ssi_err ("could not load temporal copy of trainer '%s'", "~tp_inclusion");
					}
					in->trainer = trainer;

					tp_eval_h_out_s *out = new tp_eval_h_out_s;
					out->result = 0.0f;

					ThreadPool::job_s job;
					job.n_in = sizeof (*in);
					job.in = in;
					job.n_out = sizeof (*out);
					job.out = out;
					job.job = FloatingSearch::tp_eval_h;

					tp.add (job);

					if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
						ssi_print ("+%u=%.2f\n", ndim, probs[ndim]);
					}
				}
			}

			tp.work ();

			ThreadPool::job_s job;
			for (ssi_size_t njob = 0; njob < tp.size (); njob++) {

				job = tp.get (njob);
				tp_eval_h_in_s *in = ssi_pcast (tp_eval_h_in_s, job.in);
				tp_eval_h_out_s *out = ssi_pcast (tp_eval_h_out_s, job.out);

				probs[in->dim] = out->result;

				delete in->samples;
				delete in->trainer;
				delete in;
				delete out;
			}

			tp.clear ();

			delete[] sel;

			// Wähle das beste Feature
			ssi_real_t max_val = probs[0];
			ssi_size_t max_ind = 0;
			for (ssi_size_t ndim = 1; ndim < _n_dims; ndim++) {
				if (probs[ndim] > max_val) {
					max_val = probs[ndim];
					max_ind = ndim;
				}
			}
			_scores[k].index = max_ind;
			_scores[k].value = max_val;
			probs[max_ind] = -FLT_MAX;

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "add: %u=%.2f", max_ind, max_val);

			k++;
			i++;
		}

		delete[] probs;
	}

	if (k != n_keep)
		exclusion (n_keep, k, l, r);
}



void FloatingSearch::exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r) {

	if (r > 0) {

		ssi_real_t *probs = new ssi_real_t[_n_dims];
		Evaluation eval;
		Trainer trainer (_model, _stream_index);

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "exclusion: k=%u r=%u", k, r);

		// Entferne r Features
		ssi_size_t i = 0;

		while ((k != n_keep) && i < r ) {

			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print ("selected: [");
				for (ssi_size_t c = 0; c < k; c++) {
					ssi_print (" %u", _scores[c].index);
				}
				ssi_print (" ]\n");
			}

			// schlechtestes Feature entfernen
			for (ssi_size_t ndim = 0; ndim < k; ndim++) { 
				ssi_size_t *sel = new ssi_size_t[k-1]; 

				// entfernt ein Feature 
				ssi_size_t src = 0;
				ssi_size_t des = 0;
				for (ssi_size_t j = 0; j < k; j++) {
					if (j != ndim) 
						sel[des++] = _scores[src++].index;
					else {
						src++;
					}
				}
				ISSelectDim samples_s (_samples);
				samples_s.setSelection (_stream_index, k-1, sel);
				probs[ndim] = eval_h (&eval, &trainer, samples_s);

				if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
					ssi_print ("-%u=%.2f\n", _scores[ndim].index, probs[ndim]);
				}

				delete[] sel;
			}

			// Wähle das schlechteste Feature
			ssi_real_t max_val = probs[0];
			ssi_size_t max_ind = 0;
			for (ssi_size_t ndim = 1; ndim < k; ndim++) {
				if (probs[ndim] > max_val) {
					max_val = probs[ndim];
					max_ind = ndim;
				}
			}

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "remove: %u=%2.f", _scores[max_ind].index, max_val);

			// Schlechtestes Feature entfernen und normalisieren
			ssi_size_t src = 0;
			ssi_size_t des = 0;
			for (ssi_size_t f = 0; f < k; f++) {
				if (f != max_ind) 
					_scores[des++] = _scores[src++];
				else {
					src++;
				}
			}
			k--;
			i++;

			// store value in first score
			_scores[0].value = max_val;
		}

		delete[] probs;
	}

	if (k != n_keep) {
		if (_options.nthread > 0) {
			tp_inclusion (n_keep, k, l, r);
		} else {
			inclusion (n_keep, k, l, r);
		}
	}
}

bool FloatingSearch::sffs (ssi_size_t n_keep) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "sffs: k=%u", n_keep);
	
	if (n_keep > _n_dims) {		
		ssi_wrn ("#dimension to select exceeds #dimensions");
		return false;
	}	

	ssi_size_t k = 0;

	sffs_inclusion (n_keep, k);

	return true;
}


void FloatingSearch::sffs_inclusion (ssi_size_t n_keep, ssi_size_t k) {

	ssi_real_t *probs = new ssi_real_t[_n_dims];
	Evaluation eval;
	Trainer trainer (_model, _stream_index);

	bool exists;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "inclusion: k=%u\n", k);
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print ("selected: [");
		for (ssi_size_t c = 0; c < k; c++) {
			ssi_print (" %u", _scores[c].index);
		}
		ssi_print (" ]\n");
	}

	// Neues Feature hinzufügen
	for (ssi_size_t ndim = 0; ndim < _n_dims; ndim++) { 
		
		// Vorhandene Features überspringen
		exists = false;
		for (ssi_size_t j = 0; j < k; j++) {
			if (_scores[j].index == ndim) {
				exists = true;
				probs[ndim] = 0;
			}

		}
		
		if (!exists) {
			ssi_size_t *sel = new ssi_size_t[k+1];
			for (ssi_size_t i = 0; i < k+1; i++)
				sel[i] = _scores[i].index;

			sel[k] = ndim;
			ISSelectDim samples_s (_samples);
			bool b = samples_s.setSelection (_stream_index, k + 1, sel);
			probs[ndim] = eval_h (&eval, &trainer, samples_s);

			if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
				ssi_print ("+%u=%.2f\n", ndim, probs[ndim]);
			}

			delete[] sel;
		}
	}

	// Wähle das beste Feature
	ssi_real_t max_val = probs[0];
	ssi_size_t max_ind = 0;
	for (ssi_size_t ndim = 1; ndim < _n_dims; ndim++) {
		if (probs[ndim] > max_val) {
			max_val = probs[ndim];
			max_ind = ndim;
		}
	}
	_scores[k].index = max_ind;
	_scores[k].value = max_val;
	probs[max_ind] = 0;

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "add: %u=%.2f", max_ind, max_val);

	k++;

	delete[] probs;	

	if (k != n_keep)
		sffs_exclusion (n_keep, k, max_val);
}

void FloatingSearch::sffs_exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_real_t prev_eval) {
	
	ssi_real_t *probs = new ssi_real_t[_n_dims];
	Evaluation eval;
	Trainer trainer (_model, _stream_index);

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "exclusion: k=%u p=%.2f\n", k, prev_eval);
	if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
		ssi_print ("selected: [");
		for (ssi_size_t c = 0; c < k; c++) {
			ssi_print (" %u", _scores[c].index);
		}
		ssi_print (" ]\n");
	}

	// jeweils ein Feature entfernen
	for (ssi_size_t ndim = 0; ndim < k; ndim++) { 
		ssi_size_t *sel = new ssi_size_t[k-1]; 

		// entfernt ein Feature 
		ssi_size_t src = 0;
		ssi_size_t des = 0;
		for (ssi_size_t j = 0; j < k; j++) {
			if (j != ndim) 
				sel[des++] = _scores[src++].index;
			else {
				src++;
			}
		}

		ISSelectDim samples_s (_samples);
		samples_s.setSelection (_stream_index, k-1, sel);
		probs[ndim] = eval_h (&eval, &trainer, samples_s);

		if (ssi_log_level >= SSI_LOG_LEVEL_DEBUG) {
			ssi_print ("-%u=%.2f\n", _scores[ndim].index, probs[ndim]);
		}

		delete[] sel;
	}

	// Wähle das schlechteste Feature
	ssi_real_t max_val = probs[0];
	ssi_size_t max_ind = 0;
	for (ssi_size_t ndim = 1; ndim < k; ndim++) {
		if (probs[ndim] > max_val) {
			max_val = probs[ndim];
			max_ind = ndim;
		}
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "remove: %u=%.2f %s", _scores[max_ind].index, max_val, max_val > prev_eval ? "" : "SKIP");

	delete[] probs;

	if (max_val > prev_eval) {
		// Feature entfernen und normalisieren
		ssi_size_t src = 0;
		ssi_size_t des = 0;
		for (ssi_size_t f = 0; f < k; f++) {
			if (f != max_ind) 
				_scores[des++] = _scores[src++];
			else {
				src++;
			}
		}
		k--;
		sffs_exclusion (n_keep, k, max_val);
	}
	else {
		sffs_inclusion (n_keep, k);
	}
}

SSI_INLINE ssi_real_t FloatingSearch::eval_h (Evaluation *eval, Trainer *trainer, ISamples &samples) {

	if (_options.loo) {
		eval->evalLOO (trainer, samples);
	} else if (_options.louo) {
		eval->evalLOUO (trainer, samples);
	} else if (_options.split > 0 && _options.split < 1) {
		eval->evalSplit (trainer, samples, _options.split);
	} else {
		eval->evalKFold (trainer, samples, _options.kfold);
	}
	
	return _options.eval == FloatingSearch::CLASSWISE ? eval->get_classwise_prob () : eval->get_accuracy_prob ();
}

SSI_INLINE bool FloatingSearch::tp_eval_h (ssi_size_t n_in, void *in, ssi_size_t n_out, void *out) {

	SSI_ASSERT (sizeof (tp_eval_h_in_s) == n_in);
	SSI_ASSERT (sizeof (tp_eval_h_out_s) == n_out);

	tp_eval_h_in_s *in_s = ssi_pcast (tp_eval_h_in_s, in);
	Options *options = in_s->options;
	Evaluation eval;
	Trainer *trainer = in_s->trainer;
	ISamples *samples = in_s->samples;

	if (options->loo) {
		eval.evalLOO (trainer, *samples);
	} else if (options->louo) {
		eval.evalLOUO (trainer, *samples);
	} else if (options->split > 0 && options->split < 1) {
		eval.evalSplit (trainer, *samples, options->split);
	} else {
		eval.evalKFold (trainer, *samples, options->kfold);
	}
	
	ssi_real_t result = options->eval == FloatingSearch::CLASSWISE ? eval.get_classwise_prob () : eval.get_accuracy_prob ();

	tp_eval_h_out_s *out_s = ssi_pcast (tp_eval_h_out_s, out);
	out_s->result = result;
	
	return true;
}

}
