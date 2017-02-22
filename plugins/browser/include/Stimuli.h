// Stimuli.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 17/11/2014
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_BROWSER_STIMULI_H
#define SSI_BROWSER_STIMULI_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "ioput/File/StringList.h"
#include "event/EventAddress.h"

namespace ssi {

class FileAnnotationWriter;

class Stimuli : public IObject {

public:

	class Options : public OptionList {

	public:

		Options()
			: randomize(false), labelFromFile(true), applyLabelToEvent(false), insertBlank(false) {

			startName[0] = '\0';
			endName[0] = '\0';						

			labels[0] = '\0';
			names[0] = '\0';

			folder[0] = '\0';
			setExtension("*");			

			annopath[0] = '\0';

			setSender("stimuli");
			setEvent("url");		

			SSI_OPTIONLIST_ADD_ADDRESS(address);

			addOption("names", names, SSI_MAX_CHAR, SSI_CHAR, "string with names, e.g. urls, paths, etc. (separated by ;)");
			addOption("labels", labels, SSI_MAX_CHAR, SSI_CHAR, "if not empty, assigns a label to each url (separated by ;)");

			addOption("folder", folder, SSI_MAX_CHAR, SSI_CHAR, "name of folder with files, overrides 'names' (may contain a list of folders separated by ;)");
			addOption("labelFromFile", &labelFromFile, 1, SSI_BOOL, "parse labels from stimuli names by removing all none literals");
			addOption("extension", extension, SSI_MAX_CHAR, SSI_CHAR, "file extension, e.g. *.html");			

			addOption("startName", startName, SSI_MAX_CHAR, SSI_CHAR, "if set, this url will be displayed as first page");
			addOption("endName", endName, SSI_MAX_CHAR, SSI_CHAR, "if set, this url will be displayed as last page");			
			addOption("randomize", &randomize, 1, SSI_BOOL, "randomize urls/files");			
			addOption("insertBlank", &insertBlank, 1, SSI_BOOL, "after each name an empty string is sent");

			addOption("annoPath", annopath, SSI_MAX_CHAR, SSI_CHAR, "if set to a valid path, an annotation file is created");
			addOption("applyLabelToEvent", &applyLabelToEvent, 1, SSI_BOOL, "if triggering event is a string event, replace with current label");

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender [depcerated use address]");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event [depcerated use address]");			
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

		void setFolder(const ssi_char_t *string) {
			this->folder[0] = '\0';
			if (string) {
				ssi_strcpy(this->folder, string);
			}
		}

		void setNames(const ssi_char_t *string) {
			this->names[0] = '\0';
			if (string) {
				ssi_strcpy(this->names, string);
			}
		}
		void setLabels(const ssi_char_t *string) {
			this->labels[0] = '\0';
			if (string) {
				ssi_strcpy(this->labels, string);
			}
		}

		void setStartName(const ssi_char_t *string) {
			this->startName[0] = '\0';
			if (string) {
				ssi_strcpy(this->startName, string);
			}
		}

		void setEndName(const ssi_char_t *string) {
			this->endName[0] = '\0';
			if (string) {
				ssi_strcpy(this->endName, string);
			}
		}

		void setAnnoPath(const ssi_char_t *string) {
			this->annopath[0] = '\0';
			if (string) {
				ssi_strcpy(this->annopath, string);
			}
		}

		void setExtension(const ssi_char_t *string) {
			this->extension[0] = '\0';
			if (string) {
				ssi_strcpy(this->extension, string);
			}
		}

		bool randomize;

		ssi_char_t labels[SSI_MAX_CHAR];
		ssi_char_t names[SSI_MAX_CHAR];

		ssi_char_t folder[SSI_MAX_CHAR];
		bool labelFromFile;
		ssi_char_t extension[SSI_MAX_CHAR];
		
		ssi_char_t startName[SSI_MAX_CHAR];
		ssi_char_t endName[SSI_MAX_CHAR];
		bool insertBlank;
		
		ssi_char_t annopath[SSI_MAX_CHAR];
		bool applyLabelToEvent;

		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName() { return "Stimuli"; };
	static IObject *Create(const ssi_char_t *file) { return new Stimuli(file); };
	virtual ~Stimuli();

	Options *getOptions() { return &_options; };
	const ssi_char_t *getName() { return GetCreateName(); };
	const ssi_char_t *getInfo() { return "Sends stimuli names (e.g. files from a folder or a list of urls)."; };

	void listen_enter();
	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush();

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress() {
		return _eaname.getAddress();
	}

protected:

	Stimuli(const ssi_char_t *file = 0);
	ssi_char_t *_file;
	Options _options;
	static char ssi_log_name[];

	bool loadFromFolder();
	bool loadFromNames();
	void loadHelp(ssi_size_t n, ssi_char_t **names, ssi_char_t **labels, bool randomize, bool insertBlank);
	const ssi_char_t *nextName();
	void sendName(const ssi_char_t *name, ssi_size_t time);
	ssi_char_t *labelFromFile(const ssi_char_t *file);	

	ssi_size_t _n_names;
	StringList _names;
	StringList _labels;	
	ssi_size_t _next_name;
	bool _finished;

	IEventListener *_elistener;
	ssi_event_t _ename;
	EventAddress _eaname;

	FileAnnotationWriter *_anno_writer;
};

}

#endif
