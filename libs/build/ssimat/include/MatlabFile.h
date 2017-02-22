// MatlabFile.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/04/23
// Copyright (C) University of Augsburg

#pragma once 

#ifndef SSI_MATLABFILE_H
#define SSI_MATLABFILE_H

#include "MatlabVar.h"

namespace ssi {

class MatlabFile {

public:

	enum MODE {
		READ = 0,
		WRITE,
		UPDATE
	};

	MatlabFile (const ssi_char_t *filename, 
		MatlabFile::MODE mode);
	virtual ~MatlabFile ();

	ssi_size_t getSize ();

	MatlabVar &addDoubleVar (const ssi_char_t *name, ssi_size_t rows, ssi_size_t cols);
	MatlabVar &addSingleVar (const ssi_char_t *name, ssi_size_t rows, ssi_size_t cols);

	MatlabVar &operator[] (ssi_char_t *name);

	void flush ();

	void print (FILE *file);

protected:

	MatlabVar &insert_with_replace (String &string, MatlabVar *var);
		
	ssi_char_t *_filename;
	MATFile *_file;
	MatlabFile::MODE _mode;

	typedef std::map<String,MatlabVar *> var_map_t;
	typedef std::pair<String,MatlabVar *> var_pair_t;
	var_map_t _vars;
};

}

#endif
