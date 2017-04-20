// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/11 
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

#include "ssiaudio.h"
#include "base/Factory.h"

#ifndef DLLEXP
#if _WIN32||_WIN64
#define DLLEXP extern "C" __declspec( dllexport )
#else
#define DLLEXP extern "C" __attribute__((visibility("default")))
#endif
#endif

DLLEXP bool Register (ssi::Factory *factory, FILE *logfile, ssi::IMessage *message) {

	ssi::Factory::SetFactory (factory);

	if (logfile) {
		ssiout = logfile;
	}
	if (message) {
		ssimsg = message;
	}

	bool result = true;
	#if _WIN32|_WIN64
	result = ssi::Factory::Register (ssi::Audio::GetCreateName (),ssi::Audio::Create) && result;	
	result = ssi::Factory::Register (ssi::AudioPlayer::GetCreateName (),ssi::AudioPlayer::Create) && result;
    #elif __ANDROID__
    result = ssi::Factory::Register (ssi::AudioOpenSL::GetCreateName (),ssi::AudioOpenSL::Create) && result;
    result = ssi::Factory::Register (ssi::AudioOpenSLPlayer::GetCreateName (),ssi::AudioOpenSLPlayer::Create) && result;
    #else
    result = ssi::Factory::Register (ssi::AudioPortaudio::GetCreateName (),ssi::AudioPortaudio::Create) && result;
    result = ssi::Factory::Register (ssi::AudioPortaudioPlayer::GetCreateName (),ssi::AudioPortaudioPlayer::Create) && result;
	#endif
	result = ssi::Factory::Register (ssi::AudioActivity::GetCreateName (),ssi::AudioActivity::Create) && result;
	result = ssi::Factory::Register(ssi::VoiceActivitySender::GetCreateName(), ssi::VoiceActivitySender::Create) && result;
	result = ssi::Factory::Register(ssi::VoiceActivityVerifier::GetCreateName(), ssi::VoiceActivityVerifier::Create) && result;
	result = ssi::Factory::Register (ssi::AudioIntensity::GetCreateName (),ssi::AudioIntensity::Create) && result;
	result = ssi::Factory::Register(ssi::AudioLpc::GetCreateName(), ssi::AudioLpc::Create) && result;
	result = ssi::Factory::Register (ssi::AudioConvert::GetCreateName (),ssi::AudioConvert::Create) && result;
	result = ssi::Factory::Register (ssi::SNRatio::GetCreateName (),ssi::SNRatio::Create) && result;
	result = ssi::Factory::Register (ssi::WavReader::GetCreateName (),ssi::WavReader::Create) && result;	
    result = ssi::Factory::Register (ssi::WavWriter::GetCreateName (),ssi::WavWriter::Create) && result;
    #if _WIN32|_WIN64
    result = ssi::Factory::Register (ssi::AudioMixer::GetCreateName(), ssi::AudioMixer::Create) && result;
	#endif
	result = ssi::Factory::Register (ssi::WavProvider::GetCreateName (),ssi::WavProvider::Create) && result;	
	result = ssi::Factory::Register (ssi::PreEmphasis::GetCreateName (),ssi::PreEmphasis::Create) && result;	
	result = ssi::Factory::Register(ssi::AudioMono::GetCreateName(), ssi::AudioMono::Create) && result;
	result = ssi::Factory::Register(ssi::AudioNoiseGate::GetCreateName(), ssi::AudioNoiseGate::Create) && result;

	return result;
}
