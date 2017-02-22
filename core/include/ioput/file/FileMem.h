// FileMem.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/06/21
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

#pragma once

#ifndef SSI_IOPUT_FILEMEM_H
#define SSI_IOPUT_FILEMEM_H

#include "SSI_Cons.h"

namespace ssi {

class FileMem {

public:

	enum TYPE : unsigned char  {
		BINARY = 0,
		ASCII
	};
	static ssi_char_t *TYPE_NAMES[2];

	enum ORIGIN : unsigned char  {
		BEGIN = 0,  // SEEK_SET
		CURRENT,	// SEEK_CUR
		END			// SEEK_END
	};

public:

	static FileMem *Create (TYPE type,
		ssi_size_t n_memory = 0,
		ssi_byte_t *memory = 0,
		bool shallow = false);
	virtual ~FileMem ();

	bool open ();
	bool close ();
	void clear ();
	void make (ssi_size_t n_memory);
	void set (ssi_size_t n_memory, 
		ssi_byte_t *memory, 
		bool shallow = false);
	void *getMemory () {		
		return _memory;
	}
	ssi_size_t getPosition () {
		return _pos;
	}

	virtual ssi_size_t read (void *ptr, ssi_size_t size, ssi_size_t count) = 0;
	virtual ssi_size_t write (const void *ptr, ssi_size_t size, ssi_size_t count) = 0;
	virtual ssi_size_t readLine (ssi_size_t num, ssi_char_t *string) = 0;
	virtual ssi_size_t writeLine (const ssi_char_t *string) = 0;

	virtual void setFormat (const char *delim, const char *flags) {};
	virtual void setType (ssi_type_t type) {};

	bool seek (int32_t offset, FileMem::ORIGIN origin = FileMem::BEGIN);
	ssi_size_t tell ();
	bool ready ();

	bool isOpen () { return _memory != 0; };
	FileMem::TYPE getType () { return _type; };
	ssi_byte_t *getMemory (ssi_size_t &n_memory) { n_memory = _n_memory; return _memory; };

	static void SetLogLevel (int level) { ssi_log_level = level; };

protected:

	FileMem ();

	ssi_size_t move (ssi_size_t curpos);

	static int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	FileMem::TYPE _type;	
	ssi_size_t _pos;
	ssi_size_t _n_memory;
	ssi_byte_t *_memory;
	bool _shallow;
};

}

#endif
