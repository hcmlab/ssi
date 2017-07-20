// Iom.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/04
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

#include "Iom.h"

using namespace std;
#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t Iom::ssi_log_name[] = "iom_______";

Iom::Iom (const ssi_char_t *file) 
	: ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
	_buffer_size (0),
	_buffer_count (0),
	_bvp_buffer (0),
	_bvp_buffer_ptr (0),
	_hrv_provider (0),
	_sc_buffer (0),
	_sc_buffer_ptr (0),
	_sc_provider (0),
	_last_bvp_value (0),
	_last_sc_value (0),
	_device (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Iom::~Iom () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool Iom::setProvider (const ssi_char_t *name, IProvider *_provider) {

	if (strcmp (name, SSI_IOM_BVP_PROVIDER_NAME) == 0) {
		setBvpProvider (_provider);
		return true;
	} else if (strcmp (name, SSI_IOM_SC_PROVIDER_NAME) == 0) {
		setScProvider (_provider);
		return true;
	}

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void Iom::setBvpProvider (IProvider *provider) {

	if (_hrv_provider) {
		ssi_wrn ("hrv provider already set");
	}

	_hrv_provider = provider;
	if (_hrv_provider) {
		_hrv_provider->init (&_hrv);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "bvp provider set");
	}
}

void Iom::setScProvider (IProvider *provider) {

	if (_sc_provider) {
		ssi_wrn ("sc provider already set");
	}

	_sc_provider = provider;
	if (_sc_provider) {
		_sc_provider->init (&_sc);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "sc provider set");
	}
}

bool Iom::connect () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to connect sensor...");

	// connect device
	_device = lightstone_create();
	int ret = lightstone_get_count(_device);	
	if(!ret) {
		lightstone_delete (_device);
		_device = 0;
		ssi_wrn ("no lightstones connected");
		return false;
	}
	ssi_msg (SSI_LOG_LEVEL_DEBUG, "found %d lightstones", ret);

	ret = lightstone_open (_device, _options.id);
	if(ret < 0) {
		lightstone_delete (_device);
		_device = 0;
		ssi_wrn ("cannot open lightstone");
		return false;
	}
	
	// prepare buffer
	_buffer_size = ssi_cast (ssi_size_t, _options.block * SSI_IOM_SAMPLE_RATE + 0.5);
	_buffer_count = 0;
	if (_hrv_provider) {
		_bvp_buffer = new SSI_IOM_SAMPLE_TYPE[_buffer_size];
		_bvp_buffer_ptr = _bvp_buffer;
	}
	if (_sc_provider) {
		_sc_buffer = new SSI_IOM_SAMPLE_TYPE[_buffer_size];
		_sc_buffer_ptr = _sc_buffer;
	}

	// set thread name	
	Thread::setName (getName ());

	ssi_msg (SSI_LOG_LEVEL_BASIC, "sensor connected");

	return true;
}

bool Iom::disconnect () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "try to disconnect sensor...");

	if (_device) {
		lightstone_delete (_device);
		_device = 0;
	}

	if (_hrv_provider) {
		delete[] _bvp_buffer; _bvp_buffer = 0;
		_bvp_buffer_ptr = 0;
	}
	if (_sc_provider) {
		delete[] _sc_buffer; _sc_buffer = 0;
		_sc_buffer_ptr = 0;
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "sensor disconnected");

	return true;
}


void Iom::run () {

	if (_device) {

		lightstone_info r = lightstone_get_info (_device);
		
		if(r.hrv < 0) {
			ssi_wrn ("error reading lightstone, shutting down");
			lightstone_delete (_device);
			_device = 0;			
			return;
		}

		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%f %f", r.hrv, r.scl);
		
		// removes weired peaks that occur every now and then
		if (r.hrv > 10.0) {
			r.hrv = _last_bvp_value;
			r.scl = _last_sc_value;
		}

		if (_hrv_provider) {			
			*_bvp_buffer_ptr++ = r.hrv;
			_last_bvp_value = r.hrv;
		}
		if (_sc_provider) {
			*_sc_buffer_ptr++ = r.scl;
			_last_sc_value = r.scl;
		}

		if (++_buffer_count == _buffer_size) {
			if (_hrv_provider) {
				_hrv_provider->provide (ssi_pcast (ssi_byte_t, _bvp_buffer), _buffer_size);
				_bvp_buffer_ptr = _bvp_buffer;
			}
			if (_sc_provider) {
				_sc_provider->provide (ssi_pcast (ssi_byte_t, _sc_buffer), _buffer_size);
				_sc_buffer_ptr = _sc_buffer;
			}
			_buffer_count = 0;
		}

	} else {
		sleep_ms (100);
	}
}


}
