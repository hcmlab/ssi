// GeometricRecognizer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/28
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

// *************************************************************************************************
//
// original source code by Baylor Wetzel <baylorw@yahoo.com>
// http://depts.washington.edu/aimgroup/proj/dollar/others/cpp.bw.zip
//
// Wobbrock, J.O., Wilson, A.D. and Li, Y. (2007) Gestures without libraries, toolkits
// or training: A $1 recognizer for user interface prototypes.
// <http://faculty.washington.edu/wobbrock/pubs/uist-07.1.pdf> Proceedings
// of the ACM Symposium on User Interface Software and Technology (UIST '07). Newport,
// Rhode Island (October 7-10, 2007). New York: ACM Press, pp. 159-168.
//
// *************************************************************************************************


#pragma once

#ifndef SSI_MODEL_GEOMETRICRECOGNIZER_H
#define SSI_MODEL_GEOMETRICRECOGNIZER_H

#include "GeometricRecognizerTypes.h"
#include "GestureTemplate.h"

namespace ssi
{
	class GeometricRecognizer
	{
	
	friend class Dollar$1;

	protected:
		//--- These are variables because C++ doesn't (easily) allow
		//---  constants to be floating point numbers
		double halfDiagonal;
		double angleRange;
		double anglePrecision;
		double goldenRatio;

		//--- How many points we use to define a shape
		int numPointsInGesture;
		//---- Square we resize the shapes to
		int squareSize;
		
		bool shouldIgnoreRotation;

		//--- What we match the input shape against
		GestureTemplates templates;

	public:
		GeometricRecognizer();

		int addTemplate(int gesture_id, Path2D points);
		Rectangle boundingBox(Path2D points);
		Point2D centroid(Path2D points);
		double getDistance(Point2D p1, Point2D p2);
		bool   getRotationInvariance() { return shouldIgnoreRotation; }
		double distanceAtAngle(Path2D points, GestureTemplate aTemplate, double rotation);
		double distanceAtBestAngle(Path2D points, GestureTemplate T);
		Path2D normalizePath(Path2D points);
		double pathDistance(Path2D pts1, Path2D pts2);
		double pathLength(Path2D points);
		RecognitionResult recognize(Path2D points);
		Path2D resample(Path2D points);
		Path2D rotateBy(Path2D points, double rotation);
		Path2D rotateToZero(Path2D points);
		Path2D scaleToSquare(Path2D points);
		void   setRotationInvariance(bool ignoreRotation);
		Path2D translateToOrigin(Path2D points);

		/// 2010.05.19 added by Johannes 
		bool recognizeBestPerClass (Path2D points, ssi_size_t n_classes, ssi_real_t *scores);

		void save (FILE *file);
		void load (FILE *file);
	};
}
#endif
