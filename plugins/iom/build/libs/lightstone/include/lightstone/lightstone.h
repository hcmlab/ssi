/***
 * @file lightstone.h
 * @brief Implementation of lightstone communication
 * @author Kyle Machulis (kyle@nonpolynomial.com)
 * @copyright (c) 2007-2011 Nonpolynomial Labs/Kyle Machulis
 * @license BSD License
 *
 * Project info at http://liblightstone.nonpolynomial.com/
 *
 */

/**
@mainpage  liblightstone
@author kyle@nonpolynomial.com

Cross-platform driver for the Journey to Wild Divine Lightstone

Software website: http://liblightstone.nonpolynomial.com

@section liblightstoneIntro Introduction

liblightstone is an open source, cross platform library for reading data from the Journey to Wild Divine Lightstone biometrics widget.

@section liblightstoneBackground Background

Journey To Wild Divine is a video game that presents the player with the goal of relaxation. To chart how relaxed the player is, the game comes with a USB widget that records the Heart Rate Variance (HRV) via a PulseOx monitor, and Skin Conductance Level. These two metrics allows the software to know how well the player is taking in oxygen, their pulse rate, and their level of tension.

liblightstone was written to make this data available to any software. We built on the work of the developers behind the Lightstone Monitor software (http://sourceforge.net/projects/lsm/) to establish the protocol. LSM was originally written in C#, so we have ported the algorithms to C in order to make the software accessible to more platforms.

@section liblightstoneCaveats Lightstone Caveats

The lightstone is unfortunately not without its eccentricities.

- There are two versions of the lightstone that liblightstone knows about, with two completely different VID/PID pairs.
 - The blue lightstone has a VID/PID pair of {0x0483, 0x0035}
 - The white IOM device has a VID/PID pair of {0x14FA, 0x0001} (Blue devices of this pair have also been seen)
 - These are the only two pairs that liblightstone is aware of.
- The two versions put out completely different ranges for readings. Do not expect code with hardcoded ranges to work on both devices.
- Since the device registers as an HID device, any OS with an HID manager will pick it up and register it as such. On Linux and OS X, there is no way to access the device once this happens.
 - On Linux, liblightstone takes care of detaching the kernel driver from the device for the user. This can also be fixed permanently via udev rules.
 - On OS X, liblightstone requires that a null kext be in place in order to stop the kernel from picking up the device. This kext is distributed with the OS X library package for liblightstone, as well as in a standalone version.

@section liblightstoneHardwareInformation Hardware and Protocol Information

(This section is not relevant to using liblightstone, we simply lay out the protocol in case anyone else wants to implement their own version)

The lightstone has a STMicro ST7 chip that takes the readings from the sensors, converts them into text, and outputs them in ASCII form via HID reports. To computers, this will look like a USB HID device and will register as one also (see Lightstone Caveats section for more on this)

Since the device is working over the HID layer, the serial string comes in as a "Report". Reports usually consist of individual element updates from a controller (and only elements that have changed, to reduce bandwidth), but the lightstone uses them in a incorrect way. The protocol looks like:

- Always 8 Bytes per report according to descriptor (but we get back 9 for some reason). This MUST be read as a raw report, not as individual elements
- Byte 0 was ignored
- Byte 1 is the size of the useful part of the packet
- Bytes 2-8 are the actual data
- Messages end on 0x13 (ASCII newline), receiving that character is how we know when we've formed a complete message

The messages that the device sends looks like

- <RAW>AABB CCDD<\\RAW>
- <SER>XXXX<\\RAW>
- <VER>XXXX<\\RAW>

VER and SER messages identify the hardware, but are not useful otherwise. The RAW packet contains the HRV and SCL information, in the following format:

- HRV - ((AA << 8) | (BB)) * .01 (Returning a value between 1.6-2.5)
- SCR - ((CC << 8) | (DD)) * .001 (Returning a value between 3-15)

@section liblightstoneLicense License (Standard BSD License)

Copyright (c) 2007-2011, Kyle Machulis/Nonpolynomial Labs
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
- Neither the name of the Kyle Machulis/Nonpolynomial Labs nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY Kyle Machulis/Nonpolynomial Labs ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Kyle Machulis/Nonpolynomial Labs BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE

*/


/**
@defgroup CoreFunctions Core Functions

Core communications functions to open/close/read from the lightstone
*/

#ifndef LIBLIGHTSTONE_H
#define LIBLIGHTSTONE_H

#define E_LIGHTSTONE_DRIVER_ERROR -1
#define E_LIGHTSTONE_NOT_INITED -2
#define E_LIGHTSTONE_NOT_OPENED -3

#if defined(WIN32)
#if defined(LIGHTSTONE_DYNAMIC)
/// definition for when building DLLs. Doesn't need to be defined otherwise.
#define LIGHTSTONE_DECLSPEC __declspec(dllexport)
#else
/// definition for when building DLLs. Doesn't need to be defined otherwise.
#define LIGHTSTONE_DECLSPEC
#endif
#include <windows.h>

/**
 * Structure to hold information about Windows HID devices.
 *
 * Structure taken from libnputil.
 * @ingroup CoreFunctions
 */
typedef struct {
	/// Windows device handle
	HANDLE* _dev;
	/// 0 if device is closed, > 0 otherwise
	int _is_open;
	/// 0 if device is initialized, > 0 otherwise
	int _is_inited;
} lightstone;
#else //Non-Win32 platforms
#define LIGHTSTONE_DECLSPEC

#include "libusb-1.0/libusb.h"
/**
 * Structure to hold information about libusb-1.0 devices.
 *
 * Structure taken from libnputil.
 */
typedef struct {
	/// Library context for libusb-1.0
	struct libusb_context* _context;
	/// Specific device context for libusb-1.0
	struct libusb_device_handle* _device;
	/// In transfer object for asynchronous libusb-1.0 transfers
	struct libusb_transfer* _in_transfer;
	/// Out transfer object for asynchronous libusb-1.0 transfers
	struct libusb_transfer* _out_transfer;
	/// 0 if device is closed, > 0 otherwise
	int _is_open;
	/// 0 if device is not initialized, > 0 otherwise
	int _is_inited;
} lightstone;
#endif

/// Number of VID/PID pairs that we know about
#define LIGHTSTONE_VID_PID_PAIRS_COUNT 2
/// VID/PID pairs for devices
static const unsigned int lightstone_vid_pid_pairs[2][2] = { {0x0483, 0x0035}, {0x14FA, 0x0001} };

/// In endpoint for all omron health devices
static const unsigned int LIGHTSTONE_IN_ENDPT  = 0x81;

/// Lightstone information structure
typedef struct
{
	/**
	 * Heart Rate Variability measurement
	 *
	 */
	float hrv;
	/**
	 * Skin Conductance Level measurement
	 *
	 */
	float scl;
} lightstone_info;

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * Create a lightstone device structure and return it. Must be
	 * called for each lightstone device to be used.
	 *
	 *
	 * @ingroup CoreFunctions
	 * @return Initialized device structure
	 */
	LIGHTSTONE_DECLSPEC lightstone* lightstone_create();

	/**
	 * Destroy a lightstone device. Will also close the device if it
	 * is currently open.
	 *
	 * @ingroup CoreFunctions
	 * @param dev Device structure to destroy
	 */
	LIGHTSTONE_DECLSPEC void lightstone_delete(lightstone* dev);

	/**
	 * Count the number of lightstones on the system, of all known
	 * VIDs and PIDs for the version of liblightstone
	 *
	 * @param dev Initialized device structure (does not have to be
	 * opened)
	 *
	 * @ingroup CoreFunctions
	 * @return Number of lightstones connected to the computer
	 */
	LIGHTSTONE_DECLSPEC int lightstone_get_count(lightstone* dev);

	/**
	 * Open a lightstone device of an VID/PID known to liblightstone
	 *
	 * @param dev Initialized device structure
	 * @param device_index Index of device to open on bus, starts at 0
	 *
	 * @ingroup CoreFunctions
	 * @return 0 if device opened successfully, non-zero otherwise
	 */
	LIGHTSTONE_DECLSPEC int lightstone_open(lightstone* dev, unsigned int device_index);

	/**
	 * Close an opened device
	 *
	 * @param dev Initialized and opened device to close.
	 * @ingroup CoreFunctions
	 */
	LIGHTSTONE_DECLSPEC int lightstone_close(lightstone* dev);

	/**
	 * Test whether the current device is initialized
	 *
	 * @param dev Device to check
	 *
	 * @ingroup CoreFunctions
	 * @return > 0 if device is initialized, 0 otherwise
	 */
	LIGHTSTONE_DECLSPEC int lightstone_valid(lightstone* dev);

	/**
	 * Internal function used to read via platform specific
	 * functions. Not meant for use outside of library.
	 *
	 * @param dev Initialized and opened device
	 * @param report Report buffer to read into
	 * @param report_length Lenght of report to read
	 *
	 * @ingroup CoreFunctions
	 * @return
	 */
	int lightstone_read(lightstone* dev, unsigned char *report);

	/**
	 * Retreive a single HRV/SCL pair from the device. Blocks until
	 * pair is retrived.
	 *
	 * @param dev Initialized and opened device
	 *
	 * @ingroup CoreFunctions
	 * @return Structure with latest HRV/SCL reading
	 */
	LIGHTSTONE_DECLSPEC lightstone_info lightstone_get_info(lightstone* dev);

#ifdef __cplusplus
}
#endif

#endif //LIBLIGHTSTONE_H
