// NaiveBayes.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/05
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
//
// based on an implementation by Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt

#include "NaiveBayes.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

NaiveBayes::NaiveBayes (const ssi_char_t *file) 
	: _n_features (0),
	_n_classes (0),
	_class_probs (0),
	_class_names (0),
	_means (0),	
	_std_dev (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
} 

NaiveBayes::~NaiveBayes () { 

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void NaiveBayes::release () {

	free_class_names ();
	
	delete[] _class_probs; _class_probs = 0;

	if (_means) {
		for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
			delete[] _means[nclass];
		}
		delete[] _means; _means = 0;
	}

	if (_std_dev) {
		for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
			delete[] _std_dev[nclass];
		}
		delete[] _std_dev; _std_dev = 0;
	}

	_n_features = 0;
	_n_classes = 0;
}

void NaiveBayes::init_class_names (ISamples &samples) {

	free_class_names ();

	_n_classes = samples.getClassSize ();
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy (samples.getClassName (i));
	}
}

void NaiveBayes::free_class_names () {

	if (_class_names) {
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			delete[] _class_names[i];
		}
		delete[] _class_names;
		_class_names = 0;
	}	
}

bool NaiveBayes::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = samples.getClassSize ();
	_n_features = samples.getStream (stream_index).dim;		
	ssi_size_t *n_samples = new ssi_size_t[_n_classes + 1];
	_means = new ssi_real_t *[_n_classes];
	_std_dev = new ssi_real_t *[_n_classes];
	_class_probs = new ssi_real_t[_n_classes];
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		n_samples[nclass] = samples.getSize (nclass);
		_means[nclass] = new ssi_real_t[_n_features];
		_std_dev[nclass] = new ssi_real_t[_n_features];		
		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
			_means[nclass][nfeat] = 0;
			_std_dev[nclass][nfeat] = 0;
		}
		_class_probs[nclass] = 0;
	}
	n_samples[_n_classes] = samples.getSize ();

	samples.reset ();
	ssi_sample_t *sample = 0;
	ssi_real_t *ptr = 0;
	while (sample = samples.next ()) {
		ptr = ssi_pcast (ssi_real_t, sample->streams[stream_index]->ptr);
		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
            ssi_real_t value = *ptr;
            ptr++;
			_means[sample->class_id][nfeat] += value;
			_std_dev[sample->class_id][nfeat] += value * value;
		}
	}
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		if (n_samples[nclass] > 0) {
			for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
				_means[nclass][nfeat] /= n_samples[nclass];
				_std_dev[nclass][nfeat] = sqrt (_std_dev[nclass][nfeat] / (1.0f * n_samples[nclass]) - _means[nclass][nfeat] * _means[nclass][nfeat]);
			}
			_class_probs[nclass] = 1.0f * n_samples[nclass] / n_samples[_n_classes];
		}
	}

	init_class_names (samples);
	delete[] n_samples;

	return true;
}

bool NaiveBayes::forward (ssi_stream_t &stream,
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

	ssi_real_t sum = 0;
	bool prior = _options.prior;
	if (_options.log) {

		for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
			ssi_real_t prob = prior ? SSI_NAIVEBAYES_LOG(_class_probs[nclass]) : 0;
			ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream.ptr);
			ssi_real_t temp = 0;
			for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
				if (_std_dev[nclass][nfeat] == 0) {
					ptr++;
    				continue;
				}
    			ssi_real_t sqr = _std_dev[nclass][nfeat] * _std_dev[nclass][nfeat];				
                temp = *ptr - _means[nclass][nfeat];
                ptr++;
				if (sqr !=0) {
	    			prob += -SSI_NAIVEBAYES_LOG(_std_dev[nclass][nfeat]) - (temp * temp)/(2*sqr);
				} else {
	    			prob += -SSI_NAIVEBAYES_LOG(_std_dev[nclass][nfeat]) - (temp * temp)/(2*FLT_MIN);
				}
			}
			probs[nclass] = exp (prob / _n_features);
			sum += probs[nclass];
		}
	
	} else {

		for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
			ssi_real_t norm_const = ssi_cast (ssi_real_t, sqrt (2.0 * 3.14159265358979323846));
			ssi_real_t prob = prior ? _class_probs[nclass] : 0;
			ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream.ptr);
			for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
                ssi_real_t diff = *ptr -_means[nclass][nfeat];
                ptr++;
				ssi_real_t stddev = _std_dev[nclass][nfeat];
				if (stddev ==0) {
					stddev = FLT_MIN;
				}			
				ssi_real_t temp = (1 / (norm_const * stddev)) * exp(-((diff*diff) / (2 * (stddev*stddev))));
				prob *= temp;
			}
			probs[nclass] = prob;
			sum += probs[nclass];
		}
	}

	//normalisation
	if (sum == 0) {
		ssi_wrn ("sum == 0");
		for (ssi_size_t j = 0; j < n_probs; j++) {
			probs[j] = 1.0f / n_probs;
		}
	} else {
		for (ssi_size_t j = 0; j < n_probs; j++) {
			probs[j]/=sum;
		}
	}
	
	return true;
}

bool NaiveBayes::load (const ssi_char_t *filepath) {

	release ();
  
	char line[SSI_MAX_CHAR], *token;
  
    FILE *fp = ssi_fopen (filepath,"r");
	if (!fp) {
		ssi_wrn ("can't open file %s!", filepath);      
		return false;
	}

	do {
		readLine (fp, SSI_MAX_CHAR, line);
	} while (line[0] == '#');

	token = (char *) strtok(line,"\t");
	if (token) {
		_n_classes = atoi(token);
	} else {
		ssi_wrn ("can't read number of classes from svm classifier file '%s'!", filepath);
		return false;
	}

    token = (char *) strtok(NULL,"\t");
	if (token) {
		_n_features = atoi(token); 
	} else {
		ssi_wrn ("can't read feature dimension from svm classifier file '%s'!", filepath);
		return false;
	}

	_class_probs = new ssi_real_t[_n_classes];
	_class_names = new ssi_char_t *[_n_classes];
	_means = new ssi_real_t *[_n_classes];
	_std_dev = new ssi_real_t *[_n_classes];
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		_means[nclass] = new ssi_real_t[_n_features];
		_std_dev[nclass] = new ssi_real_t[_n_features];		
		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
			_means[nclass][nfeat] = 0;
			_std_dev[nclass][nfeat] = 0;
		}
		_class_probs[nclass] = 0;
	}

	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {

		do {
			readLine (fp, SSI_MAX_CHAR, line);
		} while (line[0] == '#' || line[0] == '\0');
		
		token = (char *) strtok(line,"\t");
		_class_names[nclass] = ssi_strcpy (token);
		token = (char *) strtok(NULL,"\n");
		_class_probs[nclass] = ssi_cast (float, atof (token));

		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
			readLine (fp, SSI_MAX_CHAR, line);
			sscanf (line, "%f%f", &_means[nclass][nfeat], &_std_dev[nclass][nfeat]);
		}
	}

	fclose(fp);

	return true;
}

bool NaiveBayes::save (const ssi_char_t *filepath) {

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}

	FILE *fp = ssi_fopen (filepath, "w");

	if (!fp) {
		ssi_wrn ("Cannot open file %s!",filepath);
		return false;
	};

	fprintf (fp, "# Classifier type:\tnaive_bayes\n# number of classes\tfeature space dimension\n%d\t%d\n\n# class\tprior class probability\n# mean\tstandard deviation\n", _n_classes, _n_features);

	for (ssi_size_t i = 0; i < _n_classes;i++) {
		fprintf (fp, "%s\t%g\n", _class_names[i], _class_probs[i]);
		for (ssi_size_t j = 0; j < _n_features; j++) {
			fprintf (fp, "%g\t%g\n", _means[i][j], _std_dev[i][j]);
		}
		fprintf(fp,"\n");	     
	}

	fclose(fp);

	return true;
}

bool NaiveBayes::readLine (FILE *fp, ssi_size_t num, ssi_char_t *string) {
	
	char *string_ptr = string;
	char c;
	c = getc (fp);
	while (c != '\n' && c != EOF) {
		if (c != '\r') {
			if (--num == 0) {
				*string_ptr = '\0';
				ssi_wrn ("input string to short");
				return false;
			}
            *string_ptr = c;
            string_ptr++;
		}
		c = getc (fp);
	}
	*string_ptr = '\0';

	if (ferror (fp)) {
		ssi_wrn ("readLine() failed");
		return false;
	}

	return true;
}

}
