// EHealth.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/06/05
// Copyright (C) University of Augsburg
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

#include "EHealth.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t EHealth::ssi_log_name[] = "ehealth___";

EHealth::EHealth (const ssi_char_t *file) 
: ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {			

	for (ssi_size_t i = 0; i < SSI_EHEALTH_N_CHANNELS; i++) {
		_channel[i] = 0;
		_provider[i] = 0;
	}

	_com_port[0] = '\0';
}

EHealth::~EHealth () {

	for (ssi_size_t i = 0; i < SSI_EHEALTH_N_CHANNELS; i++) {
		delete _channel[i];		
	}
}

IChannel *EHealth::getChannel (ssi_size_t index) { 

	if (index >= SSI_EHEALTH_N_CHANNELS) {
		ssi_wrn ("requested index '%u' exceeds maximum number of channels '%u'", index, SSI_EHEALTH_N_CHANNELS);
		return 0;
	}

	if (!_channel[index]) {
	
		switch (index) {
			case SSI_EHEALTH_ECG_INDEX:					
				_channel[SSI_EHEALTH_ECG_INDEX] = new EcgChannel (_options.sr);			
				break;
			case SSI_EHEALTH_GSR_INDEX:
				_channel[SSI_EHEALTH_GSR_INDEX] = new GsrChannel (_options.sr);			
				break;
			case SSI_EHEALTH_AIR_INDEX:
				_channel[SSI_EHEALTH_AIR_INDEX] = new AirChannel (_options.sr);			
				break;
			case SSI_EHEALTH_TMP_INDEX:
				_channel[SSI_EHEALTH_TMP_INDEX] = new TmpChannel (_options.sr);			
				break;
			case SSI_EHEALTH_BPM_INDEX:
				_channel[SSI_EHEALTH_BPM_INDEX] = new BpmChannel (_options.sr);			
				break;
			case SSI_EHEALTH_OXY_INDEX:
				_channel[SSI_EHEALTH_OXY_INDEX] = new OxyChannel (_options.sr);			
				break;
		}
	}
	
	return _channel[index];
};

bool EHealth::setProvider (const ssi_char_t *name, IProvider *provider) {

	ssi_size_t sr = _options.sr;	
	ssi_size_t index = 0;

	if (strcmp (name, SSI_EHEALTH_ECG_PROVIDER_NAME) == 0) {
		index = SSI_EHEALTH_ECG_INDEX;
	} else if (strcmp (name, SSI_EHEALTH_GSR_PROVIDER_NAME) == 0) {	
		index = SSI_EHEALTH_GSR_INDEX;	
	} else if (strcmp (name, SSI_EHEALTH_AIR_PROVIDER_NAME) == 0) {	
		index = SSI_EHEALTH_AIR_INDEX;	
	} else if (strcmp (name, SSI_EHEALTH_TMP_PROVIDER_NAME) == 0) {	
		index = SSI_EHEALTH_TMP_INDEX;	
	} else if (strcmp (name, SSI_EHEALTH_BPM_PROVIDER_NAME) == 0) {	
		index = SSI_EHEALTH_BPM_INDEX;	
	} else if (strcmp (name, SSI_EHEALTH_OXY_PROVIDER_NAME) == 0) {	
		index = SSI_EHEALTH_OXY_INDEX;	
	} else {
		ssi_wrn ("channel with name '%s' does not exist", name);
		return false;
	}

	if (_provider[index]) {
		ssi_wrn ("provider with name '%s' was already set", name);
		return false;
	}
	
	IChannel *channel = getChannel (index);
	_provider[index] = provider;
	_provider[index]->init (channel);
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "provider '%s' set", channel->getName ());

	return true;
}

void EHealth::sendCommand (const char *cmd) {
	if (_serial) {
		ssi_size_t len = ssi_strlen (cmd) + 3;
		char *buffer = new char[len];
		sprintf (buffer, "<%s>", cmd);
		_serial->WriteData (buffer, len-1);
		delete[] buffer;
	}
}

bool EHealth::connect () {
		
	ssi_sprint (_com_port, "COM%u", _options.port);
	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to connect sensor (%s)...", _com_port);

	// create serial port
	_serial = new Serial (_com_port);
	if (!_serial->IsConnected ()) {
		ssi_wrn ("could not connect sensor (%s)...", _com_port);
		return false;
	}

	// request channels
	for (ssi_size_t i = 0; i < SSI_EHEALTH_N_CHANNELS; i++) {
		if (_provider[i]) {
			switch (i) {
				case SSI_EHEALTH_ECG_INDEX:					
					sendCommand (SSI_EHEALTH_ECG_PROVIDER_NAME);
					break;
				case SSI_EHEALTH_GSR_INDEX:
					sendCommand (SSI_EHEALTH_GSR_PROVIDER_NAME);			
					break;
				case SSI_EHEALTH_AIR_INDEX:
					sendCommand (SSI_EHEALTH_AIR_PROVIDER_NAME);		
					break;
				case SSI_EHEALTH_TMP_INDEX:
					sendCommand (SSI_EHEALTH_TMP_PROVIDER_NAME);	
					break;
				case SSI_EHEALTH_BPM_INDEX:
					sendCommand (SSI_EHEALTH_BPM_PROVIDER_NAME);
					break;
				case SSI_EHEALTH_OXY_INDEX:
					sendCommand (SSI_EHEALTH_OXY_PROVIDER_NAME);	
					break;
			}
		}
	}

	// start device
	sendCommand (SSI_EHEALTH_CMD_START);

	// set thread name	
	Thread::setName (getName ());
	this->run();

	ssi_msg (SSI_LOG_LEVEL_BASIC, "sensor connected");	

	return true;
}

bool EHealth::disconnect () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to disconnect sensor (%s)...", _com_port);

	if (!_serial || !_serial->IsConnected ()) {
		ssi_wrn ("sensor (%s) not connected", _com_port);
		return false;
	}

	sendCommand (SSI_EHEALTH_CMD_STOP);

	// clean up
	delete _serial; _serial = 0;

	ssi_msg (SSI_LOG_LEVEL_BASIC, "sensor disconnected");

	return true;
}

void EHealth::run () {

	if (_serial) {	    
		
		int pos = 0;
		bool found = false;

		// search for new line
		do {
			int r = _serial->ReadData (_buffer + pos, 1);
			if (r > 0 && _buffer[pos++] == '\n') {	
				found = true;
			}			
		} while (!found && pos < SSI_EHEALTH_N_BUFFER);
		
		if (found) {

			_buffer[pos] = '\0';									
			ssi_string2array (SSI_EHEALTH_N_CHANNELS, _values, _buffer, ';');

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "ECG=%.2f GSR=%.2f AIR=%.2f TMP=%.2f BPM=%.2f OXY=%.2f", _values[0], _values[1], _values[2], _values[3], _values[4], _values[5]);	

			for (ssi_size_t i = 0; i < SSI_EHEALTH_N_CHANNELS; i++) {
				if (_provider[i]) {
					_provider[i]->provide (ssi_pcast (ssi_byte_t, _values+i), 1);
				}
			}			

		} else {
			ssi_wrn ("buffer size (%d) reached without new line", SSI_EHEALTH_N_BUFFER);
		}
		
	} else {
		sleep_ms (100);
	}
}

}
