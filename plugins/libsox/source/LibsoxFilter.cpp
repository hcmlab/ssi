// LibsoxFilter.cpp
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


#include "LibsoxFilter.h"
#include "LibsoxFilterTools.h"

char LibsoxFilter::ssi_log_name[] = "libsoxfilt";

Event LibsoxFilter::_drain_in_event;
Event LibsoxFilter::_drain_out_event;
ssi_byte_t *LibsoxFilter::ssi_sox_exchange = 0;
ssi_size_t LibsoxFilter::ssi_sox_exchange_bytes = 0;
bool LibsoxFilter::drainpipeline;
bool LibsoxFilter::firstRun;
bool LibsoxFilter::is32BitFloat;
int LibsoxFilter::soxReadCount;
int LibsoxFilter::soxWriteCount;
int LibsoxFilter::blockOffsetCount;

LibsoxFilter::LibsoxFilter (const ssi_char_t *file)
	: _file (0),
	_drain_thread (0),
	ssi_sample_block_offset(0)
{
	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

}

LibsoxFilter::~LibsoxFilter () {
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void LibsoxFilter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {
	
	drainpipeline = false;
	firstRun = true;

	is32BitFloat = false;

	//check if sox effect chain input / output count > 1 -> ssi error
	soxReadCount = 0;
	soxWriteCount = 0;

	//counts the already passed offset blocks
	blockOffsetCount = 0;
		
	int sox_global_buf, sox_signal_length;
	ssi_char_t sox_effect_chain_str[SOX_MAX_CHAIN_STR_LEN];

	this->getOptions()->getOptionValue("sox_global_buffer",&sox_global_buf);
	this->getOptions()->getOptionValue("sox_signal_length",&sox_signal_length);
	this->getOptions()->getOptionValue("sox_effect_chain_str",&sox_effect_chain_str);
	this->getOptions()->getOptionValue("ssi_sample_block_offset", &ssi_sample_block_offset);

	blockOffsetCount = ssi_sample_block_offset;

	is32BitFloat = stream_in.type == SSI_FLOAT;

	if (is32BitFloat && stream_in.byte != 4) ssi_err("Invalid float sample size! %i instead of %i bytes.", stream_in.byte, 4 );
	if (!is32BitFloat && stream_in.byte != 2) ssi_err("Invalid int sample size! %i instead of %i bytes.", stream_in.byte, 2 );

	//set the parameters to the values from the option file / default values; the samplerate is set to the ssi stream samplerate; the sample type can be int_16 or float_32 and is also recieved from the ssi stream; the channel count from the input stream is used
	if (!_drain_thread) {
		_drain_thread = new LibsoxFilter::DrainThread (this);
	}
	_drain_thread->setParams(sox_effect_chain_str,sox_global_buf,sox_signal_length,stream_in.sr, is32BitFloat, stream_in.dim);
	_drain_thread->start ();

	ssi_msg (SSI_LOG_LEVEL_BASIC, "enter()..ok");
}

void LibsoxFilter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

		//buffer for data echange between static (libsox) and non static code
		ssi_sox_exchange_bytes = stream_in.byte * stream_in.dim * stream_in.num;

		//alloc only once
		if (ssi_sox_exchange == 0) {
			ssi_sox_exchange = new ssi_byte_t[ssi_sox_exchange_bytes];
		}

		if (blockOffsetCount > 0) {	//first blocks are needed to saturate the libsox pipeline -> direct output of blocks with samplevalue 0
			//ssi -> sox
			memcpy(ssi_sox_exchange, stream_in.ptr, ssi_sox_exchange_bytes);

			//let sox input handler work
			_drain_in_event.release ();
			_drain_out_event.wait ();
			//sox should now be ready

			for (int i = 0; i < ssi_sox_exchange_bytes; i++) {
				stream_out.ptr[i] = 0;
			}
		}
		else	//put the input stream data into the SoX effect chain, wait for output and copy the processed data back to the output stream
		{
			//ssi -> sox
			memcpy(ssi_sox_exchange, stream_in.ptr, ssi_sox_exchange_bytes);

			//let sox input handler work
			_drain_in_event.release ();
			_drain_out_event.wait ();
			//sox should now be ready

			//ssi <- sox
			memcpy(stream_out.ptr, ssi_sox_exchange, ssi_sox_exchange_bytes);
		}
}

void LibsoxFilter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	// drain the sox pipeline
	drainpipeline = true;
	_drain_in_event.release ();

	//wait for last sox output
	_drain_out_event.wait ();
	_drain_thread->stop();
	delete _drain_thread; _drain_thread = 0;

	delete[] ssi_sox_exchange; ssi_sox_exchange = 0;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "flush()..ok");
}


/* The function that will be called to input samples into the effects chain.*/
int LibsoxFilter::input_drain(
    sox_effect_t * effp, sox_sample_t * obuf, size_t * osamp)
{
	if (soxReadCount > 0) ssi_err("Sox tried to read more than one time -> this chain with current options isn't usable for ssi.");

	// wait for new input from ssi
	_drain_in_event.wait ();

	if (!drainpipeline) {

		if (is32BitFloat) *osamp = ssi_sox_exchange_bytes / 4;					//bytes were read but samples are needed -> assuming 32 bit float
		else *osamp = ssi_sox_exchange_bytes / 2;								//bytes were read but samples are needed -> assuming 16 bit int

		for (int i = 0; i < *osamp; i++) {
			if (is32BitFloat) {
				float sample = ((float*)ssi_sox_exchange)[i];					//get float 32 bit samples from the bytes
				obuf[i] = (sample)*(SOX_SAMPLE_MAX+1.);							//convert to the signed 32 bit internal sample format
			}
			else {
				sox_int16_t sample = ((sox_int16_t*)ssi_sox_exchange)[i];		//get signed 16 bit samples from the bytes
				obuf[i] = SOX_SIGNED_TO_SAMPLE(16,sample);						//convert to the signed 32 bit internal sample format
			}
		}
	} 
	else *osamp = 0;

	soxWriteCount = 0;

	//more than one read is only ok if there was a block offset
	if (blockOffsetCount == 0) soxReadCount++;
	if (blockOffsetCount > 0) {
		blockOffsetCount--;
		_drain_out_event.release ();
	}

	return *osamp? SOX_SUCCESS : SOX_EOF;
}


/* A `stub' effect handler to handle inputting samples to the effects
 * chain; the only function needed for this example is `drain' */
sox_effect_handler_t const * LibsoxFilter::input_handler(void)
{
	static sox_effect_handler_t handler = {
	"input", NULL, SOX_EFF_MCHAN, NULL, NULL, NULL, input_drain, NULL, NULL, 0
	};
	return &handler;
}


//always called once at the initialization;
/* The function that will be called to output samples from the effects chain.*/
int LibsoxFilter::output_flow(sox_effect_t *effp LSX_UNUSED, sox_sample_t const * ibuf,
    sox_sample_t * obuf LSX_UNUSED, size_t * isamp, size_t * osamp)
{
	*osamp = 0;																	//last chain member -> no output samples

	if (drainpipeline) return SOX_SUCCESS;										//on ssi shutdown no further output execution

	//back to ssi; if *isamp < ssi_sox_exchange_bytes -> fill with 0

	if (is32BitFloat) {
		float *sample = (float*)ssi_sox_exchange;
		for (int i = 0; i < ssi_sox_exchange_bytes/4; i++) {
			if (i < *isamp) {													//internally used signed 32 bit int -> 32 bit float
				if (ibuf[i] > SOX_SAMPLE_MAX-128) *sample = 1.f;
				else *sample = ((ibuf[i]+128)&~255)*(1./(SOX_SAMPLE_MAX+1.));
			}
			else *sample = 0;
			sample++;
		}
	}
	else {
		sox_int16_t *sample = (sox_int16_t*)ssi_sox_exchange;
		for (int i = 0; i < ssi_sox_exchange_bytes/2; i++) {
			if (i < *isamp) {
				if (ibuf[i]>SOX_SAMPLE_MAX-(1<<(31-16))) *sample = SOX_INT_MAX(16);
				else *sample = ((sox_uint32_t)(ibuf[i]+(1<<(31-16))))>>(32-16);		//internally used signed 32 bit -> 16 bit integer
			}
			else *sample = 0;
			sample++;
		}
	}

	if (!firstRun)  {
		_drain_out_event.release ();													//on first run nothing has been processed
		soxReadCount = 0;
		soxWriteCount++;
	}																					
	else firstRun = false;

	if (soxWriteCount > 1) ssi_err("Sox tried to write more than one time -> this chain with current options isn't usable for ssi.");

	return SOX_SUCCESS; /* All samples output successfully */
}


/* A `stub' effect handler to handle outputting samples from the effects
 * chain; the only function needed for this example is `flow' */
sox_effect_handler_t const * LibsoxFilter::output_handler(void)
{
	static sox_effect_handler_t handler = {
	"output", NULL, SOX_EFF_MCHAN, NULL, NULL, output_flow, NULL, NULL, NULL, 0
	};
	return &handler;
}



LibsoxFilter::DrainThread::DrainThread (LibsoxFilter *filter) 
	: Thread (true),
	effectArgStr(""),
	soxGlobalBuf(2048),
	soxSignalLength(2048),
	sampleRate(48000),
	is32BitFloat(false),
	channels(1),
	filter (filter)
{
	Thread::setName ("libsox");
}

void LibsoxFilter::DrainThread::setParams (const char* effectArgStr, int soxGlobalBuf, int soxSignalLength, int sampleRate, bool is32BitFloat, int channels) 
{
	this->effectArgStr = effectArgStr;
	this->soxGlobalBuf = soxGlobalBuf;
	this->soxSignalLength = soxSignalLength;
	this->sampleRate = sampleRate;
	this->is32BitFloat = is32BitFloat;
	this->channels = channels;
}

LibsoxFilter::DrainThread::~DrainThread () {
}

void LibsoxFilter::DrainThread::enter () {
	// init

	sox_init();			

	sox_globals.bufsiz = soxGlobalBuf;
	sox_globals.use_threads = sox_bool::sox_true;


	//set properties of the signal

	sox_signalinfo_t signal;
	sox_encodinginfo_t encoding;
		
	signal.channels = channels;
	signal.length = soxSignalLength;
	signal.mult = 0;

	if (is32BitFloat) signal.precision = 32;
	else signal.precision = 16;

	signal.rate = sampleRate;

	if (is32BitFloat) encoding.bits_per_sample = 32;
	else encoding.bits_per_sample = 16;

	encoding.compression = 0;
	encoding.opposite_endian = sox_false;
	encoding.reverse_bits =  sox_option_t::sox_option_no;
	encoding.reverse_bytes = sox_option_t::sox_option_no;
	encoding.reverse_nibbles = sox_option_t::sox_option_no;

	if (is32BitFloat) encoding.encoding = sox_encoding_t::SOX_ENCODING_FLOAT;
	else encoding.encoding = sox_encoding_t::SOX_ENCODING_SIGN2;


	bool noError = true;

	//effect chain
	sox_effect_t * e;

	//create the effect chain
	chain = sox_create_effects_chain(&encoding, &encoding);
		
	//effect chain input (handler overwrite)
	e = sox_create_effect(input_handler());
	noError = sox_add_effect(chain, e, &signal, &signal) == SOX_SUCCESS;
	SSI_ASSERT(noError);
	free(e);

	//create the effects with their options by using the commandline like string
	int argc;
	//convert string to argv
	char** argv = LibsoxFilterTools::buildargv(effectArgStr, &argc);
		
	if (argc > 0 && strlen(argv[0]) > 0) {
		noError = sox_find_effect(argv[0]);
		//SSI_ASSERT(noError);
		if (!noError) {
			ssi_err ("SoX: First argument is no effect name!");
			return;
		}

		std::vector<char*> argvEffect;
		char *effectName = argv[0];		//first parameter has to be an effect!
		int effectCount = 1;

		for (int i = 1; i < argc+1; i++) {
			bool isEffect = false;

			if (i != argc) {
				isEffect = sox_find_effect(argv[i]);
				if (!isEffect) argvEffect.push_back(argv[i]);
			}

			if (i == argc || isEffect) {
				//create effect
				e = sox_create_effect(sox_find_effect(effectName));
				if (argvEffect.size() == 0) noError = sox_effect_options(e, 0, NULL) == SOX_SUCCESS;
				else noError = sox_effect_options(e, argvEffect.size(), &argvEffect[0]) == SOX_SUCCESS;
				//SSI_ASSERT(noError);

				if (!noError) { 
					argvEffect.clear();
					free(e);

					ssi_err ("SoX: Effect option error with \"%s\"!", effectName);
					return;
				}
				noError = sox_add_effect(chain, e, &signal, &signal) == SOX_SUCCESS;
				SSI_ASSERT(noError);
				free(e);

				if (!noError) {
					argvEffect.clear();	

					ssi_err ("SoX: Can't add effect \"%s\" to the chain!", effectName);
					return;
				}

				ssi_msg (SSI_LOG_LEVEL_DETAIL, "SoX: Added effect \"%s\" with options:",effectName);
				for (int j = 0; j < argvEffect.size(); j++) ssi_msg (SSI_LOG_LEVEL_DETAIL, "%s",argvEffect.at(j));

				argvEffect.clear();

				if (i == argc) break;

				effectName = argv[i];
				effectCount++;
			}
		}
	}

		
	//effect chain output (handler overwrite)
	e = sox_create_effect(output_handler());
	noError = sox_add_effect(chain, e, &signal, &signal) == SOX_SUCCESS;
	SSI_ASSERT(noError);
	free(e);
}

void LibsoxFilter::DrainThread::run () {
	//start the signal flow in the effect chain
	sox_flow_effects(chain, NULL, NULL);
	ssi_msg (SSI_LOG_LEVEL_BASIC, "SoX effect chain flow finished");
	_drain_out_event.release ();
}

void LibsoxFilter::DrainThread::flush () {
	//clean up
	sox_delete_effects_chain(chain);
	sox_quit();
}
