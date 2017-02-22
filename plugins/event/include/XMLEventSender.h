// XMLEventSender.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/10/27
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

#ifndef SSI_IOPUT_XMLEVENTWRITER_H
#define SSI_IOPUT_XMLEVENTWRITER_H

#include "base/IConsumer.h"
#include "ioput/file/FileStreamOut.h"
#include "ioput/option/OptionList.h"
#include "base/ITheFramework.h"

namespace ssi {

class TiXmlDocument;
class XMLEventHelper;
class Monitor;
class Window;
class EventAddress;

class XMLEventSender : public IConsumer {

	friend class XMLEventHelper;

public:

	class Options : public OptionList {

	public:

		Options()
			: strbuf(SSI_MAX_CHAR),
			rowdelim(';'),
			coldelim(','),
			console (false),
			monitor (false),
			relative (false),						
			mbuf(10000),
			update(100) {

			path[0] = '\0';

			screen[0] = 0;
			screen[1] = 0;

			mpos[0] = 0;
			mpos[1] = 0;
			mpos[2] = 100;
			mpos[3] = 100;

			setMonitorName("XMLMonitor");
			setAddress("");
			setSender("sender");
			setEvent("xml");

			SSI_OPTIONLIST_ADD_ADDRESS(address);

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");
			addOption("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path (empty for stdout)");
			addOption("update", &update, 1, SSI_SIZE, "update interval in ms (if set to 0 sender will update with every change)");			
			addOption("console", &console, 1, SSI_BOOL, "output on console");
			addOption("monitor", &monitor, 1, SSI_BOOL, "output on monitor");
			addOption("relative", &relative, 1, SSI_BOOL, "arrange windows relative to screen");
			addOption("screen", &screen, 2, SSI_REAL, "customize screen region [width,height], by default set to desktop size");
			addOption("mpos", &mpos, 4, SSI_REAL, "position of monitor on screen [posx,posy,width,height], either in pixels or relative to screen");
			addOption("mname", mname, SSI_MAX_CHAR, SSI_CHAR, "name of monitor (will be displayed in title)");
			addOption("chars", &mbuf, 1, SSI_SIZE, "maximum number of chars displayed on monitor");
			addOption("rowdelim", &rowdelim, 1, SSI_CHAR, "delimiter to separate values in different rows");
			addOption("coldelim", &coldelim, 1, SSI_CHAR, "delimiter to separate values in the same column");			
			addOption("strbuf", &strbuf, 1, SSI_SIZE, "temporary string buffer, should be large enough to hold mapped values");
		};

		void setPath (const ssi_char_t *path) {
			this->path[0] = '\0';
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}

		void setMonitorPos(ssi_real_t x, ssi_real_t y, ssi_real_t width, ssi_real_t height) {
			mpos[0] = x;
			mpos[1] = y;
			mpos[2] = width;
			mpos[3] = height;
		}

		void setScreen(ssi_real_t width, ssi_real_t height) {
			screen[0] = width;
			screen[1] = height;
		}

		void setMonitorName(const ssi_char_t *name) {
			ssi_strcpy(mname, name);
		}

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}
		void setSender(const ssi_char_t *sname) {
			if (sname) {
				ssi_strcpy(this->sname, sname);
			}
		}
		void setEvent(const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy(this->ename, ename);
			}
		}
		
		ssi_char_t path[SSI_MAX_CHAR];
		bool console;
		bool monitor;
		ssi_real_t mpos[4];
		ssi_real_t screen[2];
		ssi_char_t mname[SSI_MAX_CHAR];
		bool relative;
		ssi_size_t mbuf, strbuf;
		ssi_char_t rowdelim, coldelim;
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
		ssi_size_t update;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "XMLEventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new XMLEventSender (file); };
	~XMLEventSender ();

	XMLEventSender::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Stores streams to a file on disk."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void listen_enter();
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress();

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	XMLEventSender (const ssi_char_t *file = 0);
	XMLEventSender::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ITheFramework *_frame;

	bool load();
	void close();
	bool _loaded;
	ssi_char_t *_path;
	TiXmlDocument *_doc;
	XMLEventHelper *_helper;	

	Monitor *_monitor;
	Window *_window;

	ssi_event_t _event;
	IEventListener *_listener;
	EventAddress *_address;
};

}

#endif
