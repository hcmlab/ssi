// FileProvider.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/07/23
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

#ifndef SSI_FILE_FILEPROVIDER_H
#define SSI_FILE_FILEPROVIDER_H

#include "base/IProvider.h"
#include "base/IConsumer.h"
#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class FileProvider : public IProvider {

public:

	FileProvider (IConsumer *writer,
		ITransformer *transformer = 0);
	~FileProvider ();

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return "FileProvider"; };
	const ssi_char_t *getInfo () { return "outputs stream to a file"; };

	void init (IChannel *channel);
	bool provide (ssi_byte_t *data, 
		ssi_size_t sample_number);

	void setMetaData (ssi_size_t size, const void *meta) {
		ssi_size_t meta_size = size;
		const void *meta_data = meta;
		if (_transformer) {
			_transformer->setMetaData (size, meta);
			meta_data = _transformer->getMetaData (meta_size);
		}
		if (meta_size > 0) {
			_writer->setMetaData (meta_size, meta_data);
		}
	};

protected:

	ITransformer *_transformer;
	ssi_stream_t _stream_t;

	IConsumer *_writer;
	IConsumer::info _info;
	ssi_stream_t _stream;

};

}



#endif
