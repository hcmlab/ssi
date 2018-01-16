// KMeans.h
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

// BEWARE: BETA VERSION
// --------------------
//
// The main set of utilities for runnning k-means and k-means++ on arbitrary data sets.
//
// Author: David Arthur (darthur@gmail.com), 2009
// http://www.stanford.edu/~darthur/kmpp.zip

#ifndef SSI_MODEL_KMEANS_H
#define SSI_MODEL_KMEANS_H

// Includes
#include "base/IModel.h"
#include "KmUtils.h"
#include "ioput/option/OptionList.h"
#include <iostream>

namespace ssi {

class KMeans : public IModel {	

public:

	class Options : public OptionList {

	public:

		Options () : k (5), iter (1), pp (true), smote (false), norm (false), random_seed(true), seed(1) {

			addOption ("k", &k, 1, SSI_SIZE, "number of clusters");			
			addOption ("iter", &iter, 1, SSI_INT, "number of times to independently run k-means with different starting clusters");			
			addOption ("pp", &pp, 1, SSI_BOOL, "use kmeans++ instead of kmeans");
			addOption ("smote", &smote, 1, SSI_BOOL, "use smote to over sample under represented classes");
			addOption ("norm", &norm, 1, SSI_BOOL, "apply normalization in interval [-1,1]");
			addOption ("random_seed", &random_seed, 1, SSI_BOOL, "apply random seed to center selection");
			addOption ("seed", &seed, 1, SSI_INT, "apply seed when not using randomized center selection");
		};

		ssi_size_t k;
		ssi_size_t iter;
		bool pp;
		bool smote;
		bool norm;
		bool random_seed;
		ssi_size_t seed;
	};

public:

	static const ssi_char_t *GetCreateName () { return "KMeans"; };
	static IObject *Create (const ssi_char_t *file) { return new KMeans (file); };	
	virtual ~KMeans ();
	KMeans::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "K-Means++ implementation."; };

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
	ssi_size_t getClassSize () { return _n_clusters; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _clusters != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence);
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);
	
	ssi_size_t getClusterSize () { return _n_clusters; }; // number of clusters	
	ssi_real_t *const*getClusters () { return _clusters; }; // clusters	
	ssi_size_t getAssignmentsSize () { return _n_samples; };
	const int *getAssignments () { return _assignments; }; // assigments
	ssi_size_t getIndicesPerClusterSize (ssi_size_t cluster) { return _n_indices_per_cluster[cluster]; }; // number of training samples in cluster
	const ssi_size_t *getIndicesPerCluster (ssi_size_t cluster) { return _indices_per_cluster[cluster]; }; // indices per cluster
	
protected:

	KMeans (const ssi_char_t *file = 0);
	KMeans::Options _options;
	ssi_char_t *_file;
	static const ssi_char_t *_name;
	static const ssi_char_t *_info;

	ssi_size_t _n_features;
	ssi_size_t _n_clusters;
	ssi_real_t **_clusters;
	ssi_size_t _n_samples;
	int *_assignments;
	ssi_size_t *_n_indices_per_cluster;
	ssi_size_t **_indices_per_cluster;

	bool _norm;
	ssi_real_t *_norm_as;
	ssi_real_t *_norm_bs;

protected:

	// Sets preferences for how much logging is done and where it is outputted, when k-means is run.
	static void ClearKMeansLogging();
	static void AddKMeansLogging(std::ostream *out, bool verbose);

	// Runs k-means on the given set of points.
	//   - n: The number of points in the data set
	//   - k: The number of clusters to look for
	//   - d: The number of dimensions that the data set lives in
	//   - points: An array of size n*d where points[d*i + j] gives coordinate j of point i
	//   - attempts: The number of times to independently run k-means with different starting clusters.
	//               The best result is always returned (as measured by the cost function).
	//   - clusters: This can either be null or an array of size k*d. In the latter case, it will be
	//              filled with the locations of all final cluster clusters. Specifically
	//              clusters[d*i + j] will give coordinate j of center i. If the cluster is unused, it
	//              will contain NaN instead.
	//   - assignments: This can either be null or an array of size n. In the latter case, it will be
	//                  filled with the cluster that each point is assigned to (an integer between 0
	//                  and k-1 inclusive).
	// The final cost of the clustering is also returned.
	static Scalar RunKMeans(int n, int k, int d, Scalar *points, int attempts,
					 Scalar *clusters, int *assignments);

	// Runs k-means++ on the given set of points. Set RunKMeans for info on the parameters.
	static Scalar RunKMeansPlusPlus(int n, int k, int d, Scalar *points, int attempts,
							 Scalar *clusters, int *assignments);

};

}

#endif
