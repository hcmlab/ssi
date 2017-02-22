// OpenNiKinect.cpp
// author: Felix Kistler <kistler@hcm-lab.de>
// created: 2011/01/27
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

#include "OpenNIKinect.h"
#include "base/Factory.h"
#include "SSI_SkeletonCons.h"

#include "Fubi.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#include <deque>

namespace ssi {

ssi_char_t *OpenNIKinect::ssi_log_name = "kinect____";
int OpenNIKinect::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	OpenNIKinect::OpenNIKinect (const ssi_char_t *file) 
		: _file (0),
		_depthProvider(0x0), 
		_sceneProvider(0x0), 
		_irProvider(0x0), 
		_imageProvider(0x0), 
		_nUsers (0),
		_skeletonProvider (0),
		_skeletonBuffer (0),
		_timer (0),
		_event_sender_id (SSI_FACTORY_STRINGS_INVALID_ID),
		_recognizerActive (0),
		_recognizerEvents (0),
		_recognizerIds (0),
		_hangin_counter (0),
		_hangout_counter (0),
		_hangin (0),
		_hangout (0),
		_recognizerStart (0),
		_recognizerEnd (0),
		_listener (0),
		_recognizer (0),
		_n_recognizer (0),
		_n_recognizer_postures (0),
		_n_recognizer_combinations (0),
		_is_fubi_connected (false),
		_recognizer_type(NO_RECOGNIZER)
	{
		_currentCandidate = 0;	

		if (file) {
			if (!OptionList::LoadXML (file, _options)) {
				OptionList::SaveXML (file, _options);
			}
			_file = ssi_strcpy (file);
		}
		
		_channels[0] = new DepthChannel ();
		_channels[1] = new SceneChannel (_options.showRGBScene);
		_channels[2] = new IrChannel ();
		_channels[3] = new RGBChannel ();
		_channels[4] = new SceletonChannel ();
	}

	OpenNIKinect::~OpenNIKinect () {

		if (_file) {
			OptionList::SaveXML (_file, _options);
			delete[] _file;
		}
	}

	bool OpenNIKinect::setProvider (const ssi_char_t *name, IProvider *provider) {
		
		if (strcmp (name, SSI_OPENNI_KINECT_DEPTH_IMAGE_PROVIDER_NAME) == 0) {
			setDepthProvider (provider);
			return true;
		}
		else if (strcmp (name, SSI_OPENNI_KINECT_SCENE_IMAGE_PROVIDER_NAME) == 0) {
			setSceneProvider (provider);
			return true;
		}
		else if (strcmp (name, SSI_OPENNI_KINECT_IR_IMAGE_PROVIDER_NAME) == 0) {
			setIRProvider (provider);
			return true;
		}
		else if (strcmp (name, SSI_OPENNI_KINECT_RGB_IMAGE_PROVIDER_NAME) == 0) {
			setImageProvider (provider);
			return true;
		}
		else if (strcmp (name, SSI_OPENNI_KINECT_SKELETON_PROVIDER_NAME) == 0) {
			setSkeletonProvider (provider);
			return true;
		}

		ssi_wrn ("unkown openni provider name '%s'", name);

		return false;
	}

	void OpenNIKinect::setDepthProvider (IProvider *provider) {
		if (_depthProvider) {
			ssi_wrn ("depth provider already set");
		}

		_depthProvider = provider;
		if (_depthProvider) {
			ssi_video_params_t params = getDepthParams ();
			_depthProvider->setMetaData (sizeof (params), &params);

			((DepthChannel*)_channels[0])->stream.sr = _options.sampleRate;
			_depthProvider->init (_channels[0]);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "Openni: depth provider set");
		}
	}
	void OpenNIKinect::setSceneProvider (IProvider *provider) {
		if (_sceneProvider) {
			ssi_wrn ("scene provider already set");
		}

		_sceneProvider = provider;
		if (_sceneProvider) {
			ssi_video_params_t params = getSceneParams ();
			_sceneProvider->setMetaData (sizeof (params), &params);
			
			((SceneChannel*)_channels[1])->stream.sr = _options.sampleRate;
			_sceneProvider->init (_channels[1]);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "Openni: scene provider set");
		}
	}
	void OpenNIKinect::setImageProvider (IProvider *provider) {
		if (_irProvider)
		{
			ssi_wrn("ir and rgb provider aren't supported simultaneously!");
		}

		if (_imageProvider) {
			ssi_wrn ("rgb provider already set");
		}

		_imageProvider = provider;
		if (_imageProvider) {
			ssi_video_params_t params = getRGBParams ();
			_imageProvider->setMetaData (sizeof (params), &params);
			
			((RGBChannel*)_channels[3])->stream.sr = _options.sampleRate;
			_imageProvider->init(_channels[3]);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "rgb provider set");
		}
	}
	void OpenNIKinect::setIRProvider (IProvider *provider) {
		if (_imageProvider)
		{
			ssi_wrn("ir and rgb provider aren't supported simultaneously");
		}

		if (_irProvider) {
			ssi_wrn ("ir provider already set");
		}

		_irProvider = provider;
		if (_irProvider) {
			ssi_video_params_t params = getIRParams ();
			_irProvider->setMetaData (sizeof (params), &params);
			
			((IrChannel*)_channels[2])->stream.sr = _options.sampleRate;
			_irProvider->init(_channels[2]);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "ir provider set");
		}
	}
	void OpenNIKinect::setSkeletonProvider (IProvider *provider) {
		if (_skeletonProvider) {
			ssi_wrn ("skeleton provider already set");
		}

		_skeletonProvider = provider;
		if (_skeletonProvider) {			
			SSI_SKELETON_META m;
			m.num = _options.users;
			m.type = SSI_SKELETON_TYPE::OPENNI_KINECT;
			_skeletonProvider->setMetaData (sizeof (SSI_SKELETON_META), &m);

			ssi_pcast (SceletonChannel, _channels[SSI_OPENNI_KINECT_SCELETON_CHANNEL_NUM])->stream.sr = _options.sampleRate;
			ssi_pcast(SceletonChannel, _channels[SSI_OPENNI_KINECT_SCELETON_CHANNEL_NUM])->stream.dim = JOINT_VALUES::NUM * SkeletonJoint::NUM_JOINTS * _options.users;
			
			_skeletonProvider->init (_channels[SSI_OPENNI_KINECT_SCELETON_CHANNEL_NUM]);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "skeleton provider set");
		}
	}

	bool OpenNIKinect::setEventListener (IEventListener *listener) {

		_listener = listener;
		_event_sender_id = Factory::AddString (_options.sname);
		if (_event_sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		if (!init_fubi ()) {
			return false;
		}

		if (_options.recognizer_xml[0] != '\0') {

			_recognizer_type = XML;

			if (!Fubi::loadRecognizersFromXML (_options.recognizer_xml)) {
				ssi_wrn ("could not parse xml with recognizer definitions '%s'", _options.recognizer_xml);
			}

			_n_recognizer_postures = Fubi::getNumUserDefinedRecognizers ();
			_n_recognizer_combinations = Fubi::getNumUserDefinedCombinationRecognizers ();
			_n_recognizer = _n_recognizer_postures + _n_recognizer_combinations;

			if (_n_recognizer_combinations > 0) {
				Fubi::setAutoStartCombinationRecognition (true);
			}

			if (_n_recognizer > 0) {
				_recognizer = new int[_n_recognizer];
				for (ssi_size_t i = 0; i < _n_recognizer_postures; i++) {
					_recognizer[i] = i;
				}
				for (ssi_size_t i = 0; i < _n_recognizer_combinations; i++) {
					_recognizer[_n_recognizer_postures+i] = i;
				}
			} else {
				ssi_wrn ("no recognizer found in xml with recognizer definition '%s'", _options.recognizer_xml);
			}


		} else {
			
			_recognizer_type = INDICES;

			if (_options.recognizer[0] != '\0') {
				_recognizer = ssi_parse_indices (_options.recognizer, _n_recognizer);				
			}
		}

		if(_recognizer_type != NO_RECOGNIZER)
		{
			_hangin = _options.hangin;
			_hangout = _options.hangout;
			_recognizerActive = new bool[_n_recognizer];
			_recognizerIds = new ssi_size_t[_n_recognizer];
			_recognizerStart = new ssi_size_t[_n_recognizer];
			_recognizerEnd = new ssi_size_t[_n_recognizer];
			_hangin_counter = new ssi_size_t[_n_recognizer];
			_hangout_counter = new ssi_size_t[_n_recognizer];
			_recognizerEvents = new ssi_event_t[_n_recognizer];
			for (unsigned int i= 0; i < _n_recognizer; ++i) {
				_recognizerActive[i] = false;
				if (_recognizer_type == XML) {
					if (i >= _n_recognizer_postures) {
						_recognizerIds[i] = Factory::AddString (Fubi::getUserDefinedCombinationRecognizerName (_recognizer[i]));						
					} else {
						_recognizerIds[i] = Factory::AddString (Fubi::getUserDefinedRecognizerName (_recognizer[i]));
					}
				} else {
					_recognizerIds[i] = Factory::AddString (Fubi::getPostureName (ssi_cast (Fubi::Postures::Posture, _recognizer[i])));
				}				
				ssi_event_init (_recognizerEvents[i], SSI_ETYPE_EMPTY, _event_sender_id, _recognizerIds[i]);
				_recognizerStart[i] = 0;
				_recognizerEnd[i] = 0;
				_hangin_counter[i] = 0;
				_hangout_counter[i] = 0;
			}
		}

		return true;
	}

	bool OpenNIKinect::connect () {

		if (!init_fubi ()) {
			return false;
		}

		_nUsers = _options.users;
		if (_skeletonProvider) {
			_skeletonBuffer = new SKELETON[_nUsers];
		}

		// set thread name		
		Thread::setName (getName ());

		return true;
	}

	bool OpenNIKinect::init_fubi () {

		if (_is_fubi_connected)
		{
			return true;
		}

		ssi_video_params_t depthP = getDepthParams();
		ssi_video_params_t rgbP = getRGBParams();
		ssi_video_params_t irP = getIRParams();
		_is_fubi_connected = Fubi::init(
			Fubi::SensorOptions(Fubi::StreamOptions(depthP.widthInPixels, depthP.heightInPixels, (int)depthP.framesPerSecond),
								Fubi::StreamOptions(rgbP.widthInPixels, rgbP.heightInPixels, (int)rgbP.framesPerSecond),
								Fubi::StreamOptions(-1,-1,-1), // IR stream deactivated
								Fubi::SensorType::OPENNI2, (Fubi::SkeletonTrackingProfile::Profile)_options.profile),
			Fubi::FilterOptions(0.5f, 1.0f, 0.005f)	// stronger filter especially for head nod/shakes
			);

		return _is_fubi_connected;
	}

	bool OpenNIKinect::disconnect () {

		Fubi::release();
		_is_fubi_connected = false;
		
		delete _timer; _timer = 0;
		_nUsers = 0;
		delete[] _skeletonBuffer;
		_skeletonBuffer = 0;		

		if (_listener && _recognizer_type != NO_RECOGNIZER) {
			delete[] _recognizerEvents; _recognizerEvents = 0;
			delete[] _recognizerActive; _recognizerActive = 0;
			delete[] _recognizerIds; _recognizerIds = 0;			
			delete[] _recognizerStart; _recognizerStart = 0;
			delete[] _recognizerEnd; _recognizerEnd = 0;
			delete[] _recognizer; _recognizer = 0;
			delete[] _hangin_counter; _hangin_counter = 0;
			delete[] _hangout_counter; _hangout_counter = 0;
			_hangin = 0;
			_hangout = 0;
			_n_recognizer = 0;
		}

		ssi_msg (SSI_LOG_LEVEL_BASIC, "disconnected");
		return true;
	}

	void OpenNIKinect::run () {

		if (!_timer) {
			_timer = new Timer (1/_options.sampleRate);
		}

		Fubi::updateSensor();

		
		// Provide scene data
		if (_sceneProvider)
		{
			ssi_video_params_t param = getSceneParams();
			Fubi::ImageType::Type type = Fubi::ImageType::Depth;
			unsigned int renderOpts = _options.rendering;
			if (_options.showRGBScene)
			{
				type = Fubi::ImageType::Color;
				// Flip the r and b channel as SSI wants it the other way around
				if (renderOpts & Fubi::RenderOptions::SwapRAndB)
					renderOpts &= ~Fubi::RenderOptions::SwapRAndB;
				else
					renderOpts |= Fubi::RenderOptions::SwapRAndB;
			}
			Fubi::getImage((unsigned char*)_sceneBuffer, type, (Fubi::ImageNumChannels::Channel)param.numOfChannels, Fubi::ImageDepth::D8,
				renderOpts, _options.jointsToRender, ssi_cast (Fubi::DepthImageModification::Modification, _options.depthImageModification));

			_sceneProvider->provide(_sceneBuffer, 1);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide scene data");
		}

		// Get the data
		if (_depthProvider)
		{
			ssi_video_params_t param = getDepthParams();
			Fubi::getImage((unsigned char*)_depthBuffer, Fubi::ImageType::Depth, (Fubi::ImageNumChannels::Channel)param.numOfChannels, 
				(Fubi::ImageDepth::Depth) param.depthInBitsPerChannel, Fubi::RenderOptions::None, Fubi::DepthImageModification::Raw);
			_depthProvider->provide(_depthBuffer, 1);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide depth data");
		}
		
		if (_imageProvider)
		{
			ssi_video_params_t param = getRGBParams();
			Fubi::getImage((unsigned char*)_rgbBuffer, Fubi::ImageType::Color, (Fubi::ImageNumChannels::Channel)param.numOfChannels, 
				(Fubi::ImageDepth::Depth) param.depthInBitsPerChannel, Fubi::RenderOptions::SwapRAndB);

			_imageProvider->provide(_rgbBuffer, 1);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide image data");
		}

		if (_irProvider)
		{
			ssi_video_params_t param = getIRParams();
			Fubi::getImage((unsigned char*)_irBuffer, Fubi::ImageType::IR, (Fubi::ImageNumChannels::Channel)param.numOfChannels, 
				(Fubi::ImageDepth::Depth) param.depthInBitsPerChannel, Fubi::RenderOptions::None);

			_irProvider->provide(_irBuffer, 1);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide ir data");
		}

		if (_skeletonProvider) {
				
			for (int i = 0; i < _nUsers; ++i)
			{
				EmptySkeleton (_skeletonBuffer[i]);
			}

			std::deque<FubiUser*> users = Fubi::getClosestUsers(_nUsers);
			std::deque<FubiUser*>::iterator iter = users.begin();
			std::deque<FubiUser*>::iterator end = users.end();
			for (int i = 0; iter != end; ++i, ++iter)
			{
				SKELETON &skel = _skeletonBuffer[i];
				FubiUser* user = *iter;
					
				for (ssi_size_t j = 0; j < Fubi::SkeletonJoint::NUM_JOINTS; j++)
				{
					const Fubi::SkeletonJointPosition& pos = user->currentTrackingData()->jointPositions[(Fubi::SkeletonJoint::Joint)j];
					const Fubi::SkeletonJointOrientation& ori = user->currentTrackingData()->jointOrientations[(Fubi::SkeletonJoint::Joint)j];
					const Fubi::SkeletonJointOrientation& locOri = user->currentTrackingData()->localJointOrientations[(Fubi::SkeletonJoint::Joint)j];

					if (_options.project)
					{
						Fubi::Vec3f pPos = Fubi::convertCoordinates(pos.m_position, Fubi::CoordinateType::REAL_WORLD, Fubi::CoordinateType::DEPTH);
						if (_options.scale)
						{
							pPos.x /= 640;
							pPos.y /= 480;

							if (_options.flip)
								pPos.y = 1.0f - pPos.y;
						}
					}

					Fubi::Vec3f rot = ori.m_orientation.getRot(false);
					Fubi::Vec3f localRot = locOri.m_orientation.getRot(false);

					SetSkeletonPos (skel, (SkeletonJoint::Joint) j, pos.m_position.x, pos.m_position.y, pos.m_position.z, pos.m_confidence);
					SetSkeletonRot (skel, (SkeletonJoint::Joint) j, rot.x, rot.y, rot.z, ori.m_confidence);
					SetSkeletonLocRot (skel, (SkeletonJoint::Joint) j, localRot.x, localRot.y, localRot.z, locOri.m_confidence);
				}

				if (_listener) {		
					checkPostures (user->id());					
				}
			}
			_skeletonProvider->provide (ssi_pcast (ssi_byte_t, _skeletonBuffer), 1);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide skeleton data");
		}

		//// Update the context				
		_timer->wait ();				
	}

	SSI_INLINE void OpenNIKinect::EmptySkeleton (SKELETON &s) 
	{
		memset(s, SSI_OPENNI_KINECT_SELECTOR_INVALID_TRANSF_VALUE, SkeletonJoint::NUM_JOINTS*JOINT_VALUES::NUM*sizeof(float));
	}

	SSI_INLINE void OpenNIKinect::SetSkeletonPos (SKELETON &s, SkeletonJoint::Joint joint, float x, float y, float z, float conf) 
	{
		s[joint][JOINT_VALUES::POS_X] = x;
		s[joint][JOINT_VALUES::POS_Y] = y;
		s[joint][JOINT_VALUES::POS_Z] = z;
		s[joint][JOINT_VALUES::POS_CONF] = conf;
	}

	SSI_INLINE void OpenNIKinect::SetSkeletonRot (SKELETON &s, SkeletonJoint::Joint joint, float rx, float ry, float rz, float conf) 
	{
		s[joint][JOINT_VALUES::ROT_X] = rx;
		s[joint][JOINT_VALUES::ROT_Y] = ry;
		s[joint][JOINT_VALUES::ROT_Z] = rz;
		s[joint][JOINT_VALUES::ROT_CONF] = conf;
	}

	SSI_INLINE void OpenNIKinect::SetSkeletonLocRot (SKELETON &s, SkeletonJoint::Joint joint, float rx, float ry, float rz, float conf) 
	{
		s[joint][JOINT_VALUES::LOCROT_X] = rx;
		s[joint][JOINT_VALUES::LOCROT_Y] = ry;
		s[joint][JOINT_VALUES::LOCROT_Z] = rz;
		s[joint][JOINT_VALUES::LOCROT_CONF] = conf;
	}

	void OpenNIKinect::checkPostures(unsigned int userID)
	{
		if(_recognizer_type == NO_RECOGNIZER)
			return;

		static ITheFramework *frame = Factory::GetFramework ();

		for (unsigned int i = 0; i < _n_recognizer; ++i)
		{			
			bool detected = false;
			const char *recognizerName = 0;
			if (_recognizer_type == XML) {
				if (i >= _n_recognizer_postures) {
					recognizerName = Fubi::getUserDefinedCombinationRecognizerName (_recognizer[i]);
					detected = Fubi::getCombinationRecognitionProgressOn (recognizerName, userID) == Fubi::RecognitionResult::RECOGNIZED;					
				} else {					
					recognizerName = Fubi::getUserDefinedRecognizerName (_recognizer[i]);
					detected = Fubi::recognizeGestureOn (recognizerName, userID) == Fubi::RecognitionResult::RECOGNIZED;
				}
			} else {
				Fubi::Postures::Posture p = (Fubi::Postures::Posture) _recognizer[i];
				detected = Fubi::recognizeGestureOn (p, userID) == Fubi::RecognitionResult::RECOGNIZED;
				recognizerName = Fubi::getPostureName(p);
			}

			if (detected)
			{
				if (!_recognizerActive[i]) {					
					_recognizerStart[i] = frame->GetElapsedTimeMs ();
					_recognizerActive[i] = true;					
					_hangin_counter[i] = 0;					
				}				

				if (_hangin_counter[i]++ == _hangin) {
					ssi_msg (SSI_LOG_LEVEL_DEFAULT, "user %d - gesture start: %s", userID, recognizerName);
				}

				_hangout_counter[i] = 0;
			}
			else if (_recognizerActive[i])
			{
				if (_hangin_counter[i] <= _hangin) {
					_recognizerActive[i] = false;
				} else {					
					if (_hangout_counter[i] == 0) {						
						_recognizerEnd[i] = frame->GetElapsedTimeMs ();
					}
					if (_hangout_counter[i]++ == _hangout) {
						ssi_msg (SSI_LOG_LEVEL_DEFAULT, "user %d - gesture finished: %s", userID, recognizerName);
						_recognizerActive[i] = false;
						_recognizerEvents[i].time = _recognizerStart[i];
						_recognizerEvents[i].dur = _recognizerEnd[i] - _recognizerStart[i];
						_listener->update (_recognizerEvents[i]);
					}
				}
			}
		}
	}
}

