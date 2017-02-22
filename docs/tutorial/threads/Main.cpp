// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/24
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

#include "ssi.h"
#include "MyThread.h"
using namespace ssi;

void ex_thread ();

int main () {

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	ex_thread ();

	ssi_print ("\n\n\tpress a key to quit\n");
	getchar ();

	return 0;
}

void ex_thread () {
	
	MyThread single ("single", 1000, true);
	MyThread multi_1 ("ping", 500, false);
	MyThread multi_2 ("pong", 300, false);

	single.start ();
	multi_1.start ();
	multi_2.start ();

	ssi_print ("\nPress enter to stop!\n");
	getchar ();

	single.stop ();
	multi_1.stop ();
	multi_2.stop ();	
}
