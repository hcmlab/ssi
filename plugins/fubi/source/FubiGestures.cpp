// FubiGestures.cpp
// author: Felix Kistler <kistler@informatik.uni-augsburg.de>
// created: 2012/09/10
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
// version 3 of the License, or any later version.
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

#include "FubiGestures.h"
#include "Fubi/Fubi.h"
#include "FubiLock.h"
#include "base/Factory.h"
#include "ssifubi.h"
#include "OpenNIKInect.h"
#include <string>
#include <sstream>


#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	char *FubiGestures::ssi_log_name = "fubigest__";

	FubiGestures::FubiGestures (const ssi_char_t *file)
		: ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
		_elistener (0),
		_file (0),
		_n_recognizer_postures(0),
		_n_recognizer_combinations(0),
		_recognizerStart(0),
		_recognizerEnd(0),
		_meta_received(0),
		_glue_id(0)
	{

		if (file) {
			if (!OptionList::LoadXML (file, _options)) {
				OptionList::SaveXML (file, _options);
			}
			_file = ssi_strcpy (file);
		}

		_meta_in_ptr = _meta_in;
	}

	FubiGestures::~FubiGestures () {
		
		for(ssi_size_t i=0; i<_n_recognizer_postures; ++i)
			ssi_event_destroy (_events_postures[i]);
		for(ssi_size_t i=0; i<_n_recognizer_combinations; ++i)			
			ssi_event_destroy (_events_combinations[i]);

		if (_n_recognizer_postures > 0)
		{
			delete[] _recognizerActive; _recognizerActive = 0;
			delete[] _recognizerStart; _recognizerStart = 0;
			delete[] _recognizerEnd; _recognizerEnd = 0;
			delete[] _events_postures;
		}

		if (_n_recognizer_combinations > 0)
		{
			delete[] _combinationSucces;
			delete[] _events_combinations;
		}

		if (_file) {
			OptionList::SaveXML (_file, _options);
			delete[] _file;
		}		
	}

	bool FubiGestures::setEventListener (IEventListener *listener) {

		_elistener = listener;
		_event_address.setSender (_options.sname);

		if (!Fubi::isInitialized())
		{
			if (!Fubi::init(Fubi::SensorOptions(Fubi::StreamOptions(-1), Fubi::StreamOptions(-1), Fubi::StreamOptions(-1),
				Fubi::SensorType::NONE)))
				return false;
		}

		if (_options.recognizer_xml[0] != '\0')
		{

			if (!Fubi::loadRecognizersFromXML (_options.recognizer_xml))
			{
				ssi_wrn ("could not parse xml with recognizer definitions '%s'", _options.recognizer_xml);
			}

			_n_recognizer_postures = Fubi::getNumUserDefinedRecognizers ();
			_n_recognizer_combinations = Fubi::getNumUserDefinedCombinationRecognizers ();

			if (_n_recognizer_combinations > 0)
			{
				Fubi::setAutoStartCombinationRecognition(true);
			}
			_combinationSucces =  new bool[_n_recognizer_combinations];
			for (unsigned int i = 0; i < _n_recognizer_combinations; ++i)
			{
				_combinationSucces[i] = false;
			}
		}
		
		/*
		 * define event names
		 */
		_events_postures = new ssi_event_t[_n_recognizer_postures];
		_events_combinations = new ssi_event_t[_n_recognizer_combinations];
		std::stringstream enames;
		
		//postures
		for(ssi_size_t i=0; i<_n_recognizer_postures; ++i)
		{
			enames << Fubi::getUserDefinedRecognizerName(i);			
			if(i != _n_recognizer_postures-1) 
				enames << ",";		
			
			ssi_event_init (_events_postures[i], SSI_ETYPE_EMPTY);
			_events_postures[i].sender_id = Factory::AddString (_options.sname);
			_events_postures[i].event_id = Factory::AddString (Fubi::getUserDefinedRecognizerName(i));
		}
		
		if(_n_recognizer_postures > 0 && _n_recognizer_combinations > 0)
			enames << ",";

		//combinations
		for(ssi_size_t i=0; i<_n_recognizer_combinations; ++i)
		{
			enames << Fubi::getUserDefinedCombinationRecognizerName(i);
			if(i != _n_recognizer_combinations-1) 
				enames << ",";			
			
			ssi_event_init (_events_combinations[i], SSI_ETYPE_EMPTY);
			_events_combinations[i].sender_id = Factory::AddString (_options.sname);
			_events_combinations[i].event_id = Factory::AddString (Fubi::getUserDefinedCombinationRecognizerName(i));
		}

		_event_address.setEvents (enames.str().c_str());

		_recognizerActive = new bool[_n_recognizer_postures];
		_recognizerStart = new ssi_size_t[_n_recognizer_postures];
		_recognizerEnd = new ssi_size_t[_n_recognizer_postures];

		for (unsigned int i= 0; i < _n_recognizer_postures; ++i) {
			_recognizerActive[i] = false;	
			_recognizerStart[i] = 0;
			_recognizerEnd[i] = 0;
		}

		return true;
	}

	void FubiGestures::consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {
	}

	void FubiGestures::consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		ssi_time_t time = consume_info.time;
		ssi_time_t step = 1.0 / stream_in[0].sr;

		ssi_size_t check = 0;

		ssi_size_t sample_dimension = stream_in[0].dim;

		SSI_SKELETON_META *meta = ssi_pcast(SSI_SKELETON_META, _meta_in);

		ssi_size_t num_user = (ssi_size_t)min(meta->num, Fubi::MaxUsers);
		ssi_size_t sample_number = stream_in[0].num;

		if (meta->type == SSI_SKELETON_TYPE::SSI || meta->type == SSI_SKELETON_TYPE::OPENNI_KINECT)
		{
			for(ssi_size_t k=0; k< sample_number; ++k)
			{
				
				if(meta->type == SSI_SKELETON_TYPE::SSI)
				{
					SSI_SKELETON* skel_ssi = ssi_pcast(SSI_SKELETON, stream_in[0].ptr + k * stream_in[0].dim * stream_in[0].byte);
					for (ssi_size_t i = 0; i < num_user; ++i)
					{
						for (ssi_size_t j = 0; j < OpenNIKinect::SkeletonJoint::NUM_JOINTS; ++j)
						{
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_X] = skel_ssi[i][j][SSI_SKELETON_JOINT_VALUE::POS_X];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_Y] = skel_ssi[i][j][SSI_SKELETON_JOINT_VALUE::POS_Y];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_Z] = skel_ssi[i][j][SSI_SKELETON_JOINT_VALUE::POS_Z];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_CONF] = skel_ssi[i][j][SSI_SKELETON_JOINT_VALUE::POS_CONF];

							// Fix limb orientation hierarchy (assumes SSI skeleton uses the Kinect SDK 1/2 hierarchy)
							ssi_size_t orientJointIndex = j;
							if ( (orientJointIndex >= OpenNIKinect::SkeletonJoint::LEFT_SHOULDER && orientJointIndex < OpenNIKinect::SkeletonJoint::LEFT_HAND) 
								|| (orientJointIndex >= OpenNIKinect::SkeletonJoint::RIGHT_SHOULDER && orientJointIndex < OpenNIKinect::SkeletonJoint::RIGHT_HAND)
								|| (orientJointIndex >= OpenNIKinect::SkeletonJoint::LEFT_HIP && orientJointIndex < OpenNIKinect::SkeletonJoint::LEFT_FOOT)
								|| (orientJointIndex >= OpenNIKinect::SkeletonJoint::RIGHT_HIP && orientJointIndex < OpenNIKinect::SkeletonJoint::RIGHT_FOOT) )
								++orientJointIndex;
							if (orientJointIndex == OpenNIKinect::SkeletonJoint::WAIST)
								orientJointIndex = OpenNIKinect::SkeletonJoint::TORSO;

							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_X] = skel_ssi[i][orientJointIndex][SSI_SKELETON_JOINT_VALUE::ROT_X];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_Y] = skel_ssi[i][orientJointIndex][SSI_SKELETON_JOINT_VALUE::ROT_Y];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_Z] = skel_ssi[i][orientJointIndex][SSI_SKELETON_JOINT_VALUE::ROT_Z];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_CONF] = skel_ssi[i][orientJointIndex][SSI_SKELETON_JOINT_VALUE::ROT_CONF];

							// Convert orientations to correct null pose (asssumes SSI skeleton uses the Kinect SDK 1/2 null pose, but the OpenNI coordinate system)
							convertSSIRot(_fubiSkeleton[i][j], j);
						}
					}
				}
				if(meta->type == SSI_SKELETON_TYPE::OPENNI_KINECT)
				{
					OpenNIKinect::SKELETON* skel_oni = ssi_pcast (OpenNIKinect::SKELETON, stream_in[0].ptr + k * stream_in[0].dim * stream_in[0].byte);
					for (ssi_size_t i = 0; i < num_user; ++i)
					{
						for (ssi_size_t j = 0; j < OpenNIKinect::SkeletonJoint::NUM_JOINTS; ++j)
						{
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_X] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::POS_X];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_Y] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::POS_Y];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_Z] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::POS_Z];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::POS_CONF] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::POS_CONF];

							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_X] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::ROT_X];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_Y] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::ROT_Y];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_Z] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::ROT_Z];
							_fubiSkeleton[i][j][OpenNIKinect::JOINT_VALUES::ROT_CONF] = skel_oni[i][j][OpenNIKinect::JOINT_VALUES::ROT_CONF];
						}
					}
				}

				FubiLock::Lock();

				for (ssi_size_t i = 0; i < num_user; ++i)
				{					
					Fubi::updateTrackingData(i+1, (float*)(_fubiSkeleton[i]), time); // take i+1 as the id, i = user
					if (_elistener)
					{
						ssi_size_t timems = ssi_sec2ms(time);
						checkRecognizers(i + 1, timems);
					}
					
				}

				FubiLock::Unlock();

				time += step;
			}
		}
	}

	void FubiGestures::consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

	}

	void FubiGestures::checkRecognizers(unsigned int userID, ssi_size_t time_ms)
	{
		// First postures
		for (ssi_size_t i = 0; i < _n_recognizer_postures; ++i)
		{			
			bool detected = Fubi::recognizeGestureOn (i, userID) == Fubi::RecognitionResult::RECOGNIZED;
			const char *recognizerName = Fubi::getUserDefinedRecognizerName (i);

			if (detected)
			{
				if (!_recognizerActive[i])
				{					
					

					_recognizerStart[i] = time_ms;
					_recognizerActive[i] = true;
					ssi_msg (SSI_LOG_LEVEL_DEBUG, "user %d - gesture start: %s", userID, recognizerName);
					_events_postures[i].time = _recognizerStart[i];
					_events_postures[i].dur = 0;
					_events_postures[i].state = SSI_ESTATE_CONTINUED;
					_events_postures[i].glue_id = _glue_id++;
					//strcpy(_event.ptr, recognizerName);
					//strcat(_event.ptr, "_start");
					_elistener->update (_events_postures[i]);
				}
			}
			else if (_recognizerActive[i])
			{
				
				_recognizerActive[i] = false;
				_recognizerEnd[i] = time_ms;
				ssi_msg (SSI_LOG_LEVEL_DEBUG, "user %d - gesture finished: %s", userID, recognizerName);
				_events_postures[i].time = _recognizerStart[i];
				_events_postures[i].dur = _recognizerEnd[i] - _recognizerStart[i];
				_events_postures[i].state = SSI_ESTATE_COMPLETED;
				//strcpy(_event.ptr, recognizerName);
				_elistener->update (_events_postures[i]);
			}
		}

		// Then combinations
		for (ssi_size_t i = 0; i < _n_recognizer_combinations; ++i)
		{
			std::vector<Fubi::TrackingData> userStates;
			const char* recognizerName = Fubi::getUserDefinedCombinationRecognizerName (i);
			Fubi::RecognitionResult::Result result = Fubi::getCombinationRecognitionProgressOn (recognizerName, userID, &userStates);
			if (result == Fubi::RecognitionResult::WAITING_FOR_LAST_STATE_TO_FINISH)
			{
				// Special case: last state succesful, but not yet finished
				if(!_combinationSucces[i])
				{
					//ssi_print("Start: %d", time_ms);
					ssi_msg (SSI_LOG_LEVEL_DEBUG, "user %d - combination passed min duration of last state: %s", userID, recognizerName);
					_events_combinations[i].dur = 0;
					_events_combinations[i].time = time_ms - (ssi_size_t)((userStates.back().timeStamp - userStates.front().timeStamp) * 1000.0);
					_events_combinations[i].state = SSI_ESTATE_CONTINUED;
					_events_combinations[i].glue_id = _glue_id++;
					//strcpy(_events_combinations[i].ptr, recognizerName);
					//strcat(_event.ptr, "_start");
					_elistener->update (_events_combinations[i]);
				}
				_combinationSucces[i] = true;
			}
			else if (result == Fubi::RecognitionResult::RECOGNIZED)
			{
				//ssi_print("End: %d", time_ms);
				ssi_msg (SSI_LOG_LEVEL_DEBUG, "user %d - combination finished: %s", userID, recognizerName);
				_events_combinations[i].dur = (ssi_size_t)((userStates.back().timeStamp - userStates.front().timeStamp) * 1000.0);				
				_events_combinations[i].state = SSI_ESTATE_COMPLETED;
				if (!_combinationSucces[i]) {
					_events_combinations[i].time = time_ms - _events_combinations[i].dur;
					_events_combinations[i].glue_id = _glue_id++;
				}
				//strcpy(_event.ptr, recognizerName);
				_elistener->update (_events_combinations[i]);
			}
			else
				_combinationSucces[i] = false;
		}
	}

	void FubiGestures::convertSSIRot(float* jointData, ssi_size_t jointID)
	{
		if (jointID <= Fubi::SkeletonJoint::WAIST || jointID >= Fubi::SkeletonJoint::FACE_NOSE)
			// No conversion needed for all root joints and the head (which is calculated separately from the rest of the skeleton)
			return;

		// Import rotation, undoing coordinate system conversion (so we can use the same calculation as in FubiKinectSDK2Sensor.cpp)
		Fubi::Quaternion rot(Fubi::degToRad(jointData[OpenNIKinect::JOINT_VALUES::ROT_X]),
			Fubi::degToRad(jointData[OpenNIKinect::JOINT_VALUES::ROT_Y]),
			Fubi::degToRad(jointData[OpenNIKinect::JOINT_VALUES::ROT_Z]));
		rot *= Fubi::Quaternion(0, Fubi::Math::Pi, 0);

		// Adapt null pose
		switch (jointID)
		{
			// The arms are pointing downwards instead of to the side (T-pose), and we have to change the coordinate system from bone-oriented to axis-oriented
		case Fubi::SkeletonJoint::RIGHT_SHOULDER:
			rot = Fubi::Quaternion(0, 0, -Fubi::Math::PiHalf) * (rot * Fubi::Quaternion(0, -Fubi::Math::PiHalf, 0) * Fubi::Quaternion(Fubi::Math::Pi, 0, 0));
			break;
		case Fubi::SkeletonJoint::RIGHT_ELBOW: case Fubi::SkeletonJoint::RIGHT_WRIST: case Fubi::SkeletonJoint::RIGHT_HAND:
			rot = Fubi::Quaternion(0, 0, -Fubi::Math::PiHalf) * (rot * Fubi::Quaternion(0, 0, Fubi::Math::Pi));
			break;
		case Fubi::SkeletonJoint::LEFT_SHOULDER:
			rot = Fubi::Quaternion(0, 0, Fubi::Math::PiHalf) * (rot * Fubi::Quaternion(0, Fubi::Math::PiHalf, 0) * Fubi::Quaternion(Fubi::Math::Pi, 0, 0));
			break;
		case Fubi::SkeletonJoint::LEFT_ELBOW:	case Fubi::SkeletonJoint::LEFT_WRIST:	case Fubi::SkeletonJoint::LEFT_HAND:
			rot = Fubi::Quaternion(0, 0, Fubi::Math::PiHalf) * (rot * Fubi::Quaternion(Fubi::Math::Pi, 0, 0));
			break;
			// The legs are in correct pose, but again, we have to change the coordinate system from bone-oriented to axis-oriented
		case Fubi::SkeletonJoint::RIGHT_HIP: case Fubi::SkeletonJoint::RIGHT_KNEE: case Fubi::SkeletonJoint::RIGHT_ANKLE: case Fubi::SkeletonJoint::RIGHT_FOOT:
			rot *= Fubi::Quaternion(0, -Fubi::Math::PiHalf, 0) * Fubi::Quaternion(Fubi::Math::Pi, 0, 0);
			break;
		case Fubi::SkeletonJoint::LEFT_HIP: case Fubi::SkeletonJoint::LEFT_KNEE: case Fubi::SkeletonJoint::LEFT_ANKLE: case Fubi::SkeletonJoint::LEFT_FOOT:
			rot *= Fubi::Quaternion(0, Fubi::Math::PiHalf, 0) * Fubi::Quaternion(Fubi::Math::Pi, 0, 0);
			break;
		}

		Fubi::Vec3f convertedRot = Fubi::Matrix3f(rot).getRot(true);
		jointData[OpenNIKinect::JOINT_VALUES::ROT_X] = convertedRot.x;
		jointData[OpenNIKinect::JOINT_VALUES::ROT_Y] = convertedRot.y;
		jointData[OpenNIKinect::JOINT_VALUES::ROT_Z] = convertedRot.z;
	}
}
