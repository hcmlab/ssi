// AudioAsioPlayer.cpp
// author: Simon Flutura <simon.flutura@informatik.uni-augsburg.de>
// created: 2009/08/19
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

#include "AudioOpenSLPlayer.h"
#include "graphic/DialogLibGateway.h"



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

ssi_char_t *AudioOpenSLPlayer::ssi_log_name = "audioandrp";

AudioOpenSLPlayer::AudioOpenSLPlayer (const ssi_char_t *file)
	:  _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

}

AudioOpenSLPlayer::~AudioOpenSLPlayer () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void AudioOpenSLPlayer::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	// init portaudio

        SLresult result;

        // create engine
        result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;

        // realize the engine
        result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;

        // get the engine interface, which is needed in order to create other objects
        result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;


        //ssi_err ("could not initialize portaudio: %s", Pa_GetErrorText(status));


	
}
void AudioOpenSLPlayer::VbqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){


}
// this callback handler is called every time a buffer finishes playing
void AudioOpenSLPlayer::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{

    assert(NULL != context);
    assert(bq == ((AudioOpenSLPlayer*)context)->playerBufferQueue);
    AudioOpenSLPlayer* aOSL=((AudioOpenSLPlayer*)context);

    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (--aOSL->nextCount > 0 && NULL != aOSL->bufferProcessing && 0 != aOSL->nextSize) {
        SLresult result;
        // enqueue another buffer
        result = (*bq)->Enqueue(bq, aOSL->buffer1, aOSL->nextSize);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        assert(SL_RESULT_SUCCESS == result);
        (void)result;
    }
}





void AudioOpenSLPlayer::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
  SLresult result;
        //Pa_WriteStream (_stream, stream_in[0].ptr, stream_in[0].num);
    //register player callback
    result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, bqPlayerCallback,
            this);

}

void AudioOpenSLPlayer::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

  SLresult result;


    result = (*this->playerPlay)->SetPlayState(this->playerPlay, SL_PLAYSTATE_STOPPED);
    if (SL_RESULT_SUCCESS == result) {
            //recorderSize = AUDIO_RECORDER_FRAMES * sizeof(short);
            //playerSR = SL_SAMPLINGRATE_48;
    }

    if (playerObject != NULL) {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        this->playerPlay = NULL;
        playerBufferQueue = NULL;
    }


    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }


        //printf ("could not stop stream: %s", Pa_GetErrorText (status));


}


}
