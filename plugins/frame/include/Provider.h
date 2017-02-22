// Provider.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/16
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
//Changes:
//added Interface ITransformable


#ifndef SSI_FRAME_PROVIDER_H
#define SSI_FRAME_PROVIDER_H

#include "FrameLibCons.h"
#include "base/IProvider.h"
#include "base/IFilter.h"
#include "base/ITransformable.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class TheFramework;

class Provider : public Thread, public IProvider, public ITransformable {

	friend class TheFramework;

	public:

		const void *getMetaData (ssi_size_t &size) { size = _meta_size; return _meta; };	
		void setMetaData (ssi_size_t size, const void *meta);

		int getBufferId () {
			SSI_ASSERT (_buffer_id != THEFRAMEWORK_ERROR);
			return _buffer_id;
		};

		ssi_time_t getSampleRate () {
			SSI_ASSERT (_buffer_id != THEFRAMEWORK_ERROR);
			return _sample_rate;
		}

		ssi_size_t getSampleDimension () {
			SSI_ASSERT (_buffer_id != THEFRAMEWORK_ERROR);
			return _sample_dimension;
		}

		ssi_size_t getSampleBytes () {
			SSI_ASSERT (_buffer_id != THEFRAMEWORK_ERROR);
			return _sample_bytes;
		}

		ssi_type_t getSampleType () {
			SSI_ASSERT (_buffer_id != THEFRAMEWORK_ERROR);
			return _sample_type;
		}

		static void SetLogLevel (int level) {
			ssi_log_level = level;
		}
		
	protected:

		Provider (IFilter *filter = 0,
			const char *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
			const char *watch_interval = THEFRAMEWORK_DEFAULT_WATCH_DUR,
			const char *sync_interval = THEFRAMEWORK_DEFAULT_SYNC_DUR);
		~Provider ();

		IOptions *getOptions () { return 0; };
		const ssi_char_t *getName () { return "Provider"; };
		const ssi_char_t *getInfo () { return "Provides streams to the framework."; };

		static char *ssi_log_name;
		static int ssi_log_level;	

		void init (IChannel *channel);
		ssi_char_t *_channel_name;

		bool provide (ssi_byte_t *data, 
			ssi_size_t sample_number);

		void enter ();
		void run ();
		void flush ();

		ssi_size_t _meta_size;
		ssi_byte_t *_meta;

		int _buffer_id;
		ssi_time_t _sample_rate;
		ssi_size_t _sample_dimension;
		ssi_size_t _sample_bytes;
		ssi_type_t _sample_type;
		ssi_char_t *_buffer_size;

		double _time;
		unsigned int _run_sleep; // sleep time in milliseconds between two calls to run ()

		bool _do_watch;
		bool _watch; // stores if new data has been provided since last check
		Mutex _watch_mutex; // allows thread safe access to check flag
		unsigned int _watch_iter; // total number of run iteration until next check
		unsigned int _watch_iter_counter; // number of run iterations left until next check
		bool _is_providing_zeros;
		ssi_char_t *_watch_interval;

		bool _do_sync;
		unsigned int _sync_iter;	// total number of run iteration until next sync
		unsigned int _sync_iter_counter; // number of run iterations left until next sync
		ssi_char_t *_sync_interval;

		IFilter *_filter;
		ssi_stream_t _stream;
		ssi_stream_t _filter_stream;

		TheFramework *_frame;
};

}

#endif
