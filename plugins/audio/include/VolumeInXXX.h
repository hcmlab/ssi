// VolumeInXXX.h : Module interface declaration.
// IVolume implementation for current audio device
// Developer : Alex Chmut
// Created : 8/11/98
#pragma once
#include "IVolume.h"

// User-defined callback for input lines enumeration. Return 'false' to stop enumeration.
// No construction of CVolumeInXXX objects within the callback.
typedef bool (CALLBACK *PINPUTLINEPROC)
	( UINT uLineIndex, MIXERLINE* pLineInfo, DWORD dwUserValue );

///////////////////////////////////////////////////////////////////////////////////////////////
class CVolumeInXXX
	: public IVolume
{
public:
	static	bool	EnumerateInputLines( PINPUTLINEPROC, DWORD dwUserValue );
	bool	GetMicrophoneSourceLineIndex( UINT* puLineIndex );

////////////////////////
// IVolume interface
public:
	virtual bool	IsAvailable();
	virtual void	Enable();
	virtual void	Disable();
	virtual DWORD	GetVolumeMetric();
	virtual DWORD	GetMinimalVolume();
	virtual DWORD	GetMaximalVolume();
	virtual DWORD	GetCurrentVolume();
	virtual void	SetCurrentVolume( DWORD dwValue );
	virtual void	RegisterNotificationSink( PONMICVOULUMECHANGE, DWORD );

public:
	CVolumeInXXX( UINT uLineIndex );
	~CVolumeInXXX();

private:
	bool	Init();
	void	Done();

	bool	Initialize( UINT uLineIndex );

private:
	// Status Info
	bool	m_bOK;
	bool	m_bInitialized;
	bool	m_bAvailable;

	// Mixer Info
	UINT	m_uMixerID;
	DWORD	m_dwMixerHandle;

	DWORD	m_dwLineID;
	DWORD	m_dwVolumeControlID;
	int		m_nChannelCount;
	UINT	m_uSourceLineIndex;

	UINT	m_uMicrophoneSourceLineIndex;
	
	HWND	m_hWnd;
	static	LRESULT CALLBACK MixerWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void	OnControlChanged( DWORD dwControlID );

	DWORD	m_dwMinimalVolume;
	DWORD	m_dwMaximalVolume;
	DWORD	m_dwVolumeStep;

	// User Info
	PONMICVOULUMECHANGE		m_pfUserSink;
	DWORD					m_dwUserValue;
};

typedef	CVolumeInXXX*	PCVolumeInXXX;
///////////////////////////////////////////////////////////////////////////////////////////////