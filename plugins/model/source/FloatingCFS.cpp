// FloatingCFS.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/02/23
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

#include "FloatingCFS.h"
#include "Evaluation.h"
#include "Trainer.h"

namespace ssi {

FloatingCFS::FloatingCFS (const ssi_char_t *file) :
		_model (0),
		_file (0),
		_stream_index (0),
		_n_dims (0),
		_n_classes (0),
		_n_scores (0),
		_scores (0),
		_samples (0),
		_train_instances (0),
		_corr_matrix (0),
		_std_devs (0),
		_numAttribs (0),
		_numInstances (0),
		_classIndex (0)		
	{

	ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
	ssi_log_name = ssi_strcpy ("fselect___");

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

FloatingCFS::~FloatingCFS () {

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
	delete[] ssi_log_name;
}

bool FloatingCFS::train (ISamples &samples, ssi_size_t stream_index) {

	/*if (!_model) {
		ssi_wrn ("a model has not been set yet");
		return false;
	}*/

	release ();

	_n_dims = samples.getStream (stream_index).dim;
	_n_classes = samples.getClassSize ();
	_n_scores = _options.nfirst == 0 ? _n_dims : _options.nfirst;
	_scores = new score[_n_dims];
	_stream_index = stream_index;	
	_samples = &samples;

	buildEvaluator ();

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

void FloatingCFS::print (FILE *file) {

	for (ssi_size_t i = 0; i < _n_scores; i++) {
		ssi_fprint (file, "%u: %.2f\n", _scores[i].index, _scores[i].value);
	}
}

void FloatingCFS::release () {

	delete[] _scores; _scores = 0;
	_n_scores = 0;
	_samples = 0;
	
	// CFS
	if (_train_instances)
	{
		for (ssi_size_t i = 0; i < _numInstances; i++)
		{
			delete[] _train_instances[i];
			_train_instances[i] = 0;
		}
		delete[] _train_instances;
		_train_instances = 0;
	}

	if (_corr_matrix)
	{
		for (ssi_size_t i = 0; i < _numAttribs; i++)
		{
			delete[] _corr_matrix[i];
			_corr_matrix[i] = 0;
		}
		delete[] _corr_matrix;
		_corr_matrix = 0;
	}
	delete[] _std_devs;
	_std_devs = 0;
}


bool FloatingCFS::lr_search (ssi_size_t n_keep, ssi_size_t l, ssi_size_t r) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "plus-l-minus-r: n=%u k=%u l=%u r=%u", _n_dims, n_keep, l, r);

	if (n_keep > _n_dims) {
		ssi_wrn ("#dimension to select exceeds #dimensions");
		return false;
	}	

	ssi_size_t k;

	// Initialisierung
	if ( l > r ) {
		k = 0;
		inclusion (n_keep, k, l, r);
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

void FloatingCFS::inclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r) {

	if (l > 0) {

		ssi_real_t *probs = new ssi_real_t[_n_dims];
		
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
					probs[ndim] = eval_h (k + 1, sel);					

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

void FloatingCFS::exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_size_t l, ssi_size_t r) {

	if (r > 0) {

		ssi_real_t *probs = new ssi_real_t[_n_dims];
		
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
				probs[ndim] = eval_h (k - 1, sel);

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

	if (k != n_keep)
		inclusion (n_keep, k, l, r);
}

bool FloatingCFS::sffs (ssi_size_t n_keep) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "sffs: k=%u", n_keep);

	if (n_keep > _n_dims) {		
		ssi_wrn ("#dimension to select exceeds #dimensions");
		return false;
	}	

	ssi_size_t k = 0;

	sffs_inclusion (n_keep, k);

	return true;
}


void FloatingCFS::sffs_inclusion (ssi_size_t n_keep, ssi_size_t k) {

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
			probs[ndim] = eval_h (k + 1, sel);

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

void FloatingCFS::sffs_exclusion (ssi_size_t n_keep, ssi_size_t k, ssi_real_t prev_eval) {

	ssi_real_t *probs = new ssi_real_t[_n_dims];
	
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

		probs[ndim] = eval_h (k - 1, sel);

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


/**
* Initializes the correlation matrix.
*/
void FloatingCFS::buildEvaluator()
{
	// Set values
	_numAttribs = _n_dims + 1;
	_classIndex = _n_dims;
	_numInstances = _samples->getSize ();

	// Create the _train_instances array
	_train_instances = new ssi_real_t*[_numInstances];
	_samples->reset ();
	ssi_size_t i = 0;
	ssi_sample_t *sample;
	while (sample = _samples->next ())
	{
		_train_instances[i] = new ssi_real_t[_numAttribs];

		// Initialize the matrix with Feature values
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, sample->streams[_stream_index]->ptr);
		for (ssi_size_t j = 0; j < _n_dims; j++)
		{
			_train_instances[i][j] = *ptr++;
		}

		// Save class index as last element
		_train_instances[i][_classIndex] = ssi_cast (ssi_real_t, sample->class_id);
		i++;
	}

	// Create correlation arrays
	_std_devs = new ssi_real_t[_numAttribs];
	_corr_matrix = new ssi_real_t*[_numAttribs];
	for (ssi_size_t i = 0; i < _numAttribs; i++)
	{
		_corr_matrix[i] = new ssi_real_t[i + 1];
	}

	// Feature correlation with itself and default standard deviation.
	for (ssi_size_t i = 0; i < _numAttribs; i++)
	{
		_corr_matrix[i][i] = 1.0;
		_std_devs[i] = 1.0;
	}

	// Initialize all other values with the largest negative value.
	for (ssi_size_t i = 0; i < _numAttribs; i++)
	{
		for (ssi_size_t j = 0; j < i; j++)
		{
			_corr_matrix[i][j] = -FLT_MAX;
		}
	}
}

ssi_real_t FloatingCFS::eval_h (ssi_size_t n_dims, const ssi_size_t* dims)
{
	double num = 0;
	double denom = 0;
	ssi_size_t larger, smaller;

	// Initialize a BitSet subset (like weka).
	bool *subset = new bool[_n_dims];
	for (ssi_size_t i = 0; i < _n_dims; i++)
	{
		subset[i] = false;
	}
	for (ssi_size_t i = 0; i < n_dims; i++)
	{
		subset[dims[i]] = true;
	}

	// do numerator
	for (ssi_size_t i = 0; i < _numAttribs; i++)
	{
		if (i != _classIndex && subset[i])
		{
			// Determine which value is smaller/larger: i or _classIndex.
			// This must be done, as only the lower left part of the matrix is calculated.
			// In our case the _classIndex is always larger, as it is stored as the last 'feature'.
			if (i > _classIndex)
			{
				larger = i;
				smaller = _classIndex;
			}
			else 
			{
				smaller = i;
				larger = _classIndex;
			}

			if (_corr_matrix[larger][smaller] == -FLT_MAX)
			{
				_corr_matrix[larger][smaller] = correlate (i, _classIndex);
			}

			num += _std_devs[i] * _corr_matrix[larger][smaller];
		}
	}

	// do denominator
	for (ssi_size_t i = 0; i < _numAttribs; i++)
	{
		if (i != _classIndex && subset[i])
		{
			denom += _std_devs[i] * _std_devs[i];

			for (ssi_size_t j = 0; j < i; j++)
			{
				if (subset[j])
				{
					if (_corr_matrix[i][j] == -FLT_MAX)
					{
						_corr_matrix[i][j] = correlate (i, j);
					}

					denom += 2.0 * _std_devs[i] * _std_devs[j] * _corr_matrix[i][j];
				}
			}
		}
	}

	// Clean up
	delete[] subset;

	if (denom == 0.0)
	{
		return 0.0;
	}

	ssi_real_t merit = ssi_cast (ssi_real_t, abs (num / sqrt (abs (denom))));

	return merit;
}

ssi_real_t FloatingCFS::correlate (ssi_size_t att1, ssi_size_t att2)
{
	if (att1 == _classIndex && att2 == _classIndex)
	{
		ssi_wrn ("can not correlate two classes");
		return 0;
	}
	else if (att1 == _classIndex)
	{
		return num_nom2 (att1, att2);
	}
	else if (att2 == _classIndex)
	{
		return num_nom2 (att2, att1);
	}
	else
	{
		return num_num (att1, att2);
	}
}

ssi_real_t FloatingCFS::num_num (ssi_size_t att1, ssi_size_t att2)
{
	if (att1 == _classIndex || att2 == _classIndex)
	{
		ssi_wrn ("num_num() called with the class index");
	}

    ssi_real_t diff1, diff2, num = 0.0, sx = 0.0, sy = 0.0;
    ssi_real_t mx = meanOrMode(att1);
    ssi_real_t my = meanOrMode(att2);

    for (ssi_size_t i = 0; i < _numInstances; i++)
	{
        diff1 = _train_instances[i][att1] - mx;
        diff2 = _train_instances[i][att2] - my;
        num += diff1 * diff2;
        sx += diff1 * diff1;
        sy += diff2 * diff2;
    }

    if (sx != 0.0)
	{
        if (_std_devs[att1] == 1.0)
		{
            _std_devs[att1] = sqrt (sx / _numInstances);
        }
    }

    if (sy != 0.0)
	{
        if (_std_devs[att2] == 1.0)
		{
            _std_devs[att2] = sqrt (sy / _numInstances);
        }
    }

    if ((sx * sy) > 0.0)
	{
        return abs (num / sqrt (sx * sy));
    }
	else
	{
        if (att1 != _classIndex && att2 != _classIndex)
		{
            return 1.0;
        }
		else
		{
			// This should never happen
            return 0.0;
        }
    }
}
	
/**
* att1 = nominal, att2 = numeric
*/
ssi_real_t FloatingCFS::num_nom2 (ssi_size_t att1, ssi_size_t att2)
{
	ssi_size_t i, ii, k;
	ssi_real_t temp;
	ssi_size_t mx = ssi_cast (ssi_size_t, meanOrMode(att1));
	ssi_real_t my = meanOrMode(att2);
	ssi_real_t stdv_num = 0.0;
	ssi_real_t diff1, diff2;
	ssi_real_t r = 0.0, rr;

	//	häää?
//	int nx = (!m_missingSeperate) ? _samples.attribute(att1).numValues() : _samples.attribute(att1).numValues() + 1;
	ssi_size_t nx = _n_classes;

	ssi_real_t * prior_nom = new ssi_real_t[nx]; //(ssi_real_t *) malloc(nx*sizeof(ssi_real_t));
	ssi_real_t * stdvs_nom = new ssi_real_t[nx]; //(ssi_real_t *) malloc(nx*sizeof(ssi_real_t));
	ssi_real_t * covs = new ssi_real_t[nx];		 //(ssi_real_t *) malloc(nx*sizeof(ssi_real_t));

	for (i = 0; i < nx; i++) {
		stdvs_nom[i] = covs[i] = prior_nom[i] = 0.0;
	}

	// calculate frequencies (and means) of the values of the nominal attribute
	for (i = 0; i < _numInstances; i++) {
		ii = ssi_cast (ssi_size_t, _train_instances[i][att1]);

		// increment freq for nominal
		prior_nom[ii]++;
	}

	for (k = 0; k < _numInstances; k++) {
		//inst = _samples.instance(k);
		// std dev of numeric attribute
		//diff2 =  (inst.value(att2) - my);
		diff2 =  (_train_instances[k][att2] - my);
		stdv_num += (diff2 * diff2);

		// 
		for (i = 0; i < nx; i++) {
			
			temp = (i == _train_instances[k][att1]) ? 1.0f : 0.0f;
			
			diff1 = (temp - (prior_nom[i] / _numInstances));
			stdvs_nom[i] += (diff1 * diff1);
			covs[i] += (diff1 * diff2);
		}
	}

	// calculate weighted correlation
	for (i = 0, temp = 0.0; i < nx; i++) {
		// calculate the weighted variance of the nominal
		temp += ((prior_nom[i] / _numInstances) * (stdvs_nom[i] / _numInstances));

		if ((stdvs_nom[i] * stdv_num) > 0.0) {
			//System.out.println("Stdv :"+stdvs_nom[i]);
			rr = abs (covs[i] / (sqrt(stdvs_nom[i] * stdv_num)));

			r += ((prior_nom[i] / _numInstances) * rr);
		}
		/* if there is zero variance for the numeric att at a specific 
		level of the catergorical att then if neither is the class then 
		make this correlation at this level maximally bad i.e. 1.0. 
		If either is the class then maximally bad correlation is 0.0 */
		else {
			if (att1 != _classIndex && att2 != _classIndex) {
				r += prior_nom[i] / _numInstances;
			}
		}
	}

	// set the standard deviations for these attributes if necessary
	// if ((att1 != classIndex) && (att2 != classIndex)) // =============
	if (temp != 0.0) {
		if (_std_devs[att1] == 1.0) {
			_std_devs[att1] = sqrt(temp);
		}
	}

	if (stdv_num != 0.0) {
		if (_std_devs[att2] == 1.0) {
			_std_devs[att2] = sqrt((stdv_num / _numInstances));
		}
	}

	if (r == 0.0) {
		if (att1 != _classIndex && att2 != _classIndex) {
			r = 1.0;
		}
	}

	// Clean up
	delete[] prior_nom;
	delete[] covs;
	delete[] stdvs_nom;

	return r;
}

ssi_real_t FloatingCFS::mean_float (ssi_real_t* fray, ssi_size_t length)
{
	ssi_real_t help = 0;
	for (ssi_size_t i = 0; i < length; i++)
	{
		help += fray[i];
	}

	return help / length;
}

ssi_real_t FloatingCFS::meanOrMode(ssi_size_t att) {
	if (att == _classIndex)
	{
		ssi_size_t *anz = new ssi_size_t[_n_classes];
		for (ssi_size_t i=0; i<_n_classes; i++) {
			anz[i] = 0;
		}
		for (ssi_size_t i=0; i<_numInstances; i++) {
			anz[(ssi_size_t)_train_instances[i][att]]++;
		}
		int maxindex = 0;
		for (ssi_size_t i = 1; i < _n_classes; i++) {
			if (anz[i] > anz[maxindex]) {
				maxindex = i;
			}
		}

		// Clean up
		delete[] anz;

		return ssi_cast (ssi_real_t, maxindex);
	}
	else {
		if (_numInstances == 0)
		{
			return 0;
		}

		ssi_real_t werte = 0;
		for (ssi_size_t i = 0; i < _numInstances; i++)
		{
			werte += _train_instances[i][att];
		}
		return werte / _numInstances;
	}
}

void FloatingCFS::printCorrelationMatrix (FILE *file)
{
	for (ssi_size_t i = 0; i < _numAttribs; i++)
	{
		for (ssi_size_t j = 0; j <= i; j++)
		{
			fprintf (file, "%.2f ", _corr_matrix[i][j]);
		}
		fprintf (file, "\n");
	}
}

void FloatingCFS::printStdDevs (FILE *file)
{
	for (ssi_size_t i = 0; i < _numAttribs; i++)
	{
		fprintf (file, "%.2f ", _std_devs[i]);
	}
	fprintf (file, "\n");
}

}
