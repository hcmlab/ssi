/*
 * Adapted version: (c)2015 Andreas Seiderer
 *
 * Original header:
 *
 * http://github.com/todbot/usbSearch/
 *
 * 2012, Tod E. Kurt, http://todbot.com/blog/
 *
 *
 * Uses DispHealper : http://disphelper.sourceforge.net/
 *
 * Notable VIDs & PIDs combos:
 * VID 0403 - FTDI
 * 
 * VID 0403 / PID 6001 - Arduino Diecimila
 *
 */

#pragma once

#ifndef SSI_LISTCOMPORTS_H
#define	SSI_LISTCOMPORTS_H

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdint.h>
	#include <stdarg.h>
	#include <string.h>


	#include <windows.h>
	#include <setupapi.h>
	#include "disphelper.h"

	#include <vector>

	class USBserialDevice {
		public:
			USBserialDevice(std::string comName, std::string manufacturer, std::string pnpId) {
				this->comName = comName;
				this->manufacturer = manufacturer;
				this->pnpId = pnpId;
			}

			std::string comName;
			std::string manufacturer;
			std::string pnpId;
	};

	int listComPorts(std::vector<USBserialDevice*> *devices);


#endif