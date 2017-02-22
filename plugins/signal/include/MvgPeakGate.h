// MvgPeakGate.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/11
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#ifndef SSI_SIGNAL_MVGPEAKGATE_H
#define SSI_SIGNAL_MVGPEAKGATE_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "base/IMonitor.h"
#include "base/IWindow.h"

namespace ssi {

class MvgPeakGate : public IFilter {

public:

enum METHOD {
	MOVING = 0,
	SLIDING		
};

enum THRESHOLD {
	FIX = 0,
	FIXAVG,
	FIXAVGSTD
};

class Options : public OptionList {

	public:

		Options () 
			: value (0.0f), keep (false), thres (FIX), fix (0.0f), mstd (1.0f), full (false), win (10.0), method (MOVING), monitor (false) {

			mpos[0] = 0;
			mpos[1] = 0;
			mpos[2] = 280;
			mpos[3] = 80;

			addOption ("value", &value, 1, SSI_REAL, "blocked samples are set to this value");	
			addOption ("keep", &keep, 1, SSI_BOOL, "for unblocked samples keep original values");
			addOption ("thres", &thres, 1, SSI_INT, "threshold (0=fix,1=fix+avg,2=fix+avg+mstd*std)");	
			addOption ("fix", &fix, 1, SSI_REAL, "value of fixed threshold");	
			addOption ("mstd", &mstd, 1, SSI_REAL, "multiplier for standard deviation adjusted threshold");							
			addOption ("full", &full, 1, SSI_BOOL, "use all values to update threshold, otherwise only blocked samples are used");							
			addOption ("win", &win, 1, SSI_TIME, "size of moving/sliding window in seconds");	
			addOption ("method", &method, 1, SSI_INT, "method (0=moving,1=sliding)");		
			addOption ("monitor", &monitor, 1, SSI_BOOL, "display threshold monitor");
			addOption ("mpos", &mpos, 4, SSI_INT, "position of monitor on screen [posx,posy,width,height]");
		};

		void setMonitorPos (int x, int y, int width, int height) {
			monitor = true;
			mpos[0] = x;
			mpos[1] = y;
			mpos[2] = width;
			mpos[3] = height;
		}

		ssi_real_t value;
		bool keep;
		ssi_real_t fix;
		ssi_real_t mstd;
		bool full;
		ssi_time_t win;
		METHOD method;
		THRESHOLD thres;		
		bool monitor;
		int mpos[4];
	};


public:

	static const ssi_char_t *GetCreateName () { return "MvgPeakGate"; };
	static IObject *Create (const ssi_char_t *file) { return new MvgPeakGate (file); };
	~MvgPeakGate ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Blocks out everything but peaks in the input stream."; };

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
		return sizeof (ssi_real_t);
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

protected:

	MvgPeakGate (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;

	ITransformer **_mvg;	
	ssi_stream_t *_mvg_strm;
	ssi_stream_t *_mvg_upd_strm;
	ssi_real_t *_threshold;	
	ssi_real_t *_average;
	ssi_real_t *_stdeviation;
	THRESHOLD _thres;
	ssi_real_t _value;
	bool _keep;
	bool _full;
	int _counter;
	int _counter_tot;
	ssi_real_t _fix;
	ssi_real_t _mstd;	

	ssi_char_t _string[SSI_MAX_CHAR];
	IWindow *_window;
	IMonitor *_monitor;
};

}

#endif
