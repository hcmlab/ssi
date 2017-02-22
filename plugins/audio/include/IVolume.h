// IVolume.h : IVolume interface definition.
// Developer : Alex Chmut
// Created : 8/11/98
#pragma once

#include <Windows.h>
#include <cstdio>
#include <MMSystem.h>

#include "SSI_Cons.h"

// User-defined callback for volume change notification
typedef void (CALLBACK *PONMICVOULUMECHANGE)( DWORD dwCurrentVolume, DWORD dwUserValue );

////////////////////////////////////////////////////////////////////////
// IVolume interface
class __declspec(novtable) IVolume
{
public:
	virtual bool	IsAvailable() = 0;
	virtual void	Enable() = 0;
	virtual void	Disable() = 0;

	virtual DWORD	GetVolumeMetric() = 0;

	virtual DWORD	GetMinimalVolume() = 0;
	virtual DWORD	GetMaximalVolume() = 0;

	virtual DWORD	GetCurrentVolume() = 0;
	virtual void	SetCurrentVolume( DWORD dwValue ) = 0;

	virtual void	RegisterNotificationSink( PONMICVOULUMECHANGE, DWORD ) = 0;
};

