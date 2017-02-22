// VectorFusionModality.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/11/13
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

#ifndef SSI_VECTORFUSIONMODALITY_H
#define SSI_VECTORFUSIONMODALITY_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/ITheEventBoard.h"
#include "EVector.h"
#if _WIN32||_WIN64
#include "FusionPainter.h"
#else
#define FusionPainter ssi::ICanvasClient
#endif
#include "thread/Thread.h"
#include "thread/Lock.h"
#include <deque>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "VectorFusionDefines.h"

namespace ssi {

class Window;
class Canvas;

class VectorFusionModality : public IObject {

public:

	class Options : public OptionList {

	public:

		Options (): dimension(0), update_ms(1000), decay_type (EVector::DECAY_TYPE_HYP), fusionspeed(0.1f), eventspeed (0.1f), gradient(0.5f), threshold(0.1f), accelerate (false), negative (true), print(false), paint(false), paint_events(true) {

			path[0] = '\0';

			setAddress("");
			setSenderName ("fsender");
			setEventName ("fevent");
			setAxisCaption("caption");
			setTitle("");
			setPosition(0, 0, 100, 100);

			SSI_OPTIONLIST_ADD_ADDRESS(address);

			addOption("dimension", &dimension, 1, SSI_INT, "dimension of vector space");
			addOption("update_ms", &update_ms, 1, SSI_INT, "time interval the updated fusion vector is sent via event");
			addOption("decay_type", &decay_type, 1, SSI_INT, "decay type of event vectors");
			addOption("fusionspeed", &fusionspeed, 1, SSI_FLOAT, "speed of fusion vector towards mass center");
			addOption("eventspeed", &eventspeed, 1, SSI_FLOAT, "defaulte speed of event vectors towards zero value");
			addOption("gradient", &gradient, 1, SSI_FLOAT, "gradient for exponential or hyperbolic temporal decay of event vectors");
			addOption("threshold", &threshold, 1, SSI_FLOAT, "threshold for vector norms to be included in fusion");
			addOption("accelerate", &accelerate, 1, SSI_BOOL, "accelerate fusion vector if >1 events occur");
			addOption("negative", &negative, 1, SSI_BOOL, "allow negative values");
			addOption("print", &print, 1, SSI_BOOL, "textual output");
			addOption("paint", &paint, 1, SSI_BOOL, "graphical output");
			addOption("paint_events", &paint_events, 1, SSI_BOOL, "graphical output of event vectors");
			addOption("pos", move, 4, SSI_INT, "window position (left, top, width, height)");
			addOption("title", wcaption, SSI_MAX_CHAR, SSI_CHAR, "window caption");
			addOption("caption", caption, SSI_MAX_CHAR, SSI_CHAR, "axis captions (seperated by comma)");
			addOption("path", path, SSI_MAX_CHAR, SSI_CHAR, "path to modality file");

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender [deprecated use address]");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event [deprecated use address]");
		};

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}
		void setPath (const ssi_char_t *path) {
			ssi_strcpy (this->path, path);
		}
		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEventName (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}
		void setAxisCaption (const ssi_char_t *caption) {
			if (caption) {
				ssi_strcpy (this->caption, caption);
			}
		}
		void setTitle(const ssi_char_t *caption) {
			if (caption) {
				ssi_strcpy(this->wcaption, caption);
			}
		}
		void setPosition(int left, int top, int width, int height) {
			move[0] = left; move[1] = top; move[2] = width; move[3] = height;
		}

		ssi_size_t dimension;
		ssi_size_t update_ms;
		EVector::DECAY_TYPE decay_type;
		ssi_real_t fusionspeed;
		ssi_real_t eventspeed;
		ssi_real_t gradient;
		ssi_real_t threshold;
		bool print;
		bool accelerate;
		int move[4];
		bool paint;
		bool paint_events;
		bool negative;
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t path[SSI_MAX_CHAR];		
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
		ssi_char_t caption[SSI_MAX_CHAR];
		ssi_char_t wcaption[SSI_MAX_CHAR];
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "VectorFusionModality"; };
	static IObject *Create (const ssi_char_t *file) { return new VectorFusionModality (file); };
	~VectorFusionModality ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Collects events and applies vector based fusion. sends fusion vector periodically to the board."; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	std::deque<EVector*> VectorList;
	
	bool transformEventToVector(ssi_event_t *Event);
	bool combineVectors(ssi_real_t fusion_speed, ssi_real_t delta_t, ssi_real_t threshold);

	ssi_size_t getDim() { return _options.dimension; };
	EVector* getFusionVector();
	ssi_real_t getThreshold();

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void print ();
	void initAxisCaption(ssi_size_t dim);
	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	ICanvasClient* getPlot(){ return _plot; };

	ssi_size_t getUpdateRate() { 
		return _options.update_ms;
	};

protected:

	VectorFusionModality (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	EventAddress _event_address;

	ssi_real_t *_fusion_point;
	EVector *_fusion_vector;

	ssi_size_t _dim;
	ssi_size_t _update_ms;
	ssi_size_t _update_counter;
	EVector::DECAY_TYPE _decay_type;
	ssi_real_t _fusion_speed;
	ssi_real_t _event_speed;
	ssi_real_t _gradient;
	ssi_real_t _threshold;
	ssi_real_t *_baseline;
	ssi_real_t _baseline_norm;

	ssi_size_t _framework_time;
	ssi_size_t _last_call;

	Window *_window;
	Canvas *_canvas;
	FusionPainter *_plot;

	ITheEventBoard *_board;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	bool _print;
	bool _paint;
	
	ssi_char_t **_axis_captions;

	IEventListener *_listener;
	ssi_event_t _event;

	Mutex _mutex;
	ModalitySpeedWeightMap *_modality_map;

	ssi_size_t _baseline_id;

};

}

#endif
