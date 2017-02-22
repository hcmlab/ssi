// FileCSV.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/02/20
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#pragma once

#ifndef SSI_IOPUT_FILESCV_H
#define SSI_IOPUT_FILESCV_H

#include "SSI_Cons.h"

namespace ssi {

class FileCSV {

public:

	class Column : public std::vector < ssi_char_t * > {
	};

public:

	FileCSV();
	virtual ~FileCSV();

	bool parseFile (const ssi_char_t *filepath, const ssi_char_t delim, bool has_header);
	bool parseString(const ssi_char_t *str, const ssi_char_t delim, bool has_header);
	void clear();

	bool hasHeader();
	ssi_size_t getRowSize();
	ssi_size_t getColumnSize();
	const ssi_char_t *getColumnName(ssi_size_t index);

	Column &operator[] (ssi_size_t index);
	Column &operator[] (const ssi_char_t *name);

	void print(FILE *file = ssiout);

protected:

	bool _has_header;
	ssi_size_t _n_rows;
	ssi_size_t _n_columns;
	ssi_char_t **_column_names;
	Column *_columns;

	ssi_char_t *nextLine(ssi_char_t *str, ssi_char_t **line, ssi_size_t &n_line, ssi_size_t &n_line_max);	
};

}

#endif
