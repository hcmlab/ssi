// MatlabVar.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/04/23
// Copyright (C) University of Augsburg

#include "MatlabVar.h"

namespace ssi {

MatlabVar::MatlabVar (mxArray *var)
	: _var (var),
	_string (0),
	rows (var == 0 ? 0 : ssi_cast (ssi_size_t, mxGetM (var))),
	cols (var == 0 ? 0 : ssi_cast (ssi_size_t, mxGetN (var))),
	elems (var == 0 ? 0 : ssi_cast (ssi_size_t, mxGetM (var)) * ssi_cast (ssi_size_t, mxGetN (var))),
	bytes (var == 0 ? 0 : ssi_cast (ssi_size_t, mxGetElementSize (var))) {

	// determine type
	if (!var) {
		_type = MatlabVar::EMPTY;	
	} else if (mxIsNumeric (_var)) {
		_type = MatlabVar::NUMERIC;
	} else if (mxIsChar (_var)) {
		_type = MatlabVar::STRING;
		_string = mxArrayToString (_var);
	} else if (mxIsCell (_var)) {
		_type = MatlabVar::CELL;
		mxArray *cell_var;
		for (ssi_size_t i = 0; i < rows * cols; i++) {
			cell_var = mxGetCell (_var, i); 
			_cells.push_back (new MatlabVar (cell_var));
		}
	} else if (mxIsStruct (_var)) {
		_type = MatlabVar::STRUCT;
		int n_fields = mxGetNumberOfFields (_var);
		for (int i = 0; i < n_fields; i++) {
			const char *name = mxGetFieldNameByNumber (_var, i);
			String string (name);
			var_vec_t vars;
			for (ssi_size_t j = 0; j < rows * cols; j++) {
				mxArray *next_var_p = mxGetFieldByNumber (_var, j, i);
				MatlabVar *next_var = new MatlabVar (next_var_p);
				vars.push_back (next_var);
			}
			_struct.insert (var_vec_pair_t (string, vars));
		}
	} else {
		ssi_err ("invalid type");
	}
}

MatlabVar::~MatlabVar () {

	// clean up
	for (ssi_size_t i = 0; i < _cells.size (); i++) {	
		delete _cells[i];		
	}
	var_vec_map_t::iterator iter;
	for (iter = _struct.begin(); iter != _struct.end(); iter++) {		
		for (ssi_size_t i = 0; i < iter->second.size (); i++) {
			delete iter->second[i];		
		}
	}
	mxFree (_string);
}

void *MatlabVar::getPtr () {

	if (_type == MatlabVar::EMPTY) {
		return 0;
	}

	return mxGetData (_var);
}

void MatlabVar::getScalar (double &scalar,
	ssi_size_t offset) {

	if (_type != MatlabVar::NUMERIC) {
		ssi_err ("invalid type");
	}

	if (mxGetElementSize (_var) != sizeof (double)) {
		ssi_err ("invalid byte size");
	}

	if (offset >= rows * cols) {
		ssi_err ("invalid offset");
	}
	
	scalar = *(ssi_pcast (double, mxGetData (_var)) + offset);
}

void MatlabVar::getScalar (float &scalar,
	ssi_size_t offset) {

	if (_type != MatlabVar::NUMERIC) {
		ssi_err ("invalid type");
	}

	if (mxGetElementSize (_var) != sizeof (float)) {
		ssi_err ("invalid byte size");
	}

	if (offset >= rows * cols) {
		ssi_err ("invalid offset");
	}

	scalar = *(ssi_pcast (float, mxGetData (_var)) + offset);
}

MatlabVar &MatlabVar::getCell (ssi_size_t index) {

	if (_type != MatlabVar::CELL) {
		ssi_err ("invalid type");
	}

	if (index >= _cells.size ()) {
		ssi_err ("invalid index '%u'", index);
	}

	return *_cells[index];
}

const ssi_char_t *MatlabVar::getString () {

	if (_type != MatlabVar::STRING) {
		ssi_err ("invalid type");
	}

	return _string;
}

MatlabVar &MatlabVar::getField (const ssi_char_t *name, ssi_size_t index) {

	if (_type != MatlabVar::STRUCT) {
		ssi_err ("invalid type");
	}

	var_vec_map_t::iterator it;
	it = _struct.find (name);

	if (it == _struct.end ()) {
		ssi_err ("invalid field name '%s'", name);
	}

	if (index >= _struct[name].size ()) {
		ssi_err ("invalid index '%u'", index);
	}

	return *(it->second[index]);
}

void MatlabVar::write (const void *ptr, ssi_size_t size, ssi_size_t count) {

	if (_type != MatlabVar::NUMERIC) {
		ssi_err ("invalid type");
	}

	if (mxGetElementSize (_var) != size) {
		ssi_err ("invalid byte size");
	}

	if (count != elems) {
		ssi_err ("invalid element size");
	}
	
	memcpy (mxGetData (_var), ptr, elems * size);
}

void MatlabVar::read (void *ptr, ssi_size_t size, ssi_size_t count) {

	if (_type != MatlabVar::NUMERIC) {
		ssi_err ("invalid type");
	}

	if (mxGetElementSize (_var) != size) {
		ssi_err ("invalid byte size");
	}

	if (count != elems) {
		ssi_err ("invalid element size");
	}

	memcpy (ptr, mxGetData (_var), elems * size);
}

void MatlabVar::print (FILE *file, ssi_size_t level) {

	for (ssi_size_t i = 0; i < level; i++) {
		ssi_fprint (file, "\t");
	}
	switch (_type) {
		case MatlabVar::NUMERIC:
			ssi_fprint (file, "num    %ux%u\n", rows, cols);
			break;
		case MatlabVar::STRING:
			ssi_fprint (file, "char   %ux%u\n", rows, cols);
			break;
		case MatlabVar::CELL:
			ssi_fprint (file, "cell   %ux%u\n", rows, cols);
			break;
		case MatlabVar::STRUCT:
			ssi_fprint (file, "struct %ux%u\n", rows, cols);
			break;
		case MatlabVar::EMPTY:
			ssi_fprint (file, "empty\n");
			break;
	}

	for (ssi_size_t i = 0; i < _cells.size (); i++) {	
		_cells[i]->print (file, level+1);
	}

	var_vec_map_t::iterator it;
	for (it = _struct.begin (); it != _struct.end (); it++) {	
		for (ssi_size_t i = 0; i < it->second.size (); i++) {	
			it->second[i]->print (file, level+1);
		}
	}
}

void MatlabVar::printDouble (FILE *file) {

	if (_type != MatlabVar::NUMERIC) {
		ssi_err ("invalid type");
	}

	double *ptr = ssi_pcast (double, mxGetData (_var));
	for (size_t i = 0; i < rows; i++) {		
		for (size_t j = 0; j < cols; j++) {		
			printf ("%lf ", *ptr++);
		}
		printf ("\n");
	}
}

void MatlabVar::printSingle (FILE *file) {

	if (_type != MatlabVar::NUMERIC) {
		ssi_err ("invalid type");
	}

	float *ptr = ssi_pcast (float, mxGetData (_var));
	for (size_t i = 0; i < rows; i++) {		
		for (size_t j = 0; j < cols; j++) {		
			printf ("%f ", *ptr++);
		}
		printf ("\n");
	}
}

}
