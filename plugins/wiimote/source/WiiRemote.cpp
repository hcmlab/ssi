// WiiRemote.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/20
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

#include "WiiRemote.h"


#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *WiiRemote::ssi_log_name = "wiiremote_";

WiiRemote::WiiRemote (const ssi_char_t *file) 
	: _buffer_size (0),
	_buffer_counter (0),
	_acc_provider (0),
	_acc_buffer (0),
	_ori_provider (0),
	_ori_buffer (0),
	_but_provider (0),
	_but_buffer (0),
	_ir_buffer (0),
	_ir_provider (0),
	_irraw_buffer (0),
	_irraw_provider (0),
	_mpraw_provider (0),
	_mpraw_buffer (0),
	_mpflt_provider (0), 
	_mpflt_buffer (0),
	_timer (0),
	_motion_plus_enabled (false),
	_file (0),
  counterMotionPlus(0),

	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	_channels[SSI_WII_ACC_CHANNEL_NUM] = new AccChannel ();
	_channels[SSI_WII_ORI_CHANNEL_NUM] = new OriChannel ();
	_channels[SSI_WII_BUT_CHANNEL_NUM] = new ButChannel ();
  _channels[SSI_WII_MPRAW_CHANNEL_NUM] = new MPRawChannel ();
	_channels[SSI_WII_MPFLT_CHANNEL_NUM] = new MPFltChannel ();
	_channels[SSI_WII_IRFLT_CHANNEL_NUM] = new IrFltChannel ();
	_channels[SSI_WII_IRRAW_CHANNEL_NUM] = new IrRawChannel ();

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

WiiRemote::~WiiRemote () {

	for (ssi_size_t i = 0; i < SSI_WII_CHANNEL_NUM; i++) {
		delete _channels[i];
	}

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool WiiRemote::setProvider (const ssi_char_t *name, IProvider *provider) {

	if (strcmp (name, SSI_WII_ACC_PROVIDER_NAME) == 0) {
		setAccelerationProvider (provider);
		return true;
	} else if (strcmp (name, SSI_WII_ORI_PROVIDER_NAME) == 0) {
		setOrientationProvider (provider);
		return true;
	} else if (strcmp (name, SSI_WII_BUT_PROVIDER_NAME) == 0) {
		setButtonProvider (provider);
		return true;
	} else if (strcmp (name, SSI_WII_MPRAW_PROVIDER_NAME) == 0) {
		setMotionPlusRawProvider (provider);
		return true;
	} else if (strcmp (name, SSI_WII_MPFLT_PROVIDER_NAME) == 0) {
		setMotionPlusFloatProvider (provider);
		return true;
	} else if (strcmp (name, SSI_WII_IRFLT_PROVIDER_NAME) == 0) {
		setInfraredProvider (provider);
		return true;
	} else if (strcmp (name, SSI_WII_IRRAW_PROVIDER_NAME) == 0) {
		setInfraredRawProvider (provider);
		return true;
	} 

	ssi_wrn ("unkown provider name '%s'", name);

	return false;
}

void WiiRemote::setAccelerationProvider (IProvider *provider) {

	if (_acc_provider) {
		ssi_wrn ("already set");
	}

	_acc_provider = provider;
	if (_acc_provider) {
		_acc_provider->init (_channels[SSI_WII_ACC_CHANNEL_NUM]);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "acceleration provider set");
	}
}

void WiiRemote::setOrientationProvider (IProvider *provider) {

	if (_ori_provider) {
		ssi_wrn ("already set");
	}

	_ori_provider = provider;
	if (_ori_provider) {
		_ori_provider->init (_channels[SSI_WII_ORI_CHANNEL_NUM]);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "orientation provider set");
	}
}

void WiiRemote::setButtonProvider (IProvider *provider) {

	if (_but_provider) {
		ssi_wrn ("already set");
	}

	_but_provider = provider;
	if (_but_provider) {
		_but_provider->init (_channels[SSI_WII_BUT_CHANNEL_NUM]);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "button provider set");
	}
}


void WiiRemote::setMotionPlusRawProvider (IProvider *provider) {

	if (_mpraw_provider) {
		ssi_wrn ("already set");
	}

	_mpraw_provider = provider;
	if (_mpraw_provider) {
		_motion_plus_enabled = true;
		_mpraw_provider->init (_channels[SSI_WII_MPRAW_CHANNEL_NUM]);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "motion plus raw provider set");
	}
}

void WiiRemote::setMotionPlusFloatProvider (IProvider *provider) {	

	if (_mpflt_provider) {
		ssi_wrn ("already set");
	}

	_mpflt_provider = provider;
	if (_mpflt_provider) {
		_motion_plus_enabled = true;
		_mpflt_provider->init (_channels[SSI_WII_MPFLT_CHANNEL_NUM]);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "motion plus float provider set");
	}
}

void WiiRemote::setInfraredProvider (IProvider *provider) {

	if (_ir_provider) {
		ssi_wrn ("already set");
	}

	_ir_provider = provider;
	if (_ir_provider) {
		_ir_provider->init (_channels[SSI_WII_IRFLT_CHANNEL_NUM]);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "infrared provider set");
	}
}

void WiiRemote::setInfraredRawProvider (IProvider *provider) {

	if (_irraw_provider) {
		ssi_wrn ("already set");
	}

	_irraw_provider = provider;
	if (_irraw_provider) {
		_irraw_provider->init (_channels[SSI_WII_IRRAW_CHANNEL_NUM]);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "infrared raw provider set");
	}
}

bool WiiRemote::connect () {

	_buffer_size = ssi_cast (ssi_size_t, SSI_WII_SAMPLE_RATE * _options.size + 0.5);

	if (_acc_provider) {
		_acc_buffer = new SSI_WII_ACC_SAMPLE_TYPE[_buffer_size * _channels[SSI_WII_ACC_CHANNEL_NUM]->getStream ().dim];
		_acc_buffer_ptr = _acc_buffer;
	}

	if (_ori_provider) {
		_ori_buffer = new SSI_WII_ORI_SAMPLE_TYPE[_buffer_size * _channels[SSI_WII_ORI_CHANNEL_NUM]->getStream ().dim];
		_ori_buffer_ptr = _ori_buffer;
	}

	if (_but_provider) {
		_but_buffer = new SSI_WII_BUT_SAMPLE_TYPE[_buffer_size * _channels[SSI_WII_BUT_CHANNEL_NUM]->getStream ().dim];
		_but_buffer_ptr = _but_buffer;
	}

	if (_mpraw_provider) {
		_mpraw_buffer = new SSI_WII_MPRAW_SAMPLE_TYPE[_buffer_size * _channels[SSI_WII_MPRAW_CHANNEL_NUM]->getStream ().dim];
		_mpraw_buffer_ptr = _mpraw_buffer;
	}

	if (_mpflt_provider) {
		_mpflt_buffer = new SSI_WII_MPFLT_SAMPLE_TYPE[_buffer_size * _channels[SSI_WII_MPFLT_CHANNEL_NUM]->getStream ().dim];
		_mpflt_buffer_ptr = _mpflt_buffer;
	}

	if (_ir_provider) {
		_ir_buffer = new SSI_WII_IRFLT_SAMPLE_TYPE[_buffer_size * _channels[SSI_WII_IRFLT_CHANNEL_NUM]->getStream ().dim];
		_ir_buffer_ptr = _ir_buffer;
	}

	if (_irraw_provider) {
		_irraw_buffer = new SSI_WII_IRRAW_SAMPLE_TYPE[_buffer_size * _channels[SSI_WII_IRRAW_CHANNEL_NUM]->getStream ().dim];
		_irraw_buffer_ptr = _irraw_buffer;
	}

	// connect _wiimote
	bool status = _wii.Connect (ssi_cast (unsigned int, _options.device == wiimote::FIRST_AVAILABLE ? wiimote::FIRST_AVAILABLE : _options.device + 1));

	if (status) {
		
    _wii.SetReportType (wiimote::IN_BUTTONS_ACCEL_EXT);

	}

	if (status && (_irraw_provider ||_ir_provider)) {
    _wii.SetReportType (wiimote::IN_BUTTONS_ACCEL_IR_EXT);
	}

	if (!status) {
		ssi_wrn ("could not connect [%d]", _options.device);
	}

	_buffer_counter = 0;
	_timer = 0;

	if (status) {

		ssi_msg (SSI_LOG_LEVEL_BASIC, "connected [%d]", _options.device);

		if (_acc_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         acceleration\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
				SSI_WII_SAMPLE_RATE, 
				_buffer_size / SSI_WII_SAMPLE_RATE,
				_channels[SSI_WII_ACC_CHANNEL_NUM]->getStream ().dim, 
			    _channels[SSI_WII_ACC_CHANNEL_NUM]->getStream ().byte);
		}

		if (_ori_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         orientation\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
				SSI_WII_SAMPLE_RATE, 
				_buffer_size / SSI_WII_SAMPLE_RATE,
				_channels[SSI_WII_ORI_CHANNEL_NUM]->getStream ().dim, 
				_channels[SSI_WII_ORI_CHANNEL_NUM]->getStream ().byte);
		}

		if (_but_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         button\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
				SSI_WII_SAMPLE_RATE, 
				_buffer_size / SSI_WII_SAMPLE_RATE,
			   _channels[SSI_WII_BUT_CHANNEL_NUM]->getStream ().dim, 
			   _channels[SSI_WII_BUT_CHANNEL_NUM]->getStream ().byte);
		}

		if (_mpraw_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         yaw raw\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
				SSI_WII_SAMPLE_RATE, 
				_buffer_size / SSI_WII_SAMPLE_RATE,
			   _channels[SSI_WII_MPRAW_CHANNEL_NUM]->getStream ().dim, 
			   _channels[SSI_WII_MPRAW_CHANNEL_NUM]->getStream ().byte);
		}

		if (_mpflt_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         yaw flt\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
				SSI_WII_SAMPLE_RATE, 
				_buffer_size / SSI_WII_SAMPLE_RATE,
			   _channels[SSI_WII_MPFLT_CHANNEL_NUM]->getStream ().dim, 
			   _channels[SSI_WII_MPFLT_CHANNEL_NUM]->getStream ().byte);
		}

		if (_irraw_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         infrared raw\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
			   SSI_WII_SAMPLE_RATE, 
			   _buffer_size / SSI_WII_SAMPLE_RATE,
			   _channels[SSI_WII_IRRAW_CHANNEL_NUM]->getStream ().dim, 
			   _channels[SSI_WII_IRRAW_CHANNEL_NUM]->getStream ().byte);
		}

		if (_ir_provider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL) {
			ssi_print ("\
         infrared\n\
         rate\t= %.2lf\t\n\
         size\t= %.2lf\n\
         dim\t= %d\n\
         bytes\t= %d\n",
			   SSI_WII_SAMPLE_RATE, 
			   _buffer_size / SSI_WII_SAMPLE_RATE,
			   _channels[SSI_WII_IRFLT_CHANNEL_NUM]->getStream ().dim, 
			   _channels[SSI_WII_IRFLT_CHANNEL_NUM]->getStream ().byte);
		}

	}

	// set thread name
	ssi_char_t thread_name[SSI_MAX_CHAR];
	ssi_sprint (thread_name, "%s@%d", getName (), _options.device);
	Thread::setName (thread_name);

	return status;
}

bool WiiRemote::disconnect () {

  _wii.Disconnect ();
	Sleep (200);
	
	ssi_msg (SSI_LOG_LEVEL_BASIC, "disconnected [%d]", _options.device);

	delete _timer;
	delete[] _acc_buffer;
	_acc_buffer = 0;
	delete[] _ori_buffer;
	_ori_buffer = 0;
	delete[] _but_buffer;
	_but_buffer = 0;		
	delete[] _mpraw_buffer;
  _mpraw_buffer = 0;		
	delete[] _mpflt_buffer;
	_mpflt_buffer = 0;		
	delete[] _ir_buffer;
	_ir_buffer = 0;	
	delete[] _irraw_buffer;
	_irraw_buffer = 0;
	_buffer_counter = 0;
	_buffer_size = 0;

	return true;
}


void WiiRemote::run () {

	if (!_timer) {
		_timer = new Timer (1.0 / SSI_WII_SAMPLE_RATE);
	}

  int status = _wii.RefreshState ();
  
  if (_wii.ConnectionLost ()) {
		ssi_wrn ("connection lost [%d]", _options.device);
		sleep_s (1.0);
		return;
	}

	++_buffer_counter;

	if (_acc_provider) {

    *_acc_buffer_ptr++ = _wii.Acceleration.X;
    *_acc_buffer_ptr++ = _wii.Acceleration.Y;
    *_acc_buffer_ptr++ = _wii.Acceleration.Z;

		if (_buffer_counter == _buffer_size) {

			_acc_provider->provide (ssi_pcast (ssi_byte_t, _acc_buffer), _buffer_size);
			_acc_buffer_ptr = _acc_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide acceleration data");
		}
	}

	if (_ori_provider) {

    *_ori_buffer_ptr++ = _wii.Acceleration.Orientation.Pitch;
    *_ori_buffer_ptr++ = _wii.Acceleration.Orientation.Roll;

		if (_buffer_counter == _buffer_size) {

			_ori_provider->provide (ssi_pcast (ssi_byte_t, _ori_buffer), _buffer_size);
			_ori_buffer_ptr = _ori_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide orientation data");
		}
	}

	if (_but_provider) {

    *_but_buffer_ptr++ = _wii.Button.Bits & ALL;

		if (_buffer_counter == _buffer_size) {

			_but_provider->provide (ssi_pcast (ssi_byte_t, _but_buffer), _buffer_size);
			_but_buffer_ptr = _but_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide button data");
		}
	}

	if (_mpraw_provider) {

    _wii.ChangedCallback = on_state_change;
   
		*_mpraw_buffer_ptr++ = _wii.MotionPlus.Raw.Roll;
    *_mpraw_buffer_ptr++ = _wii.MotionPlus.Raw.Pitch;
    *_mpraw_buffer_ptr++ = _wii.MotionPlus.Raw.Yaw;

		if (_buffer_counter == _buffer_size) {

      int buffer=_buffer_size;
			_mpraw_provider->provide (ssi_pcast (ssi_byte_t, _mpraw_buffer),_buffer_size);
			_mpraw_buffer_ptr = _mpraw_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide motion plus data (raw)");
		}
	}

	if (_mpflt_provider) {

    _wii.ChangedCallback = on_state_change;

    *_mpflt_buffer_ptr++ = _wii.MotionPlus.Speed.Roll;
    *_mpflt_buffer_ptr++ = _wii.MotionPlus.Speed.Pitch;
    *_mpflt_buffer_ptr++ = _wii.MotionPlus.Speed.Yaw;

		if (_buffer_counter == _buffer_size) {

			_mpflt_provider->provide (ssi_pcast (ssi_byte_t, _mpflt_buffer), _buffer_size);
			_mpflt_buffer_ptr = _mpflt_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide motion plus data (float)");
		}
	}

	if (_ir_provider) {

    *_ir_buffer_ptr++ = _wii.IR.Dot[0].X;
    *_ir_buffer_ptr++ = _wii.IR.Dot[0].Y;

		if (_buffer_counter == _buffer_size) {

			_ir_provider->provide (ssi_pcast (ssi_byte_t, _ir_buffer), _buffer_size);
			_ir_buffer_ptr = _ir_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide infrared data");
		}
	}

	if (_irraw_provider) {

    *_irraw_buffer_ptr++ = _wii.IR.Dot[0].RawX;
    *_irraw_buffer_ptr++ = _wii.IR.Dot[0].RawY;

		if (_buffer_counter == _buffer_size) {

			_irraw_provider->provide (ssi_pcast (ssi_byte_t, _irraw_buffer), _buffer_size);
			_irraw_buffer_ptr = _irraw_buffer;

			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide infrared raw data");
		}
	}


	if (_buffer_counter == _buffer_size) {
		_buffer_counter = 0;
	}
	_timer->wait ();
}


void WiiRemote::on_state_change (wiimote &_wiimote, state_change_flags changed,const wiimote_state &remote_state)
{
  // a MotionPlus was detected
  if(changed & MOTIONPLUS_DETECTED)
	{
	// enable it if there isn't a normal extension plugged into it
	// (MotionPlus devices don't report like normal extensions until
	//  enabled - and then, other extensions attached to it will no longer be
	//  reported (so disable it when you want to access to those again).

	if(_wiimote.ExtensionType == wiimote_state::NONE) {
		bool res = _wiimote.EnableMotionPlus();
		_ASSERT(res);
		}
	}
else if(changed & MOTIONPLUS_EXTENSION_CONNECTED)
	{
	// an extension is connected to the MotionPlus.  We can't read it if the
	//  MotionPlus is currently enabled, so disable it:

	if(_wiimote.MotionPlusEnabled())
		_wiimote.DisableMotionPlus();
	}
else if(changed & MOTIONPLUS_EXTENSION_DISCONNECTED)
	{
	// the extension plugged into the MotionPlus was removed, so enable
	//  the MotionPlus data again:

	if(_wiimote.MotionPlusConnected())
		_wiimote.EnableMotionPlus();
	}
// extension was just connected:
else if(changed & EXTENSION_CONNECTED)
	{
/*#ifdef USE_BEEPS_AND_DELAYS
	Beep(1000, 200);
#endif*/
	// switch to a report mode that includes the extension data (we will
	//  loose the IR dot sizes)
	// note: there is no need to set report types for a Balance Board.
	if(!_wiimote.IsBalanceBoard())
		_wiimote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
	}
// extension was just disconnected:
else if(changed & EXTENSION_DISCONNECTED)
	{
/*#ifdef USE_BEEPS_AND_DELAYS
	Beep(200, 300);
#endif*/
	// use a non-extension report mode (this gives us back the IR dot sizes)

	_wiimote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);
	}
}

}
