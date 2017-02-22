// KmUtils.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/14
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

// See KMeans.h
//
// Author: David Arthur (darthur@gmail.com), 2009
// http://www.stanford.edu/~darthur/kmpp.zip

// Includes
#include "KmUtils.h"
#include "KMeans.h"
#include "KmTree.h"
#include "model/ModelTools.h"
#include "ioput/file/File.h"
#include "ISOverSample.h"
#include <sstream>
#include <time.h>
#include <vector>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

KMeans::KMeans (const ssi_char_t *file) 
	: _clusters (0),
	_n_clusters (0),
	_n_features (0),
	_n_samples (0),
	_assignments (0),
	_n_indices_per_cluster (0),
	_indices_per_cluster (0),
	_norm (false),
	_norm_as (0),
	_norm_bs (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

KMeans::~KMeans () {

	release ();

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool KMeans::train (ISamples &samples, ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}
	
	_n_clusters = _options.k;	
	_norm = _options.norm;
		
	ssi_size_t *classes;
	ssi_real_t **matrix;
	if (_options.smote) {
		ISOverSample sover (&samples);
		sover.setOver (ISOverSample::SMOTE);
		ModelTools::CreateSampleMatrix (sover, stream_index, _n_samples, _n_features, &classes, &matrix);
	} else {
		ModelTools::CreateSampleMatrix (samples, stream_index, _n_samples, _n_features, &classes, &matrix);
	}

	if (_norm) {		

		_norm_as = new ssi_real_t[_n_features];
		_norm_bs = new ssi_real_t[_n_features];
		ssi_size_t *minpos = new ssi_size_t[_n_features];
		ssi_size_t *maxpos = new ssi_size_t[_n_features];
		ssi_minmax (_n_samples, _n_features, matrix[0], _norm_as, minpos, _norm_bs, maxpos);
		ssi_real_t *aptr = _norm_as;
		ssi_real_t *bptr = _norm_bs;
		for (ssi_size_t j = 0; j < _n_features; j++) {						
			*bptr -= *aptr++;
			if (*bptr == 0.0f) {
				ssi_wrn ("found zero interval, set to 1");
				*bptr = 1.0f;
			}
			bptr++;
		}
		delete[] minpos;
		delete[] maxpos;

		ssi_real_t *ptr = matrix[0];
		for (ssi_size_t i = 0; i < _n_samples; i++) {
			ssi_real_t *aptr = _norm_as;
			ssi_real_t *bptr = _norm_bs;
			for (ssi_size_t j = 0; j < _n_features; j++) {
				*ptr -= *aptr++;		
				*ptr++ /= *bptr++;
			}
		}
	}

	ssi_real_t *clusters_data = new ssi_real_t[_n_clusters * _n_features];
	_clusters = new ssi_real_t *[_n_clusters];
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		_clusters[i] = clusters_data + i * _n_features;
	}
	_assignments = new int[_n_samples];

	ssi_real_t cost;
	int n = ssi_cast (int, _n_samples);
	int k = ssi_cast (int, _n_clusters);
	int d = ssi_cast (int, _n_features);
	int attempts = ssi_cast (int, _options.iter);
	if(_options.random_seed){
		srand ( ssi_size_t(time(NULL)) );
	}else{
		srand ( _options.seed );
	}
	if (_options.pp) {
		cost = RunKMeansPlusPlus (n, k, d, matrix[0], attempts, _clusters[0], _assignments);
	} else {
		cost = RunKMeans (n, k, d, matrix[0], attempts, _clusters[0], _assignments);
	}

	// DEBUG
/*
	File *km_debug = File::CreateAndOpen (File::BINARY, File::WRITE, "km_debug");
	km_debug->write (&_n_samples, sizeof (_n_samples), 1);
	km_debug->write (&_n_features, sizeof (_n_features), 1);
	ssi_size_t n_classes = samples.getClassSize ();
	km_debug->write (&n_classes, sizeof (n_classes), 1);
	km_debug->write (&k, sizeof (k), 1);
	km_debug->write (matrix[0], sizeof (matrix[0][0]), _n_samples * _n_features);	
	km_debug->write (classes, sizeof (classes[0]), _n_samples);
	km_debug->write (_clusters[0], sizeof (_clusters[0][0]), k * _n_features);
	km_debug->write (_assignments, sizeof (_assignments[0]), k);
	delete km_debug;
*/
	// DEBUG

	ModelTools::ReleaseSampleMatrix (_n_samples, classes, matrix);

	_n_indices_per_cluster = new ssi_size_t[_n_clusters];
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		_n_indices_per_cluster[i] = 0;
	}
	for (ssi_size_t i = 0; i < _n_samples; i++) {
		_n_indices_per_cluster[_assignments[i]]++;
	}
	_indices_per_cluster = new ssi_size_t *[_n_clusters];
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		_indices_per_cluster[i] = new ssi_size_t[_n_indices_per_cluster[i]];
	}
	ssi_size_t *count = new ssi_size_t[_n_clusters];
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		count[i] = 0;
	}
	for (ssi_size_t i = 0; i < _n_samples; i++) {
		_indices_per_cluster[_assignments[i]][count[_assignments[i]]++] = i;		
	}
	delete[] count;

	return true;
}

bool KMeans::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_clusters) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_probs != _n_clusters) {
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

	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream.ptr);
	
	if (_norm) {
		ssi_real_t *clone = new ssi_real_t[_n_features];
		memcpy (clone, stream.ptr, sizeof (ssi_real_t) * _n_features);
		ptr = clone;
		
		ssi_real_t *aptr = _norm_as;
		ssi_real_t *bptr = _norm_bs;
		for (ssi_size_t j = 0; j < _n_features; j++) {
			*ptr -= *aptr++;		
			*ptr++ /= *bptr++;
		}

		ptr = clone;
	}


	ssi_real_t sum = 0;
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		probs[i] = 1.0f / KMeans_PointDistSq (ptr, _clusters[i], _n_features);
		sum += probs[i];
	}
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		probs[i] /= sum;
	}

	if (_norm) {
		delete[] ptr;
	}

	return true;
}

bool KMeans::load (const ssi_char_t *filepath) {

	release ();

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	file->read (&_options.k, sizeof (_options.k), 1);
	file->read (&_options.iter, sizeof (_options.iter), 1);
	file->read (&_options.pp, sizeof (_options.pp), 1);
	file->read (&_n_features, sizeof (_n_features), 1);
	file->read (&_n_clusters, sizeof (_n_clusters), 1);		

	ssi_real_t *clusters_data = new ssi_real_t[_n_clusters * _n_features];
	_clusters = new ssi_real_t *[_n_clusters];
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		_clusters[i] = clusters_data + i * _n_features;
	}
	file->read (_clusters[0], sizeof (ssi_real_t), _n_clusters * _n_features);

	file->read (&_n_samples, sizeof (_n_samples), 1);
	_assignments = new int[_n_samples];
	file->read (_assignments, sizeof (_n_samples), _n_samples);
	_n_indices_per_cluster = new ssi_size_t[_n_clusters];
	file->read (_n_indices_per_cluster, sizeof (ssi_size_t), _n_clusters);
	_indices_per_cluster = new ssi_size_t *[_n_clusters];
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		_indices_per_cluster[i] = new ssi_size_t[_n_indices_per_cluster[i]];
		file->read (_indices_per_cluster[i], sizeof (ssi_size_t), _n_indices_per_cluster[i]);
	}

	// normalization
	_norm = _options.norm = false;
	if (file->read (&_options.norm, sizeof (_options.norm), 1)) {
		if (_options.norm) {
			_norm = true;
			_norm_as = new ssi_real_t[_n_features];
			_norm_bs = new ssi_real_t[_n_features];
			file->read (_norm_as, sizeof (ssi_real_t), _n_features);
			file->read (_norm_bs, sizeof (ssi_real_t), _n_features);
		}
	}

	delete file;

	return true;
}


bool KMeans::save (const ssi_char_t *filepath) {

	if (!_clusters) {
		ssi_wrn ("not trained");
		return false;
	}	

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&_options.k, sizeof (_options.k), 1);
	file->write (&_options.iter, sizeof (_options.iter), 1);
	file->write (&_options.pp, sizeof (_options.pp), 1);
	file->write (&_n_features, sizeof (_n_features), 1);
	file->write (&_n_clusters, sizeof (_n_clusters), 1);		
	file->write (_clusters[0], sizeof (ssi_real_t), _n_clusters * _n_features);
	file->write (&_n_samples, sizeof (_n_samples), 1);
	file->write (_assignments, sizeof (_n_samples), _n_samples);
	file->write (_n_indices_per_cluster, sizeof (ssi_size_t), _n_clusters);
	for (ssi_size_t i = 0; i < _n_clusters; i++) {
		file->write (_indices_per_cluster[i], sizeof (ssi_size_t), _n_indices_per_cluster[i]);
	}
	file->write (&_options.norm, sizeof (_options.norm), 1);
	if (_options.norm) {
		file->write (_norm_as, sizeof (ssi_real_t), _n_features);
		file->write (_norm_bs, sizeof (ssi_real_t), _n_features);
	}

	delete file;

	return true;
}

void KMeans::release () {

	delete[] _norm_as; _norm_as = 0;
	delete[] _norm_bs; _norm_bs = 0;
	_norm = false;
	if (_clusters) {
		delete[] _clusters[0];
		delete[] _clusters;
		_clusters = 0;
	}
	delete[] _assignments;
	_assignments = 0;
	delete[] _n_indices_per_cluster;
	_n_indices_per_cluster = 0;
	if (_indices_per_cluster) {
		for (ssi_size_t i = 0; i < _n_clusters; i++) {
			delete[] _indices_per_cluster[i];
		}
		delete[] _indices_per_cluster;
		_indices_per_cluster = 0;
	}
	_n_samples = 0;
	_n_clusters = 0;
	_n_features = 0;	
}



// Logging
static std::vector<std::ostream*> gLogOutputs;
static std::vector<std::ostream*> gVerboseLogOutputs;
#define LOG(verbose, text) {                                               \
  std::vector<std::ostream*> &outputs = (verbose? gVerboseLogOutputs : gLogOutputs); \
  if (outputs.size() > 0) {                                                \
    std::ostringstream string_stream;                                           \
    string_stream << text;                                                 \
    for (int i = 0; i < (int)outputs.size(); i++)                          \
      *(outputs[i]) << string_stream.str();                                \
  }                                                                        \
}
void KMeans::AddKMeansLogging(std::ostream *out, bool verbose) {
  if (verbose)
    gVerboseLogOutputs.push_back(out);
  gLogOutputs.push_back(out);
}
void KMeans::ClearKMeansLogging() {
  gLogOutputs.clear();
  gVerboseLogOutputs.clear();
}

// Returns the number of seconds since the program began execution.
static double GetSeconds() {
  return double(clock()) / CLOCKS_PER_SEC;
}

// See KMeans.h
// Performs one full execution of k-means, logging any relevant information, and tracking meta
// statistics for the run. If min or max values are negative, they are treated as unset.
// best_clusters and best_assignment can be 0, in which case they are not set.
static void RunKMeansOnce(const KmTree &tree, int n, int k, int d, Scalar *points, Scalar *clusters,
                          Scalar *min_cost, Scalar *max_cost, Scalar *total_cost,
                          double start_time, double *min_time, double *max_time,
                          double *total_time, Scalar *best_clusters, int *best_assignment) {
  const Scalar kEpsilon = Scalar(1e-8);  // Used to determine when to terminate k-means

  // Do iterations of k-means until the cost stabilizes
  Scalar old_cost = 0;
  bool is_done = false;
  for (int iteration = 0; !is_done; iteration++) {
    Scalar new_cost = tree.DoKMeansStep(k, clusters, 0);
    is_done = (iteration > 0 && new_cost >= (1 - kEpsilon) * old_cost);
    old_cost = new_cost;
    LOG(true, "Completed iteration #" << (iteration+1) << ", cost=" << new_cost << "..." << std::endl);
  }
  double this_time = GetSeconds() - start_time;

  // Log the clustering we found
  LOG(false, "Completed run: cost=" << old_cost << " (" << this_time << " seconds)" << std::endl);

  // Handle a new min cost, updating best_clusters and best_assignment as appropriate
  if (*min_cost < 0 || old_cost < *min_cost) {
    *min_cost = old_cost;
    if (best_assignment != 0)
      tree.DoKMeansStep(k, clusters, best_assignment);
    if (best_clusters != 0)
      memcpy(best_clusters, clusters, sizeof(Scalar)*k*d);
  }

  // Update all other aggregate stats
  if (*max_cost < old_cost) *max_cost = old_cost;
  *total_cost += old_cost;
  if (*min_time < 0 || *min_time > this_time)
    *min_time = this_time;
  if (*max_time < this_time) *max_time = this_time;
  *total_time += this_time;
}

// Outputs all meta-stats for a set of k-means or k-means++ runs.
void LogMetaStats(Scalar min_cost, Scalar max_cost, Scalar total_cost,
                  double min_time, double max_time, double total_time, int num_attempts) {
  LOG(false, "Aggregate info over " << num_attempts << " runs:" << std::endl);
  LOG(false, "  Cost: min=" << min_cost << " average=" << (total_cost / num_attempts)
          << " max=" << max_cost << std::endl);
  LOG(false, "  Time: min=" << min_time << " average=" << (total_time / num_attempts)
          << " max=" << max_time << std::endl << std::endl);
}

// See KMeans.h
Scalar KMeans::RunKMeans(int n, int k, int d, Scalar *points, int attempts,
                 Scalar *ret_clusters, int *ret_assignment) {
  KM_ASSERT(k >= 1);
  
  // Create the tree and log
  LOG(false, "Running k-means..." << std::endl);
  KmTree tree(n, d, points);
  LOG(false, "Done preprocessing..." << std::endl);

  // Initialization
  Scalar *clusters = (Scalar*)malloc(sizeof(Scalar)*k*d);
  int *unused_clusters = (int*)malloc(sizeof(int)*n);
  KM_ASSERT(clusters != 0 && unused_clusters != 0);
  Scalar min_cost = -1, max_cost = -1, total_cost = 0;
  double min_time = -1, max_time = -1, total_time = 0;
  
  // Handle k > n
  if (k > n) {
    memset(clusters + n*d, -1, (k-d)*sizeof(Scalar));
    k = n;
  }

  // Run all the attempts
  for (int attempt = 0; attempt < attempts; attempt++) {
    double start_time = GetSeconds();

    // Choose clusters uniformly at random
    for (int i = 0; i < n; i++)
      unused_clusters[i] = i;
    int num_unused_clusters = n;
    for (int i = 0; i < k; i++) {
      int j = KMeans_GetRandom(num_unused_clusters--);
      memcpy(clusters + i*d, points + unused_clusters[j]*d, d*sizeof(Scalar));
      unused_clusters[j] = unused_clusters[num_unused_clusters];
    }
    
    // Run k-means
    RunKMeansOnce(tree, n, k, d, points, clusters, &min_cost, &max_cost, &total_cost, start_time,
                  &min_time, &max_time, &total_time, ret_clusters, ret_assignment);
  }
  LogMetaStats(min_cost, max_cost, total_cost, min_time, max_time, total_time, attempts);

  // Clean up and return
  free(unused_clusters);
  free(clusters);
  return min_cost;
}

// See KMeans.h
Scalar KMeans::RunKMeansPlusPlus(int n, int k, int d, Scalar *points, int attempts,
                         Scalar *ret_clusters, int *ret_assignment) {
  KM_ASSERT(k >= 1);

  // Create the tree and log
  LOG(false, "Running k-means++..." << std::endl);
  KmTree tree(n, d, points);
  LOG(false, "Done preprocessing..." << std::endl);

  // Initialization
  Scalar *clusters = (Scalar*)malloc(sizeof(Scalar)*k*d);
  KM_ASSERT(clusters != 0);
  Scalar min_cost = -1, max_cost = -1, total_cost = 0;
  double min_time = -1, max_time = -1, total_time = 0;

  // Run all the attempts
  for (int attempt = 0; attempt < attempts; attempt++) {
    double start_time = GetSeconds();

    // Choose clusters using k-means++ seeding
    tree.SeedKMeansPlusPlus(k, clusters);
    
    // Run k-means
    RunKMeansOnce(tree, n, k, d, points, clusters, &min_cost, &max_cost, &total_cost, start_time,
                  &min_time, &max_time, &total_time, ret_clusters, ret_assignment);
  }
  LogMetaStats(min_cost, max_cost, total_cost, min_time, max_time, total_time, attempts);

  // Clean up and return
  free(clusters);
  return min_cost;
}

}
