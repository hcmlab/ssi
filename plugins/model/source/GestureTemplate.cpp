// GestureTemplate.cpp
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

#include "GestureTemplate.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

GestureTemplate::GestureTemplate (int gesture_id, Path2D points)
	: gesture_id (gesture_id),
	points (points) {
}

GestureTemplate::GestureTemplate ()
	: gesture_id (UNKOWN_GESTURE_ID) {
}

void GestureTemplate::save (FILE *file) {

	Path2D::iterator it;
	ssi_size_t size = ssi_cast (ssi_size_t, points.size ());
	fwrite (&gesture_id, sizeof (gesture_id), 1, file);
	fwrite (&size, sizeof (size), 1, file);
	for (it = points.begin (); it != points.end (); it++) {
		fwrite (&it->x, sizeof (it->x), 1, file);
		fwrite (&it->y, sizeof (it->y), 1, file);
	}
}

void GestureTemplate::load (FILE *file) {

	points.clear ();
	ssi_size_t size = 0;
	fread (&gesture_id, sizeof (gesture_id), 1, file);
	fread (&size, sizeof (size), 1, file);
	for (ssi_size_t i = 0; i < size; i++) {
		Point2D point;
		fread (&point.x, sizeof (point.x), 1, file);
		fread (&point.y, sizeof (point.y), 1, file);
		points.push_back (point);
	}
}

}
