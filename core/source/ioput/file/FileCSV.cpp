// FileCSV.cpp
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

#include "ioput/file/FileCSV.h"
#include "ioput/file/FileTools.h"

namespace ssi {

FileCSV::FileCSV() 
	: _has_header(false),
	_n_columns (0),
	_n_rows(0),
	_column_names(0),
	_columns(0) {
}

FileCSV::~FileCSV() {
	clear();
}

void FileCSV::clear () {
	
	for (ssi_size_t i = 0; i < _n_columns; i++) {
		for (ssi_size_t j = 0; j < _n_rows; j++) {		
			delete[] _columns[i][j];
		}
		_columns[i].clear();
		delete[] _column_names[i];
	}
	delete[] _column_names; _column_names = 0;
	delete[] _columns;
	_n_columns = _n_rows = 0;
	_has_header = false;
}

bool FileCSV::hasHeader() {
	return _has_header;
}

ssi_size_t FileCSV::getColumnSize() {
	return _n_columns;
}


ssi_size_t FileCSV::getRowSize() {
	return _n_rows;
}

const ssi_char_t *FileCSV::getColumnName(ssi_size_t index) {

	if (index > _n_columns) {
		ssi_err("#index '%u' exceeds #columns '%u'", index, _n_columns);
	}

	return _column_names[index];
}

FileCSV::Column &FileCSV::operator[] (ssi_size_t index) {

	if (index > _n_columns) {
		ssi_err("#index '%u' exceeds #columns '%u'", index, _n_columns);
	}

	return _columns[index];
}

FileCSV::Column &FileCSV::operator[] (const ssi_char_t *name) {

	if (_n_columns == 0) {
		ssi_err("empty, nothing to return");
	}

	for (ssi_size_t i = 0; i < _n_columns; i++) {
		if (ssi_strcmp(_column_names[i], name)) {
			return _columns[i];
		}
	}

	ssi_wrn("invalid column name '%s', return 1st column", name);
	return _columns[0];
}

bool FileCSV::parseFile(const ssi_char_t *filepath, const ssi_char_t delim, bool has_header) {

	ssi_size_t n_str;
	ssi_char_t *str = FileTools::ReadAsciiFile(filepath, n_str);

	bool result = false;
	if (str && n_str > 0) {
		result = parseString(str, delim, has_header);
	} else {
		ssi_wrn("empty file '%s'", filepath);		
	}

	delete[] str;
	return result;
}

bool FileCSV::parseString(const ssi_char_t *str, const ssi_char_t delim, bool has_header) {
	
	ssi_char_t *line = 0;
	ssi_size_t n_line = 0;
	ssi_size_t n_max_line = 0;
	ssi_char_t *ptr = ssi_ccast (ssi_char_t *, str);

	if (has_header) {
		if (ptr = nextLine(ptr, &line, n_line, n_max_line)) {
			ssi_size_t n_tokens = ssi_split_string_count(line, delim);
			if (n_tokens == 0){
				ssi_wrn("empty header");
				return false;
			}
 			_column_names = new ssi_char_t *[n_tokens];
			ssi_split_string(n_tokens, _column_names, line, delim);
			_n_columns = n_tokens;
			_columns = new Column[_n_columns];
			_has_header = true;
		} else {
			ssi_wrn("empty header");
			return false;
		}
	}

	ssi_size_t n_tokens = 0;
	ssi_char_t **tokens = 0;

	if (_n_columns > 0) {
		tokens = new ssi_char_t *[_n_columns];
		for (ssi_size_t i = 0; i < _n_columns; i++) {
			tokens[i] = 0;
		}
	}
	while (ptr = nextLine(ptr, &line, n_line, n_max_line)) {
		n_tokens = ssi_split_string_count(line, delim);
		if (n_tokens == 0) {
			continue;
		}
		if (_n_columns == 0) { // create dummy header
			_n_columns = n_tokens;		
			_column_names = new ssi_char_t *[_n_columns];
			for (ssi_size_t i = 0; i < _n_columns; i++) {
				_column_names[i] = new ssi_char_t[10];
				ssi_sprint(_column_names[i], "%u", i);
			}
			tokens = new ssi_char_t *[_n_columns];
			_columns = new Column[_n_columns];
		}
		if (_n_columns != n_tokens) {
			ssi_wrn("#tokens '%u' != #columns '%u'", n_tokens, _n_columns);
			continue;
		}
		if (!ssi_split_string(n_tokens, tokens, line, delim)) {
			continue;
		}
		for (ssi_size_t i = 0; i < _n_columns; i++) {
			_columns[i].push_back(tokens[i]);					
		}
		_n_rows++;
	}

	delete[] tokens;
	delete[] line;

	return true;

}

ssi_char_t *FileCSV::nextLine(ssi_char_t *str, ssi_char_t **line, ssi_size_t &n_line, ssi_size_t &n_line_max) {

	n_line = 0;
	ssi_char_t *ptr = str;

	if (!*ptr) {
		return 0;
	}

	if (ptr == '\0') {
		return 0;
	}

	while (*ptr != '\n' && *ptr != '\0') {
		ptr++;
		n_line++;
	}
	if (*ptr == '\n') {
		ptr++;
	}

	if (n_line > n_line_max) {
		n_line_max = n_line;
		delete[] *line;
		*line = new ssi_char_t[n_line_max + 1];		
	}
	memcpy(*line, str, n_line);
	(*line)[n_line] = '\0';

	return ptr;
}

void FileCSV::print(FILE *file) {

	if (_has_header) {
		for (ssi_size_t i = 0; i < getColumnSize(); i++) {
			ssi_fprint(file, "%s\t", getColumnName(i));
		}
		ssi_fprint(file, "\n");
	}
	
	for (ssi_size_t i = 0; i < getRowSize(); i++) {
		for (ssi_size_t j = 0; j < getColumnSize(); j++) {
			ssi_fprint(file, "%s\t", _columns[j][i]);
		}
		ssi_fprint(file, "\n");
	}
}

}

