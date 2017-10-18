// OnlineNaiveBayes.cpp
// author: Raphael Schwarz <raphaelschwarz@student.uni-augsbrug.de>
// created: 2017/04/12
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
// but WITHOUT ANY WARRANTY; without even the implied warranty of-+
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************
//
// class based on implementation of Naive Bayes of SSI Framework
// online algorithm based on Knuths recursive algorithm for calculating mean and standard dev.
// see 'The Art of Computer Programming, Volume 2 (3rd Ed.): Seminumerical Algorithms' Page

#include "OnlineNaiveBayes.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

OnlineNaiveBayes::OnlineNaiveBayes (const ssi_char_t *file) 
	: _n_features (0),
	_n_classes (0),
    _class_weight_sum(0),
	_class_probs (0),
    _class_names (0),
	_means (0),	
    _variance(0),
	_std_dev (0),
    _file (0){

	if (file) {
        if (!OptionList::LoadXML (file, &_options)) {
            OptionList::SaveXML (file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

OnlineNaiveBayes::~OnlineNaiveBayes () { 

	release ();
	if (_file) {
        OptionList::SaveXML (_file, &_options);
		delete[] _file;
	}
}

void OnlineNaiveBayes::release () {

	free_class_names ();
	
	delete[] _class_probs; _class_probs = 0;

	if (_means) {
		for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
			delete[] _means[nclass];
		}
		delete[] _means; _means = 0;
	}

    if (_variance) {
        for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
            delete[] _variance[nclass];
        }
        delete[] _variance; _variance = 0;
    }

	if (_std_dev) {
		for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
			delete[] _std_dev[nclass];
		}
		delete[] _std_dev; _std_dev = 0;
	}

    delete[] _class_weight_sum; _class_weight_sum = 0;

	_n_features = 0;
	_n_classes = 0;
}

void OnlineNaiveBayes::init_class_names (ISamples &samples) {

	free_class_names ();

	_n_classes = samples.getClassSize ();
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy (samples.getClassName (i));
	}
}

void OnlineNaiveBayes::free_class_names () {

	if (_class_names) {
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			delete[] _class_names[i];
		}
		delete[] _class_names;
		_class_names = 0;
	}	
}

bool OnlineNaiveBayes::train (ISamples &samples,
	ssi_size_t stream_index) {

    //Lock lock(_mutex);

    ssi_size_t tmp = samples.getSize();

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

    if (isTrained ()) {
        ssi_msg(SSI_LOG_LEVEL_DEFAULT, "start extending model");
    } else {

        //Initialize array for weight of classes
        _class_weight_sum = new ssi_size_t[samples.getClassSize()];
        for(int i = 0; i < samples.getClassSize(); i++){
            _class_weight_sum[i] = 0;
        }

        //Count of classes
        _n_classes = samples.getClassSize ();

        //Count of features
        _n_features = samples.getStream (stream_index).dim;


        //Array for mean value of each feature per class
        _means = new ssi_real_t *[_n_classes];

        //Array for variance of each feature per class
        _variance = new ssi_real_t *[_n_classes];

        //Standard deviation
        _std_dev = new ssi_real_t *[_n_classes];

        //Probability of each class
        _class_probs = new ssi_real_t[_n_classes];

        //Initialisation
        for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
            _means[nclass] = new ssi_real_t[_n_features];
            _variance[nclass] = new ssi_real_t[_n_features];
            _std_dev[nclass] = new ssi_real_t[_n_features];

            for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
                _means[nclass][nfeat] = 0;
                _variance[nclass][nfeat] = 0;
                _std_dev[nclass][nfeat] = 0;
            }
            _class_probs[nclass] = 0;
        }


    }

    samples.reset ();

    ssi_sample_t *sample = 0;
    ssi_real_t *ptr = 0;

    while (sample = samples.next ()) {

        //Sample value
        ptr = ssi_pcast (ssi_real_t, sample->streams[stream_index]->ptr);

        //Iterate through all features
        for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {

            ssi_real_t value = *ptr;
            ptr++;

            if(_class_weight_sum[sample->class_id] > 0) {
                ssi_size_t tmpWeightSum = _class_weight_sum[sample->class_id] + 1;
                ssi_real_t last_mean = _means[sample->class_id][nfeat];

                //current mean = sample weight * (sample value - last mean) / weight sum;
                //skip variable for sample weight cause in our case weight is always 1
                _means[sample->class_id][nfeat] += (value - last_mean)/tmpWeightSum;

                //std dev = sample weight * (sample value - last mean) * (value - actual mean)
                //skip variable for sample weight cause in our case weight is always 1
                _variance[sample->class_id][nfeat] += (value - last_mean) * (value - _means[sample->class_id][nfeat]);

            } else {
                _means[sample->class_id][nfeat] = value;
            }
        }

        //Increment for each sample
        _class_weight_sum[sample->class_id]++;

    }

    ssi_size_t weightSum = 0;

    for(int i = 0; i < _n_classes; i++){
        weightSum += _class_weight_sum[i];
    }

    //Calculation of standard deviation
    for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
        if (_class_weight_sum[nclass] > 0) {
            for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
                ssi_real_t tmpVariance = 0.0;
                //(_class_weight_sum[nclass]-1) not necessary because weights counting starts with 0
                //and not with 1 like in Knuth Art of Computer Programmin Vol. 2 Page 216
                tmpVariance = _variance[nclass][nfeat]/(_class_weight_sum[nclass]);
                _std_dev[nclass][nfeat] = sqrt (tmpVariance);
            }
            _class_probs[nclass] = 1.0f * _class_weight_sum[nclass] / weightSum;
        }
    }

    init_class_names (samples);

    return true;
}

bool OnlineNaiveBayes::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_probs != _n_classes) {
        ssi_wrn ("#classes differs %d!=%d",n_probs,_n_classes);
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
			ssi_real_t prob = prior ? SSI_ONLINENAIVEBAYES_LOG(_class_probs[nclass]) : 0;
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
	    			prob += -SSI_ONLINENAIVEBAYES_LOG(_std_dev[nclass][nfeat]) - (temp * temp)/(2*sqr);
				} else {
	    			prob += -SSI_ONLINENAIVEBAYES_LOG(_std_dev[nclass][nfeat]) - (temp * temp)/(2*FLT_MIN);
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

bool OnlineNaiveBayes::load (const ssi_char_t *filepath) {

	release ();
  
	char line[SSI_MAX_CHAR], *token;

    FILE *fp = /*File::CreateAndOpen(File::ASCII, File::READ, filepath);*/ssi_fopen (filepath,"r");
	if (!fp) {
		ssi_wrn ("can't open file %s!", filepath);      
		return false;
	}

	do {
        /*fp->readLine(SSI_MAX_CHAR,line);*/readLine (fp, SSI_MAX_CHAR, line);
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
    _class_weight_sum = new ssi_size_t[_n_classes];
    _variance = new ssi_real_t *[_n_classes];
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		_means[nclass] = new ssi_real_t[_n_features];
        _std_dev[nclass] = new ssi_real_t[_n_features];
        _variance[nclass] = new ssi_real_t[_n_features];
		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
			_means[nclass][nfeat] = 0;
			_std_dev[nclass][nfeat] = 0;
            _variance[nclass][nfeat] = 0;
		}
		_class_probs[nclass] = 0;
        _class_weight_sum[nclass] = 0;
	}

	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {

		do {
            /*fp->readLine(SSI_MAX_CHAR,line);*/readLine (fp, SSI_MAX_CHAR, line);
		} while (line[0] == '#' || line[0] == '\0');
		
        token = (char *) strtok(line,"\t");
        if(token){
            _class_names[nclass] = ssi_strcpy (token);
        } else {
            ssi_wrn ("can't read class name from svm classifier file '%s'!", filepath);
            return false;
        }


        token = (char *) strtok(NULL,"\n");
        _class_probs[nclass] = ssi_cast (float, atof (token));

        //Read number of used samples for current class
        /*fp->readLine(SSI_MAX_CHAR,line);*/readLine (fp, SSI_MAX_CHAR, line);
        token = (char *) strtok(line, "\n");
        _class_weight_sum[nclass] = ssi_cast (uint32_t, atoi(token));

		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
            /*fp->readLine(SSI_MAX_CHAR,line);*/readLine (fp, SSI_MAX_CHAR, line);
            sscanf (line, "%f%f%f", &_means[nclass][nfeat], &_variance[nclass][nfeat], &_std_dev[nclass][nfeat]);
        }

	}

    /*fp->close();*/fclose(fp);

	return true;
}

bool OnlineNaiveBayes::save (const ssi_char_t *filepath) {

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}

    FILE *fp = ssi_fopen (filepath, "w");

	if (!fp) {
		ssi_wrn ("Cannot open file %s!",filepath);
		return false;
	};

    fprintf (fp, "# Classifier type:\t%s\n# number of classes\tfeature space dimension\n%d\t%d\n\n# class\tprior class probability\n# samples used\n# mean\tvariance\tstandard deviation\n",GetCreateName(), _n_classes, _n_features);

	for (ssi_size_t i = 0; i < _n_classes;i++) {
		fprintf (fp, "%s\t%g\n", _class_names[i], _class_probs[i]);
        fprintf (fp, "%d\n", _class_weight_sum[i]);
		for (ssi_size_t j = 0; j < _n_features; j++) {
            fprintf (fp, "%g\t%g\t%g\n", _means[i][j], _variance[i][j], _std_dev[i][j]);
		}
		fprintf(fp,"\n");	     
	}

	fclose(fp);

	return true;
}

bool OnlineNaiveBayes::readLine (FILE *fp, ssi_size_t num, ssi_char_t *string) {
	
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
