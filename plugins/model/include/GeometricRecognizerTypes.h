// GeometricRecognizerTypes.h
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

#ifndef SSI_MODEL_GEOMETRICRECOGNIZERTYPES_H
#define SSI_MODEL_GEOMETRICRECOGNIZERTYPES_H

/*
* This code taken (and modified) from the javascript version of:
* The $1 Gesture Classifier
*
*		Jacob O. Wobbrock
* 		The Information School
*		University of Washington
*		Mary Gates Hall, Box 352840
*		Seattle, WA 98195-2840
*		wobbrock@u.washington.edu
*
*		Andrew D. Wilson
*		Microsoft Research
*		One Microsoft Way
*		Redmond, WA 98052
*		awilson@microsoft.com
*
*		Yang Li
*		Department of Computer Science and Engineering
* 		University of Washington
*		The Allen Center, Box 352350
*		Seattle, WA 98195-2840
* 		yangli@cs.washington.edu
*/

#include "SSI_Cons.h"
#include <list>

namespace ssi
{
	class Point2D
	{
	public:
		//--- Wobbrock used doubles for these, not ints
		//int x, y;
		double x, y;
		Point2D() 
		{
			this->x=0; 
			this->y=0;
		}
		Point2D(double x, double y)
		{
			this->x = x;
			this->y = y;
		}
	};

	typedef std::vector<Point2D>  Path2D;
	typedef Path2D::iterator Path2DIterator;

	class Rectangle
	{
	public:
		double x, y, width, height;
		Rectangle(double x, double y, double width, double height)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}
	};

	class RecognitionResult
	{
	public:
		int gesture_id;
		double score;
		RecognitionResult(int gesture_id, double score)
		{
			this->gesture_id = gesture_id;
			this->score = score;
		}
	};
}
#endif
