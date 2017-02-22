// MyWinsensors.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 25/3/2015
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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_WINSENSORS_MYWINSENSORS_H
#define SSI_WINSENSORS_MYWINSENSORS_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "thread/Lock.h"
#include "base/Factory.h"

#include <sstream>

#define SENSOR_INDEX_ACC 0
#define SENSOR_INDEX_GYR 1
#define SENSOR_INDEX_INC 2
#define SENSOR_INDEX_LIG 3
#define SENSOR_INDEX_COM 4


namespace ssi {

	class Winsensors : public IObject {

	public:

		class Options : public OptionList {

		public:

			Options() : pollMs(100) {
				setSender("Winsensors");
				setAllSensorsOn();
				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
				addOption("sensorpoll", &pollMs, 1, SSI_INT, "default sensor polling time in ms");

				addOption("activesensors", activeSensors, 5, SSI_BOOL, "activate / disable sensors [acc, gyr, incl, light, comp]");
			}

			void setSender(const ssi_char_t *sname) {
				if (sname) {
					ssi_strcpy(this->sname, sname);
				}
			}

			void setSensors(bool *list) {
				if (list) {
					memcpy(activeSensors, list, sizeof(bool) * 5);
				}
			}

			void setAllSensorsOn() {
				for (int i = 0; i < 5; i++)
					activeSensors[i] = true;
			}

			void setAllSensorsOff() {
				for (int i = 0; i < 5; i++)
					activeSensors[i] = false;
			}

			ssi_char_t sname[SSI_MAX_CHAR];

			bool activeSensors[5];

			ssi_size_t pollMs;

		};

	public:

		static const ssi_char_t *GetCreateName() { return "Winsensors"; };
		static IObject *Create(const ssi_char_t *file) { return new Winsensors(file); };
		~Winsensors();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "windows sensors"; };


		//event sender
		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}


		//event sender
		IEventListener	*_elistener;
		EventAddress	_event_address;
		ssi_event_t		_event_acc, _event_gyr, _event_light, _event_incl, _event_comp;
		ITheFramework	*_frame;


		void listenerUpdate(ssi_event_t _event);

	protected:

		Winsensors(const ssi_char_t *file = 0);
		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		Mutex listenerMutex;


	};


}

#endif
