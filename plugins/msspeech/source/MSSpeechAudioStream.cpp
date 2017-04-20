// MSSpeechAudioStream.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/10/10
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

#include "MSSpeechAudioStream.h"
#include <stdio.h>

MSSpeechAudioStream::MSSpeechAudioStream(ULONG n_buffer) :
    m_cRef(1),
    _n_read (0),
	_buffer (0),
	_n_buffer (n_buffer),
	_read_pos (0),
	_write_pos (0),
	_n_available (0),
	_is_capturing (false),
    m_hDataReady(NULL)
{

	_buffer = new BYTE[_n_buffer];	

    InitializeCriticalSection(&m_Lock);

    m_hDataReady = CreateEvent( NULL, FALSE, FALSE, NULL );    
}

MSSpeechAudioStream::~MSSpeechAudioStream()
{
	if (NULL != m_hDataReady)
    {
        SetEvent(m_hDataReady);
        CloseHandle(m_hDataReady);
        m_hDataReady = NULL;
    }

	// Why Comment This ?
    //DeleteCriticalSection(&m_Lock);

	delete[] _buffer; _buffer = 0;
	_read_pos = 0;
	_write_pos = 0;
	_n_available = 0;
	_n_read = 0;
}

STDMETHODIMP MSSpeechAudioStream::Read (void *buffer, ULONG n_buffer, ULONG *n_read)
{
   if (n_read == NULL)
   {
       return E_INVALIDARG;
   }

    ULONG bytesPendingToRead = n_buffer;
    while (bytesPendingToRead > 0 && _is_capturing)
    {
		EnterCriticalSection(&m_Lock);
		int n_available = _n_available;
		LeaveCriticalSection (&m_Lock);        

        if (n_available == 0) //no data, wait ...
        {
            WaitForSingleObject(m_hDataReady, INFINITE);
			if (!_is_capturing) {
				*n_read = 0;
				return S_OK;
			}
        }

		Read ((BYTE**)&buffer, &bytesPendingToRead);
    }

    ULONG bytesRead = n_buffer - bytesPendingToRead;
    _n_read += bytesRead;
    *n_read = bytesRead;

    return S_OK;
}

STDMETHODIMP MSSpeechAudioStream::Write(const void *,ULONG,ULONG *)
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition )   
{
    if (plibNewPosition != NULL)
    {
        plibNewPosition->QuadPart = _n_read + dlibMove.QuadPart;
    }
    return S_OK;
}

STDMETHODIMP MSSpeechAudioStream::SetSize(ULARGE_INTEGER)
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::CopyTo(IStream *,ULARGE_INTEGER,ULARGE_INTEGER *,ULARGE_INTEGER *)
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::Commit(DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::UnlockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::Stat(STATSTG *,DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP MSSpeechAudioStream::Clone(IStream **)
{
    return E_NOTIMPL;
}

void MSSpeechAudioStream::Stop () {
	_is_capturing = false;
	SetEvent(m_hDataReady);
}

void MSSpeechAudioStream::Write (BYTE *data, UINT n_data)
{
	_is_capturing = true;

	EnterCriticalSection(&m_Lock);

	UINT _n_left = _n_buffer - _write_pos;

	if (n_data < _n_left) {
		memcpy (_buffer + _write_pos, data, n_data);
		_write_pos += n_data;		
	} else {		
		UINT _n_over = n_data - _n_left;
		memcpy (_buffer + _write_pos, data, _n_left);
		memcpy (_buffer, data + _n_left, _n_over);
		_write_pos = _n_over;
	}
	_n_available += n_data;
    SetEvent(m_hDataReady);

	LeaveCriticalSection(&m_Lock);
}

void MSSpeechAudioStream::Read (BYTE **data, ULONG* n_data)
{
    EnterCriticalSection(&m_Lock);

    //Copy as much data as we can or need        
    ULONG n_copy = min (_n_available, *n_data);

	UINT _n_left = _n_buffer - _read_pos; 
	if (n_copy < _n_left) {
		memcpy (*data, _buffer + _read_pos, n_copy);
		_read_pos += n_copy;    
	} else {
		UINT _n_over = n_copy - _n_left;
		memcpy (*data, _buffer + _read_pos, _n_left);
		memcpy ((*data) + _n_left, _buffer, _n_over);
		_read_pos = _n_over;
	}

    *data = (*data)+n_copy;
    *n_data = (*n_data)-n_copy;
	_n_available -= n_copy;

    LeaveCriticalSection(&m_Lock);
}


