// LibsoxFilter.h
// author: Andreas Seiderer
// created: 2013/02/08
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

#pragma once

#ifndef _LIBSOXFILTER_H
#define _LIBSOXFILTER_H

#include "base/IFilter.h"
#include "thread/Thread.h"
#include "thread/Event.h"
using namespace ssi;

#include "ioput/option/OptionList.h"

#include "sox.h"


#define SOX_MAX_CHAIN_STR_LEN 4096

/*
* allows 16 bit signed int and 32 bit float audio samples;

* the sample rate is set to the ssi stream sample rate;

* probably at least 4 channels (dimensions) can be used;

* 1 sox effect chain can be used;

* if there is an effect in the filter chain that needs more than one ssi sample block size at the beginning (for example noisered because it's using fft (2048 samples) + window overlap (1024 samples)) but then needs constantly only one sample block, you can use the sample offset; since an ssi filter needs always directly an output if it gets an input from ssi, the first output sample block will contain samples with the value 0; the offset needed for a specific effect chain has to be correctly adapted before otherwise errors will occur (e.g. offset too low: sox is not ready with an output but ssi awaits it, offset too high: the filter tries to put more samples than needed into the sox chain and doesn't handle the unexpected outputs of sox);

* working settings for noisered: sox_global_buffer = sox_signal_length = 1024, ssi_sample_block_offset = 1, wav->getOptions()->blockInSamples = 1024;

* pay attention that filters can cause clipping - you can use the effect gain with according parameters to reduce the volume at the beginning;

* if stereo (or n channels) is used you have to remember, that each dimension (channel) in ssi uses the set count of samples -> you have to double (multiply by n) the buffer sizes in the option file that worked with mono;

*/
class LibsoxFilter : public IFilter {

	class DrainThread : public Thread {
		public:
			DrainThread (LibsoxFilter *filter);
			~DrainThread ();
			void setParams (const char* effectArgStr, int soxGlobalBuf, int soxSignalLength, int sampleRate, bool is32BitFloat, int channels);
			void enter ();
			void run ();
			void flush ();

		private:
			sox_effects_chain_t * chain;
			const char* effectArgStr; 
			int soxGlobalBuf, soxSignalLength, sampleRate, channels;
			bool is32BitFloat;
			LibsoxFilter *filter;
	};

public:

	class Options : public OptionList {

		public:

			Options ()
			: sox_global_buf(4800),
			sox_signal_length(4800),
			ssi_sample_block_offset(0)
			{
				addOption("sox_global_buffer",&sox_global_buf, 1, SSI_UINT , "SoX doc \"sox_globals.bufsiz\": Default size (in bytes) used by libSoX for blocks of sample data. Plugins should use similarly-sized buffers to get best performance.");
				addOption("sox_signal_length",&sox_signal_length, 1, SSI_UINT , "in / out signal length (in samples) between SoX effects");
				addOption("sox_effect_chain_str",sox_effect_chain_str, SOX_MAX_CHAIN_STR_LEN, SSI_CHAR , "SoX effect chain arguments");
				addOption("ssi_sample_block_offset", &ssi_sample_block_offset, 1, SSI_UINT, "the offset of sample blocks from ssi after which 1 input block results always in 1 output block");

				sox_effect_chain_str[0] = '\0';
			}

			void setChain(const ssi_char_t *chain) {
				ssi_strcpy(sox_effect_chain_str, chain);
			}

			ssi_size_t sox_global_buf, sox_signal_length, ssi_sample_block_offset;
			ssi_char_t sox_effect_chain_str[SOX_MAX_CHAIN_STR_LEN];
	};


	static const ssi_char_t *GetCreateName () { return "LibsoxFilter"; };
	static IObject *Create (const ssi_char_t *file) { return new LibsoxFilter (file); };
	~LibsoxFilter ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "libsox filter"; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		return sample_dimension_in;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		return sample_type_in;
	}

protected:

	LibsoxFilter (const ssi_char_t *file = 0);
	static char ssi_log_name[];
	Options _options;
	ssi_char_t *_file;

	ssi_size_t ssi_sample_block_offset;

	static bool drainpipeline;
	static bool firstRun;

	static bool is32BitFloat;

	//check if sox effect chain input / output count > 1 -> ssi error
	static int soxReadCount;
	static int soxWriteCount;

	//counts the already passed offset blocks
	static int blockOffsetCount;

	static ssi_byte_t *ssi_sox_exchange;
	static ssi_size_t ssi_sox_exchange_bytes;

private:

	static int input_drain(sox_effect_t * effp, sox_sample_t * obuf, size_t * osamp);
	static sox_effect_handler_t const * input_handler(void);
	static int output_flow(sox_effect_t *effp LSX_UNUSED, sox_sample_t const * ibuf,sox_sample_t * obuf LSX_UNUSED, size_t * isamp, size_t * osamp);
	static sox_effect_handler_t const * output_handler(void);

	static Event _drain_in_event;
	static Event _drain_out_event;
	DrainThread *_drain_thread;


};

#endif
