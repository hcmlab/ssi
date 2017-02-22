// EventPainter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/08
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

#ifndef SSI_GRAPHIC_EVENTPAINTER_H
#define SSI_GRAPHIC_EVENTPAINTER_H

#include "PaintBars.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class EventPainter : public IObject {

public:

	class Options : public OptionList {

	public:

		Options ()
			: type(PaintBars::TYPE::BAR), reset(false), global(false), autoscale(true), axisPrecision(2), fix(1.0f) {

			name[0] = '\0';		
			barNames[0] = '\0';
			setPos (0,0,100,100);	

			addOption ("title", name, SSI_MAX_CHAR, SSI_CHAR, "plot caption");
			addOption ("barNames", barNames, SSI_MAX_CHAR, SSI_CHAR, "bar names separated by ',' (if not available tuple ids are used)");
			addOption ("type", &type, 1, SSI_UCHAR, "plot type (0=bar, 1=positive bar)");								
			addOption ("pos", pos, 4, SSI_INT, "window position (top, left, width, height)");  		
			addOption ("autoscale", &autoscale, 1, SSI_BOOL, "automatically scale to maximum (also see global, fix and reset)");			
			addOption ("global", &global, 1, SSI_BOOL, "apply a global maximum (otherwise limits are calculated for each event dimension)");
			addOption ("reset", &reset, 1, SSI_BOOL, "reset y axis to the current global maximum (only applied if global=true)");			
			addOption ("fix", &fix, 1, SSI_REAL, "set a fixed maximum (only applied if global=true, reset=false and autoscale=false)");			
			addOption ("axisPrecision", &axisPrecision, 1, SSI_SIZE, "precision for axis values");							
		};

		void setTitle (const ssi_char_t *name) {
			ssi_strcpy (this->name, name);
		}
		void setBarName(const ssi_char_t *barNames) {
			ssi_strcpy(this->barNames, barNames);
		}
		void setPos(int left, int top, int width, int height) {
			pos[0] = left; pos[1] = top; pos[2] = width; pos[3] = height;
		}

		ssi_char_t name[SSI_MAX_CHAR]; 
		ssi_char_t barNames[SSI_MAX_CHAR];
		PaintBars::TYPE::List type;
		int pos[4];
		bool reset;
		bool autoscale;
		bool global;
		ssi_real_t fix;
		ssi_size_t axisPrecision;
	};

public: 

	static const ssi_char_t *GetCreateName () { return "EventPainter"; };
	static IObject *Create (const ssi_char_t *file) { return new EventPainter (file); };
	~EventPainter ();
	
	EventPainter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Viewer for events."; };
	
	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	void listen_enter();
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

protected:

	EventPainter (const ssi_char_t *file = 0);
	EventPainter::Options _options;
	ssi_char_t *_file;

	IWindow *_window;
	ICanvas *_canvas;
	PaintBars *_client;

	static const ssi_char_t *ssi_log_name;
	static int ssi_log_level;
};

}

#endif
