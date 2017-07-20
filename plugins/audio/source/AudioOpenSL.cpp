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


#include "AudioOpenSL.h"

#include "base/IProvider.h"
#include "thread/Lock.h"
#include "base/StringList.h"
#include "WavProvider.h"

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



ssi_char_t *AudioOpenSL::ssi_log_name = "audioandri";

AudioOpenSL::AudioOpenSL(const ssi_char_t* file)
	: _file(NULL), 
	_dataProvider (0)
{

    recorderBuffer1=0;
    recorderBuffer2=0;
    recorderBufferProcessing=0;

	if(file)
	{
		if(!OptionList::LoadXML(file, &_options))
			OptionList::SaveXML(file, &_options);
		_file = ssi_strcpy(file);
	}
}

AudioOpenSL::~AudioOpenSL()
{
	if(_file)
	{
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
        if(_options.scale)
            delete[] scaledBuffer;

        delete[] recorderBuffer1;
        delete[] recorderBuffer2;
         delete[] recorderBufferProcessing;

}

bool AudioOpenSL::setProvider(const ssi_char_t *name, ssi::IProvider *provider)
{
	SSI_ASSERT(provider);

	if (_dataProvider) {
		ssi_wrn ("already set");
		return false;
	}

    if (strcmp (name, SSI_AUDIOOPENSL_PROVIDER_NAME) != 0) {
		ssi_wrn ("unkown provider name %s", name);
		return false;
	}



	if (Parse (_options.channels)) {
		
		//generate offset values


		
	} else {
		ssi_wrn ("could not parse channel list");
	}
        if(_options.scale)
            _audio_channel.set_scaled(true);

	_dataProvider = provider;
	_audio_channel.stream.sr = _options.sr;
        _audio_channel.stream.dim = 1;
	_dataProvider->init (&_audio_channel);

	return true;
}

bool AudioOpenSL::createEngine()
{
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
}

bool AudioOpenSL::createAudioRecorder()
{
    SLresult result;

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
            SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};

    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorderObject, &audioSrc,
            &audioSnk, 1, id, req);
    if (SL_RESULT_SUCCESS != result) {
        return false;
    }

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return false;
    }

    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
            &recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, SlStreamCallback,
           this);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    return true;
}

bool AudioOpenSL::connect()
{
	ssi_msg(SSI_LOG_LEVEL_DETAIL, "try to connect sensor...");


        _frame_size = _options.block;

        recorderBuffer1=new short int[_options.block];
        recorderBuffer2=new short int[_options.block];

        if(_options.scale)
            scaledBuffer=new float[_options.block];
        else
            scaledBuffer=0;

        recorderBufferProcessing=new short int[_options.block];

        if(!createEngine())
        {
            //
        }

        //start recording stream
        if(!createAudioRecorder())
        {

        }


          //      ssi_wrn("Could not open stream:");

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");
	
	if(_dataProvider && ssi_log_level >= SSI_LOG_LEVEL_DETAIL)
	{
		ssi_print ("\
             device\t= %s\n\
             rate\t= %.2lf\n\
             block\t= %d\n\
             dim\t= %d\n\
             bytes\t= %d\n",
			 _options.device,
                         _options.sr,
			 _options.block,
                         sizeof(short));
	}

	return true;
}

bool AudioOpenSL::start () {

        SLresult result;
        bool bStart=true;

        // in case already recording, stop recording and clear buffer queue
        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;
        result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;

        // the buffer is not valid for playback yet
        int recorderSize = 0;

        // enqueue an empty buffer to be filled by the recorder
        // (for streaming recording, we would enqueue at least 2 empty buffers to start things off)

        result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recorderBuffer1,
                _frame_size * sizeof(short));
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which would indicate a programming error
        assert(SL_RESULT_SUCCESS == result);
        (void)result;

        // start recording
        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;



        /*if()
	{
                ssi_wrn("Could not start stream:");

		return false;
        }*/

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "start streaming");

	return true;
}

bool AudioOpenSL::stop () {

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "stop streaming");



        // destroy audio recorder object, and invalidate all associated interfaces
        if (recorderObject != NULL) {
            (*recorderObject)->Destroy(recorderObject);
            recorderObject = NULL;
            recorderRecord = NULL;
            recorderBufferQueue = NULL;
        }


        // destroy engine object, and invalidate all associated interfaces
        if (engineObject != NULL) {
            (*engineObject)->Destroy(engineObject);
            engineObject = NULL;
            engineEngine = NULL;
        }


    //ssi_wrn("Disconnect failed: %s", Pa_GetErrorText(status));




	return true;
}

bool AudioOpenSL::disconnect()
{		

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "disconnect");



	return true;
}

bool AudioOpenSL::supportsReconnect()
{
	return false;
}

void AudioOpenSL::VSlStreamCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{

    if(_options.scale)
    {
        for(int i=0; i < _options.block; i++)
                    scaledBuffer[i]=recorderBufferProcessing[i]/32768.0f;
        _dataProvider->provide(ssi_pcast(ssi_byte_t, scaledBuffer), _options.block);
    }
    else
        _dataProvider->provide(ssi_pcast(ssi_byte_t, recorderBufferProcessing), _options.block);


}

void AudioOpenSL::SlStreamCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
        //PAudioOpenSLData pData = ssi_pcast(AudioOpenSLData, userData);
        //return pData->Instance->VPaStreamCallback(input, output, frameCount, timeInfo, statusFlags, userData);

    AudioOpenSL* manager =(AudioOpenSL*)context;

    short *recorderBuffer;
    if (manager->bufferOneFilled) {
            recorderBuffer = manager->recorderBuffer1;
    }
    else {
            recorderBuffer = manager->recorderBuffer2;
    }

    //TODO prevent copying buffer without locking?
    memcpy ( (manager->recorderBufferProcessing), recorderBuffer, sizeof(short)*manager->_frame_size );



    //toggle between two buffers so that one can be filled while the other is processed
    // for streaming recording, we call Enqueue to give recorder the next buffer to fill
    if (manager->bufferOneFilled) {
        SLresult result = (*manager->recorderBufferQueue)->Enqueue(manager->recorderBufferQueue, manager->recorderBuffer2, manager->_frame_size * sizeof(short));
        manager->bufferOneFilled = false;
    }
    else {
        SLresult result = (*manager->recorderBufferQueue)->Enqueue(manager->recorderBufferQueue, manager->recorderBuffer1, manager->_frame_size * sizeof(short));
        manager->bufferOneFilled = true;
    }

    manager->VSlStreamCallback(bq, context);

 }



bool AudioOpenSL::Parse (const ssi_char_t *indices) {
	
	if (!indices || indices[0] == '\0') {
		return false;
	}

	ssi_char_t string[SSI_MAX_CHAR];
	
	char *pch;
	strcpy (string, indices);
	pch = strtok (string, ", ");
	int index;

	std::vector<int> items;
	
	while (pch != NULL) {
		index = atoi (pch);
		items.push_back(index);
		pch = strtok (NULL, ", ");
	}



	return true;
}


}
