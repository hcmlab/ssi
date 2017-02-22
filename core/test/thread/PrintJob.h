// PrintJob.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/13
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "thread/Thread.h"
using namespace ssi;

#include "Printer.h"

class PrintJob : public Thread {

public:

	PrintJob (Printer *printer_, int len_)
		: printer (printer_), len (len_) {
		job = new char[len+1];
		job[len] = '\0';
	}

	~PrintJob () {
		if (job)
			delete job;
	};

	virtual void run () {
		for (int i = 0; i < len; i++)
			job[i] = (char) (rand() % 25 + 65);
		printer->print (job);
	}

private:

	int len;
	char *job;
	Printer *printer;

};
