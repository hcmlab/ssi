// KmUtils.h
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
// Utilities for arbitrary dimensional points in space. All points are treated as simple value
// arrays. This is done for two reasons:
//   - Using value arrays instead of a point class makes all point operations very explicit, which
//     makes their usage easier to optimize.
//   - A value array is about as universal a format as possible, which makes it easier for
//     people to use the k-means code in any project.
// Also contains assertion code that can be disabled if desired.
//
// Author: David Arthur (darthur@gmail.com), 2009
// http://www.stanford.edu/~darthur/kmpp.zip

#ifndef SSI_MODEL_KM_UTILS_H
#define SSI_MODEL_KM_UTILS_H

// Includes
#include "SSI_Cons.h"

namespace ssi {

// The data-type used for a single coordinate for points
typedef ssi_real_t Scalar;

// Point utilities
// ===============

// Point creation and deletion
inline Scalar *KMeans_PointAllocate(int d) {
  return (Scalar*)malloc(d * sizeof(Scalar));
}

inline void KMeans_PointFree(Scalar *p) {
  free(p);
}

inline void KMeans_PointCopy(Scalar *p1, const Scalar *p2, int d) {
  memcpy(p1, p2, d * sizeof(Scalar));
}

// Point std::vector tools
inline void KMeans_PointAdd(Scalar *p1, const Scalar *p2, int d) {
  for (int i = 0; i < d; i++)
    p1[i] += p2[i];
}

inline void KMeans_PointScale(Scalar *p, Scalar scale, int d) {
  for (int i = 0; i < d; i++)
    p[i] *= scale;
}

inline Scalar KMeans_PointDistSq(const Scalar *p1, const Scalar *p2, int d) {
  Scalar result = 0;
  for (int i = 0; i < d; i++)
    result += (p1[i] - p2[i]) * (p1[i] - p2[i]);
  return result;
}

// Assertions
// ==========

// Comment out ENABLE_KMEANS_ASSERTS to turn off ASSERTS for added speed.
#ifdef _DEBUG
#define ENABLE_KMEANS_ASSERTS
#endif
#ifdef ENABLE_KMEANS_ASSERTS
int __KMeansAssertionFailure(const char *file, int line, const char *expression);
#define KM_ASSERT(expression) \
  (void)((expression) != 0? 0 : __KMeansAssertionFailure(__FILE__, __LINE__, #expression))
#else
#define KM_ASSERT(expression)
#endif

// Miscellaneous utilities
// =======================

// Returns a random integer chosen uniformly from the range [0, n-1]. Note that RAND_MAX could be
// less than n. On Visual Studio, it is only 32767. For larger values of RAND_MAX, we need to be
// careful of overflow.
inline int KMeans_GetRandom(int n) {
  int u = rand() * RAND_MAX + rand();
  return ((u % n) + n) % n;
}

}

#endif
