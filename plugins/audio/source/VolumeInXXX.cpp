// VolumeInXXX.cpp : Module interface implementation.
// Developer : Alex Chmut
// Created : 8/11/98
#include "VolumeInXXX.h"

/////////////////////////////////////////////////////////////////////////////
// 		Defines
#define	BAD_DWORD	(DWORD)-1
#define	WND_CLASS_NAME	"Input Volume Msg Wnd Class"
#define	WND_NAME		"Input Volume Msg Wnd"

/////////////////////////////////////////////////////////////////////////////
// 		Globals
PCVolumeInXXX g_pThis = NULL;

////////////////////////////////////////////////////////////

//{{{ Audio specific functions
#define AUDFREQ			22050	// Frequency
#define AUDCHANNELS		1		// Number of channels
#define AUDBITSMPL		16		// Number of bits per sample
inline
void SetDeviceType( WAVEFORMATEX* pwfe )
{
	memset( pwfe, 0, sizeof(WAVEFORMATEX) );
	WORD  nBlockAlign = (AUDCHANNELS*AUDBITSMPL)/8;
	DWORD nSamplesPerSec = AUDFREQ;
	pwfe->wFormatTag = WAVE_FORMAT_PCM;
	pwfe->nChannels = AUDCHANNELS;
	pwfe->nBlockAlign = nBlockAlign;
	pwfe->nSamplesPerSec = nSamplesPerSec;
	pwfe->wBitsPerSample = AUDBITSMPL;
	pwfe->nAvgBytesPerSec = nSamplesPerSec*nBlockAlign;
}
//}}} Audio specific functions

/////////////////////////////////////////////////////////////////////////////
// 		Implementation
//////////////
CVolumeInXXX::CVolumeInXXX( UINT uLineIndex )
	:	m_bOK(false),
		m_bInitialized(false),
		m_bAvailable(false),

		m_uMixerID(0L),
		m_dwMixerHandle(0L),
		m_hWnd(NULL),

		m_uMicrophoneSourceLineIndex(BAD_DWORD),

		m_dwMinimalVolume(BAD_DWORD),
		m_dwMaximalVolume(BAD_DWORD),

		m_pfUserSink(NULL),
		m_dwUserValue(0L)
{
	if ( m_bOK = Init() )
	{
		g_pThis = this;
		if ( !Initialize( uLineIndex ) )
		{
			Done();
			g_pThis = NULL;
		}
	}
}
//////////////
CVolumeInXXX::~CVolumeInXXX()
{
	if ( m_bOK )
		Done();
	g_pThis = NULL;
}
//////////////
bool CVolumeInXXX::Init()
{
	if ( !mixerGetNumDevs() )
		return false;
	// Getting Mixer ID
	HWAVEIN hwaveIn;
	MMRESULT mmResult;
	WAVEFORMATEX WaveFmt;
	SetDeviceType( &WaveFmt );
	mmResult = waveInOpen( &hwaveIn, WAVE_MAPPER, &WaveFmt, 0L, 0L, CALLBACK_NULL );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not open WaveIn Mapper. mmResult=%d", mmResult );
		return false;
	} else {
		mmResult = mixerGetID( (HMIXEROBJ)hwaveIn, &m_uMixerID, MIXER_OBJECTF_HWAVEIN );
		waveInClose( hwaveIn );
		if ( mmResult != MMSYSERR_NOERROR )
		{
			ssi_wrn (".InputXxxVolume: FAILURE: WaveIn Mapper in Mixer is not available. mmResult=%d", mmResult );
			return false;
		}
	}
	// Exposing Window to Mixer
	WNDCLASSEX wcx;
	memset( &wcx, 0, sizeof(WNDCLASSEX) );	
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.lpszClassName = WND_CLASS_NAME;
	wcx.lpfnWndProc = (WNDPROC)MixerWndProc;
	::RegisterClassEx(&wcx);
	m_hWnd = CreateWindow(	WND_CLASS_NAME,
							WND_NAME,
							WS_POPUP | WS_DISABLED,
							0, 0, 0, 0,
							NULL, NULL, NULL, NULL );
	if ( !m_hWnd )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not create internal window" );
		return false;
	}
	::ShowWindow(m_hWnd, SW_HIDE);
	mmResult = mixerOpen( (LPHMIXER)&m_dwMixerHandle, m_uMixerID, (DWORD)m_hWnd, 0L, CALLBACK_WINDOW );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not open Mixer. mmResult=%d", mmResult );
		::DestroyWindow( m_hWnd );
		return false;
	}
	return true;
}
//////////////
void CVolumeInXXX::Done()
{
	if ( mixerClose( (HMIXER)m_dwMixerHandle ) != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: WARNING: Could not close Mixer" );
	}
	::DestroyWindow( m_hWnd );
	m_bInitialized = false;
	m_bOK = false;
}
//////////////
void CVolumeInXXX::OnControlChanged( DWORD dwControlID )
{
	if ( m_dwVolumeControlID == dwControlID )
	{
		DWORD dwVolume = GetCurrentVolume();
		if ( (dwVolume!=BAD_DWORD) && (m_pfUserSink) )
		{
			(*m_pfUserSink)( dwVolume, m_dwUserValue );
		}
	}
}
//////////////
bool CVolumeInXXX::Initialize( UINT uLineIndex )
{
	MMRESULT mmResult;
	if ( !m_bOK )
		return false;
	ssi_msg (SSI_LOG_LEVEL_DETAIL, ".InputXxxVolume: Initializing for the Source Line (%d) ..", uLineIndex );
	MIXERLINE MixerLine;
	memset( &MixerLine, 0, sizeof(MIXERLINE) );
	MixerLine.cbStruct = sizeof(MIXERLINE);
	MixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	mmResult = mixerGetLineInfo( (HMIXEROBJ)m_dwMixerHandle, &MixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not get WaveIn Destionation Line for the requested source while initilaizing. mmResult=%d", mmResult );
		return false;
	}

	MIXERCONTROL Control;
	memset( &Control, 0, sizeof(MIXERCONTROL) );
	Control.cbStruct = sizeof(MIXERCONTROL);

	MIXERLINECONTROLS LineControls;
	memset( &LineControls, 0, sizeof(MIXERLINECONTROLS) );
	LineControls.cbStruct = sizeof(MIXERLINECONTROLS);

	MIXERLINE Line;
	memset( &Line, 0, sizeof(MIXERLINE) );
	Line.cbStruct = sizeof(MIXERLINE);

	if ( ( uLineIndex < MixerLine.cConnections ) )
	{
		Line.dwDestination = MixerLine.dwDestination;
		Line.dwSource = uLineIndex;
		mmResult = mixerGetLineInfo( (HMIXEROBJ)m_dwMixerHandle, &Line, MIXER_GETLINEINFOF_SOURCE );
		if ( mmResult != MMSYSERR_NOERROR )
		{
			ssi_wrn (".InputXxxVolume: FAILURE: Could not get the requested Source Line while initilaizing. mmResult=%d", mmResult );
			return false;
		}
		ssi_msg (SSI_LOG_LEVEL_DETAIL, ".InputXxxVolume: \"%s\" Source Line adopted", Line.szShortName );
		LineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		LineControls.dwLineID = Line.dwLineID;
		LineControls.cControls = 1;
		LineControls.cbmxctrl = sizeof(MIXERCONTROL);
		LineControls.pamxctrl = &Control;
		mmResult = mixerGetLineControls( (HMIXEROBJ)m_dwMixerHandle, &LineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE );
		if ( mmResult == MMSYSERR_NOERROR )
		{
			if ( !(Control.fdwControl & MIXERCONTROL_CONTROLF_DISABLED) )
			{
				m_bAvailable = true;
				ssi_msg (SSI_LOG_LEVEL_DETAIL, ".InputXxxVolume: \"%s\" Volume control for the Source Line adopted", Control.szShortName );
			} else {
				ssi_wrn (".InputXxxVolume: WARNING: The Volume Control is disabled" );
			}
		} else {
			ssi_wrn (".InputXxxVolume: WARNING: Could not get the requested Source Line Volume Control for the requested line while initilaizing. mmResult=%d", mmResult );
		}		
	} else {
		ssi_wrn (".InputXxxVolume: FAILURE: Invalid Source Line index passed" );
		return false;
	}

	// Retrieving Microphone Source Line
	for ( UINT uLine = 0; uLine < MixerLine.cConnections; uLine++ )
	{
		MIXERLINE MicrophoneLine;
		memset( &MicrophoneLine, 0, sizeof(MIXERLINE) );
		MicrophoneLine.cbStruct = sizeof(MIXERLINE);
		MicrophoneLine.dwDestination = MixerLine.dwDestination;
		MicrophoneLine.dwSource = uLine;
		mmResult = mixerGetLineInfo( (HMIXEROBJ)m_dwMixerHandle, &MicrophoneLine, MIXER_GETLINEINFOF_SOURCE );
		if ( mmResult == MMSYSERR_NOERROR )
		{
			if ( MicrophoneLine.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE )
			{
				m_uMicrophoneSourceLineIndex = MicrophoneLine.dwSource;
				ssi_msg (SSI_LOG_LEVEL_DETAIL, ".InputXxxVolume: Microphone Source Line \"%s\" has been found", MicrophoneLine.szShortName );
				break;
			}
		}
	}
	if ( m_uMicrophoneSourceLineIndex == BAD_DWORD )
	{
		ssi_wrn (".InputXxxVolume: WARNING: Could not retrieve Microphone Source Line" );
	}

	m_uSourceLineIndex = uLineIndex;
	m_nChannelCount = Line.cChannels;
	m_dwLineID = LineControls.dwLineID;
	m_dwVolumeControlID = Control.dwControlID;
	m_dwMinimalVolume = Control.Bounds.dwMinimum;
	m_dwMaximalVolume = Control.Bounds.dwMaximum;
	m_dwVolumeStep = Control.Metrics.cSteps;

	m_bInitialized = true;
	return true;
}
//////////////////////////////////////////////
bool CVolumeInXXX::GetMicrophoneSourceLineIndex( UINT* puLineIndex )
{
	if ( !puLineIndex || !m_bInitialized || (m_uMicrophoneSourceLineIndex==BAD_DWORD) )
		return false;
	*puLineIndex = m_uMicrophoneSourceLineIndex;
	return true;
}
//////////////////////////////////////////////
// IVolume interface
//////////////
bool CVolumeInXXX::IsAvailable()
{
	return m_bAvailable;
}
//////////////
void CVolumeInXXX::Enable()
{
	if ( !m_bInitialized )
		return;
	bool bAnyEnabled = false;
	MMRESULT mmResult;

	MIXERLINE lineDestination;
	memset( &lineDestination, 0, sizeof(MIXERLINE) );
	lineDestination.cbStruct = sizeof(MIXERLINE);
	lineDestination.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	mmResult = mixerGetLineInfo( (HMIXEROBJ)m_dwMixerHandle, &lineDestination, MIXER_GETLINEINFOF_COMPONENTTYPE );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not get the Destination Line while enabling. mmResult=%d", mmResult );
		return;
	}
	// Getting all line's controls
	int nControlCount = lineDestination.cControls;
	int nChannelCount = lineDestination.cChannels;
	MIXERLINECONTROLS LineControls;
	memset( &LineControls, 0, sizeof(MIXERLINECONTROLS) );
	MIXERCONTROL* aControls = (MIXERCONTROL*)malloc( nControlCount * sizeof(MIXERCONTROL) );
	if ( !aControls )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Out of memory while enabling the line" );
		return;
	}
	memset( &aControls[0], 0, sizeof(nControlCount * sizeof(MIXERCONTROL)) );
	for ( int i = 0; i < nControlCount; i++ )
	{
		aControls[i].cbStruct = sizeof(MIXERCONTROL);
	}
	LineControls.cbStruct = sizeof(MIXERLINECONTROLS);
	LineControls.dwLineID = lineDestination.dwLineID;
	LineControls.cControls = nControlCount;
	LineControls.cbmxctrl = sizeof(MIXERCONTROL);
	LineControls.pamxctrl = &aControls[0];
	mmResult = mixerGetLineControls( (HMIXEROBJ)m_dwMixerHandle, &LineControls, MIXER_GETLINECONTROLSF_ALL );
	if ( mmResult == MMSYSERR_NOERROR )
	{
		for (int i = 0; i < nControlCount; i++ )
		{
			if ( aControls[i].dwControlType & MIXERCONTROL_CT_UNITS_BOOLEAN )
			{
				MIXERCONTROLDETAILS_BOOLEAN* aDetails = NULL;
				int nMultipleItems = aControls[i].cMultipleItems;
				int nChannels = nChannelCount;
				// MIXERCONTROLDETAILS
				MIXERCONTROLDETAILS ControlDetails;
				memset( &ControlDetails, 0, sizeof(MIXERCONTROLDETAILS) );
				ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
				ControlDetails.dwControlID = aControls[i].dwControlID;
				if ( aControls[i].fdwControl & MIXERCONTROL_CONTROLF_UNIFORM )
				{
					nChannels = 1;
				}
				if ( aControls[i].fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE )
				{
					nMultipleItems = aControls[i].cMultipleItems;
					aDetails = (MIXERCONTROLDETAILS_BOOLEAN*)malloc(nMultipleItems*nChannels*sizeof(MIXERCONTROLDETAILS_BOOLEAN));
					if ( !aDetails )
					{
						ssi_wrn (".InputXxxVolume: FAILURE: Out of memory while enabling the line" );
						continue;
					}
					for ( int nItem = 0; nItem < nMultipleItems; nItem++ )
					{
						LONG lValue = FALSE;
						if ( nItem == (int)m_uSourceLineIndex )
							lValue = TRUE;
						for ( int nChannel = 0; nChannel < nChannels; nChannel++ )
						{
							aDetails[nItem+nChannel].fValue = lValue;
						}
					}
				} else {
					nMultipleItems = 0;
					aDetails = (MIXERCONTROLDETAILS_BOOLEAN*)malloc(nChannels*sizeof(MIXERCONTROLDETAILS_BOOLEAN));
					if ( !aDetails )
					{
						ssi_wrn (".InputXxxVolume: FAILURE: Out of memory while enabling the line" );
						continue;
					}
					for ( int nChannel = 0; nChannel < nChannels; nChannel++ )
					{
						aDetails[nChannel].fValue = (LONG)TRUE;
					}
				}
				ControlDetails.cChannels = nChannels;
				ControlDetails.cMultipleItems = nMultipleItems;
				ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
				ControlDetails.paDetails = &aDetails[0];
				mmResult = mixerSetControlDetails( (HMIXEROBJ)m_dwMixerHandle, &ControlDetails, 0L );
				if ( mmResult == MMSYSERR_NOERROR )
				{
					ssi_msg (SSI_LOG_LEVEL_DETAIL, ".InputXxxVolume: Enabling Line: Line control \"%s\" has been enabled", aControls[i].szShortName );
					bAnyEnabled = true;
				}
				free( aDetails );
			}
		}
	} else {
		ssi_wrn (".InputXxxVolume: FAILURE: Could not get the line's controls while enabling. mmResult=%d", mmResult );
	}
	free( aControls );
	if ( !bAnyEnabled )
	{
		ssi_wrn (".InputXxxVolume: WARNING: No controls were found for enabling the line" );
	}
}
//////////////
void CVolumeInXXX::Disable()
{
	ssi_wrn (".InputXxxVolume: WARNING: Disable line has no sense. The function not implemented" );
}
//////////////
DWORD CVolumeInXXX::GetVolumeMetric()
{
	if ( !m_bAvailable )
		return BAD_DWORD;
	return m_dwVolumeStep;
}
//////////////
DWORD CVolumeInXXX::GetMinimalVolume()
{
	if ( !m_bAvailable )
		return BAD_DWORD;
	return m_dwMinimalVolume;
}
//////////////
DWORD CVolumeInXXX::GetMaximalVolume()
{
	if ( !m_bAvailable )
		return BAD_DWORD;
	return m_dwMaximalVolume;
}
//////////////
DWORD CVolumeInXXX::GetCurrentVolume()
{
	if ( !m_bAvailable )
		return BAD_DWORD;
	MIXERCONTROLDETAILS_UNSIGNED* aDetails = (MIXERCONTROLDETAILS_UNSIGNED*)malloc(m_nChannelCount*sizeof(MIXERCONTROLDETAILS_UNSIGNED));
	if ( !aDetails )
		return BAD_DWORD;
	MIXERCONTROLDETAILS ControlDetails;
	memset( &ControlDetails, 0, sizeof(MIXERCONTROLDETAILS) );
	ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	ControlDetails.dwControlID = m_dwVolumeControlID;
	ControlDetails.cChannels = m_nChannelCount;
	ControlDetails.cMultipleItems = 0;
	ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	ControlDetails.paDetails = &aDetails[0];
	MMRESULT mmResult = mixerGetControlDetails( (HMIXEROBJ)m_dwMixerHandle, &ControlDetails, MIXER_GETCONTROLDETAILSF_VALUE );
	DWORD dw = aDetails[0].dwValue;
	free( aDetails );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not get volume. mmResult=%d", mmResult );
		return BAD_DWORD;
	}
	return dw;
}
//////////////
void CVolumeInXXX::SetCurrentVolume( DWORD dwValue )
{
	if ( !m_bAvailable || (dwValue<m_dwMinimalVolume) || (dwValue>m_dwMaximalVolume) )
		return;
	MIXERCONTROLDETAILS_UNSIGNED* aDetails = (MIXERCONTROLDETAILS_UNSIGNED*)malloc(m_nChannelCount*sizeof(MIXERCONTROLDETAILS_UNSIGNED));
	if ( !aDetails )
		return;
	for ( int i = 0; i < m_nChannelCount; i++ )
	{
		aDetails[i].dwValue = dwValue;
	}
	MIXERCONTROLDETAILS ControlDetails;
	memset( &ControlDetails, 0, sizeof(MIXERCONTROLDETAILS) );
	ControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	ControlDetails.dwControlID = m_dwVolumeControlID;
	ControlDetails.cChannels = m_nChannelCount;
	ControlDetails.cMultipleItems = 0;
	ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	ControlDetails.paDetails = &aDetails[0];
	MMRESULT mmResult = mixerSetControlDetails( (HMIXEROBJ)m_dwMixerHandle, &ControlDetails, MIXER_SETCONTROLDETAILSF_VALUE );
	free( aDetails );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not set volume(%d) mmResult=%d", dwValue, mmResult );
	}
}
//////////////
void CVolumeInXXX::RegisterNotificationSink( PONMICVOULUMECHANGE pfUserSink, DWORD dwUserValue )
{
	m_pfUserSink = pfUserSink;
	m_dwUserValue = dwUserValue;
}
////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CVolumeInXXX::MixerWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == MM_MIXM_CONTROL_CHANGE )
	{
		if ( g_pThis )
		{
			g_pThis->OnControlChanged( (DWORD)lParam );
		}
	}
	return ::DefWindowProc( hwnd, uMsg, wParam, lParam);
}
////////////////////////////////////////////////////////////////////////
bool CVolumeInXXX::EnumerateInputLines( PINPUTLINEPROC pUserCallback, DWORD dwUserValue )
{
	if ( !pUserCallback )
		return false;
	MMRESULT mmResult;
	HWAVEIN hwaveIn;
	WAVEFORMATEX WaveFmt;
	SetDeviceType( &WaveFmt );
	mmResult = waveInOpen( &hwaveIn, WAVE_MAPPER, &WaveFmt, 0L, 0L, CALLBACK_NULL );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: Could not open WaveIn Mapper. mmResult=%d", mmResult );
		return false;
	}
	UINT uMixerID;
	DWORD dwMixerHandle;
	mmResult = mixerGetID( (HMIXEROBJ)hwaveIn, &uMixerID, MIXER_OBJECTF_HWAVEIN );
	waveInClose( hwaveIn );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		ssi_wrn (".InputXxxVolume: FAILURE: WaveIn Mapper in Mixer is not available. mmResult=%d", mmResult );
		return false;
	}
	mmResult = mixerOpen( (LPHMIXER)&dwMixerHandle, uMixerID, 0L, 0L, 0L );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		mixerClose( (HMIXER)dwMixerHandle );
		ssi_wrn (".InputXxxVolume: FAILURE: Could not open Mixer. mmResult=%d", mmResult );
		return false;
	}
	MIXERLINE MixerLine;
	memset( &MixerLine, 0, sizeof(MIXERLINE) );
	MixerLine.cbStruct = sizeof(MIXERLINE);
	MixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	mmResult = mixerGetLineInfo( (HMIXEROBJ)dwMixerHandle, &MixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE );
	if ( mmResult != MMSYSERR_NOERROR )
	{
		mixerClose( (HMIXER)dwMixerHandle );
		ssi_wrn (".InputXxxVolume: FAILURE: Could not get WaveIn Destionation Line for the requested source while enumerating. mmResult=%d", mmResult );
		return false;
	}
	MIXERLINE Line;
	for ( UINT uLineIndex = 0; uLineIndex < MixerLine.cConnections; uLineIndex++ )
	{
		memset( &Line, 0, sizeof(MIXERLINE) );
		Line.cbStruct = sizeof(MIXERLINE);
		Line.dwDestination = MixerLine.dwDestination;
		Line.dwSource = uLineIndex;
		mmResult = mixerGetLineInfo( (HMIXEROBJ)dwMixerHandle, &Line, MIXER_GETLINEINFOF_SOURCE );
		if ( mmResult != MMSYSERR_NOERROR )
		{
			mixerClose( (HMIXER)dwMixerHandle );
			ssi_wrn (".InputXxxVolume: FAILURE: Could not get the interated Source Line while enumerating. mmResult=%d", mmResult );
			return false;
		}
		if ( !((*pUserCallback)( uLineIndex, &Line, dwUserValue )) )
		{
			break;
		}
	}
	mixerClose( (HMIXER)dwMixerHandle );
	return true;
}
