///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// ACADEMIC OR NON-PROFIT ORGANIZATION NONCOMMERCIAL RESEARCH USE ONLY
//
// BY USING OR DOWNLOADING THE SOFTWARE, YOU ARE AGREEING TO THE TERMS OF THIS LICENSE AGREEMENT.  
// IF YOU DO NOT AGREE WITH THESE TERMS, YOU MAY NOT USE OR DOWNLOAD THE SOFTWARE.
//
// License can be found in OpenFace-license.txt
//

// Precompiled headers stuff

#ifndef __STDAFX_h_
#define __STDAFX_h_

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

// dlib dependencies for face detection
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

// C++ stuff
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <vector>
#include <map>

#define _USE_MATH_DEFINES
#include <cmath>

// Boost stuff
#include <filesystem.hpp>
#include <filesystem/fstream.hpp>

// OpenBLAS stuff

#include <openblas_config.h>
// Instead of including cblas.h and f77blas.h (the definitions from OpenBLAS and other BLAS libraries differ, declare the required OpenBLAS functionality here)
#ifdef __cplusplus
extern "C" {
	/* Assume C declarations for C++ */
#endif  /* __cplusplus */

	/*Set the number of threads on runtime.*/
	void openblas_set_num_threads(int num_threads);

	void sgemm_(char *, char *, blasint *, blasint *, blasint *, float *,
		float  *, blasint *, float  *, blasint *, float  *, float  *, blasint *);
}


#endif
