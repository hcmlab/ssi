// AudioOut.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
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
//
// following a tutorial by David Overton (http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=4422&lngWId=3)

#include "AudioOut.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

AudioOut::AudioOut() : _currentBuffer(0), _freeBufferCounter(0)
{
}

AudioOut::WaveBuffer* AudioOut::getFreeBuffer() {		

	//check if there is still room in current buffer
	if(!_waveBuffers[_currentBuffer].locked || _waveBuffers[_currentBuffer].usedBytes < _bufferSize)
	{
		if(!_waveBuffers[_currentBuffer].locked)
		{
			EnterCriticalSection(&_waveCriticalSection); 
			_waveBuffers[_currentBuffer].locked = true;
			_waveBuffers[_currentBuffer].usedBytes = 0;
			_freeBufferCounter--; 
			LeaveCriticalSection(&_waveCriticalSection);
		}

		return &_waveBuffers[_currentBuffer];
	}

	//if not, we need a new buffer
	//check if there are free buffers
	while(_freeBufferCounter <= 0)
	{
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "wave buffer capacity exceeded. You may need more buffer for playback");
		_freeBuffer.wait();
	} 

	_currentBuffer++;
	if(_currentBuffer >= _numBuffers) _currentBuffer = 0;	
	
    EnterCriticalSection(&_waveCriticalSection); 
	_freeBufferCounter--; 
	_waveBuffers[_currentBuffer].locked = true;
	_waveBuffers[_currentBuffer].usedBytes = 0;
    LeaveCriticalSection(&_waveCriticalSection);
	
	return &_waveBuffers[_currentBuffer];
}

void AudioOut::freeBuffer(ssi_size_t bufferID) {	
 
    EnterCriticalSection(&_waveCriticalSection); 	
	_waveBuffers[bufferID].locked = false;
	_freeBufferCounter++; 
    LeaveCriticalSection(&_waveCriticalSection); 

	if(_freeBufferCounter == 1)
		_freeBuffer.release();
}

HWAVEOUT AudioOut::init (int device_id, WAVEFORMATEX wfx, ssi_size_t bufferSizeInBytes, ssi_size_t numBuffers) {

	HWAVEOUT hWaveOut;

	InitializeCriticalSection(&_waveCriticalSection);

	_bufferSize = bufferSizeInBytes;
	_numBuffers = numBuffers;
    allocateBuffers (bufferSizeInBytes, numBuffers); 
	_freeBufferCounter = numBuffers;
 
    /* 
     * try to open the default wave device. WAVE_MAPPER is 
     * a constant defined in mmsystem.h, it always points to the 
     * default wave device on the system (some people have 2 or 
     * more sound cards). 
     */ 
    if(waveOutOpen( 
        &hWaveOut,  
        device_id,  
        &wfx,  
        (DWORD_PTR)waveOutProc,  
        (DWORD_PTR)this,  
        CALLBACK_FUNCTION 
    ) != MMSYSERR_NOERROR) { 
        ssi_err ("Unable to open wave mapper device"); 
    }

	return hWaveOut;
}

void AudioOut::clean (HWAVEOUT hWaveOut) {

	/*
	 * check whether last buffer is still active
	 */
	if(_waveBuffers[_currentBuffer].locked || _waveBuffers[_currentBuffer].usedBytes < _bufferSize)
		freeBuffer(_currentBuffer);

	/* 
     * wait for all buffers to complete 
     */ 
	while (_freeBufferCounter < ssi_cast (int, _numBuffers)) 
        Sleep(10); 
 
    /* 
     * unprepare any buffers that are still prepared 
     */ 
	for(int i = 0; i < _freeBufferCounter; i++)  
        if(_waveBuffers[i].header.dwFlags & WHDR_PREPARED) 
            waveOutUnprepareHeader(hWaveOut, &_waveBuffers[i].header, sizeof(WAVEHDR));      

	_freeBuffer.release();	
    LeaveCriticalSection(&_waveCriticalSection); 
    DeleteCriticalSection (&_waveCriticalSection); 

    freeBuffers(); 
    waveOutClose(hWaveOut); 
}

void AudioOut::writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size) 
{ 
    WaveBuffer* current; 
    int remain; 
	MMRESULT result;
  
    while(size > 0) { 

		current = getFreeBuffer();

        /*  
         * first make sure the header we're going to use is unprepared 
         */ 
        if(current->header.dwFlags & WHDR_PREPARED)  
		{
			result = waveOutUnprepareHeader(hWaveOut, &current->header, sizeof(WAVEHDR)); 
			checkResult("waveOutUnprepareHeader", result);
		}
 
        if(size < (int)(_bufferSize - current->usedBytes)) { 
            memcpy(current->header.lpData + current->usedBytes, data, size); 
            current->usedBytes += size; 
            break; 
        } 
 
        remain = _bufferSize - current->usedBytes; 
        memcpy(current->header.lpData + current->usedBytes, data, remain); 
        size -= remain; 
        data += remain; 
        current->usedBytes = _bufferSize; 
        
		result = waveOutPrepareHeader(hWaveOut, &current->header, sizeof(WAVEHDR));
        checkResult("waveOutPrepareHeader", result);

		result = waveOutWrite(hWaveOut, &current->header, sizeof(WAVEHDR));
		checkResult("hWaveOut", result);

		//updateCurrentBuffer();
    } 
} 
 
void AudioOut::allocateBuffers(int size, int count) 
{ 
    unsigned char* buffer; 
    DWORD totalBufferSize = (size + sizeof(WaveBuffer)) * count; 
     
    /* 
     * allocate memory for the entire set in one go 
     */ 
    if((buffer = ssi_pcast (unsigned char, HeapAlloc( 
        GetProcessHeap(),  
        HEAP_ZERO_MEMORY,  
        totalBufferSize 
    ))) == NULL) { 
        ssi_err ("Memory allocation error");       
    } 
 
    /* 
     * and set up the pointers to each bit 
     */ 
    _waveBuffers = (WaveBuffer*)buffer; 
    buffer += sizeof(WaveBuffer) * count; 

    for(int i = 0; i < count; i++)
	{ 
		_waveBuffers[i].id = i;
		_waveBuffers[i].usedBytes = 0;
		_waveBuffers[i].locked = false;
        _waveBuffers[i].header.dwBufferLength = size; 
		_waveBuffers[i].header.lpData = ssi_pcast (char, buffer); 
		_waveBuffers[i].header.dwUser = i;
        buffer += size; 
    } 
} 
 
void AudioOut::freeBuffers() 
{ 
    /*  
     * and this is why allocateBuffers works the way it does 
     */  
    HeapFree(GetProcessHeap(), 0, _waveBuffers); 
} 
 
void CALLBACK AudioOut::waveOutProc( 
    HWAVEOUT hWaveOut,  
    UINT uMsg,  
    DWORD_PTR dwInstance,   
    DWORD_PTR dwParam1,     
    DWORD_PTR dwParam2      
) 
{ 
	AudioOut* audio = (AudioOut*)dwInstance; 
    /* 
     * ignore calls that occur due to openining and closing the 
     * device. 
     */ 
    if(uMsg != WOM_DONE) 
        return; 

	audio->freeBuffer( ((WAVEHDR*)dwParam1)->dwUser );
}

bool AudioOut::checkResult (const ssi_char_t *func, MMRESULT result) {
	
	if(result == MMSYSERR_NOERROR)
		return true;

	char buf[164];
	::waveInGetErrorText (result, reinterpret_cast<LPSTR> (buf), sizeof (buf));
	ssi_wrn ("%s() : %s", func, buf);
	return false;
}

}
