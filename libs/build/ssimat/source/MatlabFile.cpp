// MatlabFile.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/04/23
// Copyright (C) University of Augsburg

#include "MatlabFile.h"

namespace ssi {

MatlabFile::MatlabFile (const ssi_char_t *filename,
	MatlabFile::MODE mode) 
	: _file (0),
	_filename (0),
	_mode (mode) {

	// copy filename
	_filename =	ssi_strcpy (filename);

	// open mat file
	switch (_mode) {
		case MatlabFile::READ:
			_file = matOpen (_filename, "r");
			break;
		case MatlabFile::WRITE:
			_file = matOpen (_filename, "w");
			break;
		case MatlabFile::UPDATE:
			_file = matOpen (_filename, "u");
			break;
		default:
			ssi_err ("invalid mode");
			break;
	}

	// check if operation was successful
	if (!_file) {
		ssi_err ("could not open file '%s'", filename);
	}

	// read variables
	mxArray *var_p = 0;
	const ssi_char_t *var_name = 0;
	while (var_p = matGetNextVariable (_file, &var_name)) {
		String string (var_name);
		MatlabVar *var = new MatlabVar (var_p);
		_vars.insert (var_pair_t (string, var));
	}
}

MatlabFile::~MatlabFile () {

	// close file
	matClose (_file);

	// clean up
	var_map_t::iterator iter;
	for (iter = _vars.begin(); iter != _vars.end(); iter++) {
		mxDestroyArray (iter->second->_var);
		delete iter->second;		
	}
	delete[] _filename;
}

ssi_size_t MatlabFile::getSize () {

	return ssi_cast (ssi_size_t, _vars.size ());
}

MatlabVar &MatlabFile::operator[] (ssi_char_t *name) {

	String string (name);
	MatlabVar *var = _vars[string];
	if (!var) {
		ssi_err ("invalid name");
	}
	return *var;
}

MatlabVar &MatlabFile::insert_with_replace (String &string, MatlabVar *var) {

	// check if var with this name already exists
	var_map_t::iterator it;
	it = _vars.find (string);

	// either insert or replace new var
	if (it == _vars.end ()) {
		_vars.insert (var_pair_t (string, var));
	} else {
		ssi_wrn ("a variable with name '%s' already exists and will be replaced", string.str ());
		MatlabVar *tmp = it->second;
		it->second = var;
		delete tmp;
	}

	return *var;
}

MatlabVar &MatlabFile::addDoubleVar (const ssi_char_t *name, ssi_size_t rows, ssi_size_t cols) {

	// create var
	mxArray *var_p = mxCreateNumericMatrix (rows, cols, mxDOUBLE_CLASS, mxREAL);
	String string (name);
	MatlabVar *var = new MatlabVar (var_p);

	return insert_with_replace (string, var);
}

MatlabVar &MatlabFile::addSingleVar (const ssi_char_t *name, ssi_size_t rows, ssi_size_t cols) {

	// create var
	mxArray *var_p = mxCreateNumericMatrix (rows, cols, mxSINGLE_CLASS, mxREAL );
	String string (name);
	MatlabVar *var = new MatlabVar (var_p);

	return insert_with_replace (string, var);
}

void MatlabFile::flush () {

	if (_mode == MatlabFile::READ) {
		ssi_err ("readonly");
	}

	var_map_t::iterator iter;
	for (iter = _vars.begin(); iter != _vars.end(); iter++) {		
		matPutVariable (_file, iter->first.str (), iter->second->_var);	
	}
}

void MatlabFile::print (FILE *file) {
	
	var_map_t::iterator iter;
	for (iter = _vars.begin(); iter != _vars.end(); iter++) {
		ssi_fprint (file, "%s:\n", iter->first.str ());	
		iter->second->print (file, 1);
		ssi_fprint (file, "\n");
	}
}

}

