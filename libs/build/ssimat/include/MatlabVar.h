// MatlabVar.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/04/23
// Copyright (C) University of Augsburg

#pragma once 

#ifndef SSI_MATLABVAR_H
#define SSI_MATLABVAR_H

#pragma comment(lib, "libmat.lib")
#pragma comment(lib, "libmx.lib")

#include "SSI_Cons.h"
#include "base/String.h"
#include <mat.h>

namespace ssi {

class MatlabVar {

	friend class MatlabFile;

protected:

	MatlabVar (mxArray *var);
	virtual ~MatlabVar ();

public:	

	enum TYPE {
		EMPTY,
		NUMERIC,
		STRING,
		CELL,
		STRUCT
	};
	
	const ssi_size_t rows;
	const ssi_size_t cols;
	const ssi_size_t elems;
	const ssi_size_t bytes;

	TYPE *getType ();
	void *getPtr ();

	void getScalar (double &scalar, 
		ssi_size_t offset = 0);
	void getScalar (float &scalar, 
		ssi_size_t offset = 0);

	MatlabVar &getCell (ssi_size_t index);
	MatlabVar &getField (const ssi_char_t *name, ssi_size_t index);
	const ssi_char_t *getString ();

	void write (const void *ptr, 
		ssi_size_t size,
		ssi_size_t count);
	void read (void *ptr,
		ssi_size_t size,
		ssi_size_t count);

	void printDouble (FILE *file);
	void printSingle (FILE *file);

protected:

	void print (FILE *file, ssi_size_t level);
	
	mxArray *_var;	
	TYPE _type;
	typedef std::vector<MatlabVar *> var_vec_t;
	var_vec_t _cells; // for cells
	typedef std::map<String,var_vec_t> var_vec_map_t;
	typedef std::pair<String,var_vec_t> var_vec_pair_t;
	var_vec_map_t _struct; // for structs
	ssi_char_t *_string; // for string content
};

}

#endif
