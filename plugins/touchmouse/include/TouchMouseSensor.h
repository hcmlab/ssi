/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#pragma once

#ifdef TOUCHMOUSESENSOR_EXPORTS
#define TOUCHMOUSESENSOR_API __declspec(dllexport)
#else
#define TOUCHMOUSESENSOR_API __declspec(dllimport)
#pragma comment(lib, "TouchMouseSensor.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Mouse event structure detailing the state of the mouse.
struct TOUCHMOUSESTATUS
{
    // Unique identifier for mouse. If more than one mouse is present they 
    // will each report a different identifier. When a mouse disconnects 
    // its identifier may be reused when another connection is made.
    LONG64 m_dwID;

    // Mouse disconnected indicator. Normally false, the last report from 
    // a mouse connection will set this true.
    BOOL m_fDisconnect;

    // The elapsed time in milliseconds since the previous report. This 
    // time is derived from a clock on the mouse itself and should be 
    // considered more accurate than any other source of timing for the
    // report.
    //
    // The elapsed time is only good for short time spans. The delta is 
    // reported as zero if more than about 100ms of time has elapsed.
    DWORD m_dwTimeDelta;

    // Width of the image.
    //
    // Current mice all report 15. If pabImage contains an image, 
    // pabImage[0] is the top-left pixel, pabImage[1] the pixel to the 
    // right of pabImage[0] and pabImage[m_dwImageWidth] the pixel below
    // the pabImage[0].
    DWORD m_dwImageWidth;

    // Height of the image.
    //
    // Current mice all report 13.
    DWORD m_dwImageHeight;
};

// Callback type for accepting TOUCHMOUSESTATUS and sensor image.
typedef void (*TouchMouseCallback)(const TOUCHMOUSESTATUS* const pTouchMouseStatus, 
                                   const BYTE* const pabImage, 
                                   DWORD dwImageSize);

// Sets the callback function to be used.
void TOUCHMOUSESENSOR_API RegisterTouchMouseCallback(TouchMouseCallback callback);

// Resets the callback function.
//
// Calls to the previous function are not guaranteed to stop immediately.
void TOUCHMOUSESENSOR_API UnregisterTouchMouseCallback();

#ifdef __cplusplus
}
#endif
