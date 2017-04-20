// Evaluation2Latex.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/01/12
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

#include "Evaluation2Latex.h"
#include "ModelTools.h"
#include "ioput/file/File.h"
#include "ISSelectUser.h"
#include "ISSelectSample.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *Evaluation2Latex::ssi_log_name = "eval2latex";
int Evaluation2Latex::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

Evaluation2Latex::Evaluation2Latex ()
	: _file (0),
	_n_classes (0),
	_n_columns (0) {
}

Evaluation2Latex::~Evaluation2Latex () {
}

bool Evaluation2Latex::open (const ssi_char_t *filename) {

	_file = fopen (filename, "w");
	if (!_file) {
		ssi_wrn ("could not open file '%s' for writing", filename);
		return false;
	}

	return true;
}

bool Evaluation2Latex::close () {

	if (_file) {
		fclose (_file);
		_file = 0;
	} else {
		ssi_wrn ("empty file");
		return false;
	}	

	_n_classes = 0;
	_n_columns = 0;

	return true;
}

bool Evaluation2Latex::writeHead (Evaluation &eval, const ssi_char_t *caption, const ssi_char_t *label) {

	if (!_file) {
		ssi_wrn ("empty file");
		return false;
	}

	_n_classes = eval.get_class_size ();
	_n_columns = _n_classes + 2;

	ssi_fprint (_file, "\\begin{table}[h]\n");
	if (caption) {
		ssi_fprint (_file, "\\caption{%s}\n", caption);
	}
	if (label) {
		ssi_fprint (_file, "\\label{tab:%s}\n", label);
	}
	ssi_fprint (_file, "\\centering\n\\begin{tabular}{");
	for (ssi_size_t i = 0; i < _n_columns; i++) {
		ssi_fprint (_file, "c");
	}
	ssi_fprint (_file, "}\n\\toprule\n& \\multicolumn{%u}{c}{{\\em recognition results in \\%%}}\\\\\n\\cmidrule(r){2-%u}\\\n", _n_columns-1, _n_columns);
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		ssi_fprint (_file, "& %s ", eval.get_class_name (i));
	}
	ssi_fprint (_file, "& {\\bf average}\\\\\n");

	return true;
}

bool Evaluation2Latex::writeText (const ssi_char_t *text, bool add_rules) {

	if (!_file) {
		ssi_wrn ("empty file");
		return false;
	}

	if (add_rules) {
		writeRule ();
	}
	ssi_fprint (_file, "\\multicolumn{%u}{c}{%s}\\\\\n", _n_columns, text);
	if (add_rules) {
		writeRule ();
	}

	return true;
}

bool Evaluation2Latex::writeRule () {

	if (!_file) {
		ssi_wrn ("empty file");
		return false;
	}

	ssi_fprint (_file, "\\midrule\n");

	return true;
}

bool Evaluation2Latex::writeEval (const ssi_char_t *name, Evaluation &eval) {

	if (!_file) {
		ssi_wrn ("empty file");
		return false;
	}

	ssi_fprint (_file, "{\\bf %s} ", name);
	for (ssi_size_t i = 0; i < eval.get_class_size (); i++) {
		ssi_fprint (_file, "& %.2f ", eval.get_class_prob (i));
	}
	ssi_fprint (_file, "& {\\bf %.2f}\\\\\n", eval.get_classwise_prob ());	

	return true;
}

bool Evaluation2Latex::writeTail () {

	if (!_file) {
		ssi_wrn ("empty file");
		return false;
	}

	ssi_fprint (_file, "\\bottomrule\n\\end{tabular}\n\\end{table}\n");

	return true;
}



}
