// Evaluation2Latex.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/01/19
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

/*
 
lets you evaluate a model
   
*/

#pragma once

#ifndef SSI_MODEL_EVALUATION2LATEX_H
#define SSI_MODEL_EVALUATION2LATEX_H

#include "Evaluation.h"
#include "ioput/file/File.h"

namespace ssi {

class Evaluation2Latex {

public:

	Evaluation2Latex ();
	~Evaluation2Latex ();

	bool open (const ssi_char_t *filename);
	bool close ();

	bool set (FILE *file) { _file = file; };
	FILE *get () { return _file; };
	
	bool writeHead (Evaluation &eval, const ssi_char_t *caption, const ssi_char_t *label);	
	bool writeText (const ssi_char_t *text, bool add_rules);
	bool writeRule ();	
	bool writeEval (const ssi_char_t *name, Evaluation &eval);
	bool writeTail ();

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	FILE *_file;
	ssi_size_t _n_classes;
	ssi_size_t _n_columns;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
};

}

#endif
