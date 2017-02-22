// WavTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/01/28
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

#ifndef SSI_IOPUT_WAVTOOLS_H
#define SSI_IOPUT_WAVTOOLS_H

#include "SSI_Cons.h"
#include "ioput/file/File.h"
#include "ioput/wav/WavHeader.h"

namespace ssi {

class WavTools {                                                                

public:

	static WAVEFORMATEX CreateFormat(ssi_stream_t &stream);
	static WavHeader CreateHeader(WAVEFORMATEX format, ssi_size_t n_samples);
	static WavChunkHeader CreateChunkHeader(WAVEFORMATEX format, ssi_size_t n_samples);

	static bool ReadWavHeader (File &file,
		WavHeader &header);
	static bool ReadWavChunk (File &file,
		WavChunkHeader &chunk);
	static bool ReadWavFile (const ssi_char_t *filename,
		ssi_stream_t &stream,
		bool convert2float = false);
	static bool ReadWavFile (File &file, 
		ssi_stream_t &stream,
		bool convert2float = false);
	
	static bool WriteWavHeader (File &file,
		WAVEFORMATEX format,
		ssi_size_t n_samples);
	static bool WriteWavChunk (File &file,
		WAVEFORMATEX format,		
		ssi_size_t n_samples);
	static bool WriteWavFile (const ssi_char_t *filename, 
		ssi_stream_t &stream);
	static bool WriteWavFile (File &file, 
		ssi_stream_t &stream);

	static ssi_byte_t *WriteWavMemory(ssi_stream_t &stream, ssi_size_t &n_bytes);
	static ssi_byte_t *WriteWavChunkMemory(ssi_stream_t &stream, ssi_size_t &n_bytes);

	static bool ConvertVXtoWAV (const ssi_char_t *filename);
	static bool ConvertWAVtoVX (const ssi_char_t *filename, 
		File::VERSION version = File::DEFAULT_VERSION);
	static bool RepairWavFile (const ssi_char_t *path,
		ssi_stream_t &data);

protected:

	static ssi_char_t *ssi_log_name;
};

}

#endif
