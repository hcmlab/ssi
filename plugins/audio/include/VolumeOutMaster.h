// VolumeOutMaster.h : Module interface declaration.
// IVolume implementation for master audio volume
// Developer : Alex Chmut
// Created : 8/11/98
#pragma once
#include "IVolume.h"

///////////////////////////////////////////////////////////////////////////////////////////////
class CVolumeOutMaster
	: public IVolume
{

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
	CVolumeOutMaster();
	~CVolumeOutMaster();

private:
	bool	Init();
	void	Done();

	bool	Initialize();
	void	EnableLine( bool bEnable = true );

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

typedef	CVolumeOutMaster*	PCVolumeOutMaster;
///////////////////////////////////////////////////////////////////////////////////////////////

