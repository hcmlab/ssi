/**
@copyright COPYRIGHT 2019 - PROPERTY OF TOBII PRO AB
@copyright 2019 TOBII PRO AB - KARLSROVAGEN 2D, DANDERYD 182 53, SWEDEN - All Rights Reserved.

@copyright NOTICE:  All information contained herein is, and remains, the property of Tobii Pro AB and its suppliers, if any.  The intellectual and technical concepts contained herein are proprietary to Tobii Pro AB and its suppliers and may be covered by U.S.and Foreign Patents, patent applications, and are protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is strictly forbidden unless prior written permission is obtained from Tobii Pro AB.
*/

/**
 * @file tobii_research.h
 * @brief <b>Generic SDK functions.</b>
 *
 */

#ifndef TOBII_RESEARCH_H_
#define TOBII_RESEARCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#if _WIN32 || _WIN64
#ifdef TOBII_STATIC_LIB
#define TOBII_RESEARCH_CALL
#define TOBII_RESEARCH_API
#else
#define TOBII_RESEARCH_CALL __cdecl
#ifdef TOBII_EXPORTING
#define TOBII_RESEARCH_API __declspec(dllexport)
#else
#define TOBII_RESEARCH_API __declspec(dllimport)
#endif /* TOBII_EXPORTING */
#endif /* TOBII_STATIC_LIB */
#else
#define TOBII_RESEARCH_API
#define TOBII_RESEARCH_CALL
#endif /* _WIN32 */


/**
Status codes returned by the SDK.
 */
typedef enum {
    /**
    No error.
    */
    TOBII_RESEARCH_STATUS_OK,

    /**
    Fatal error. This should normally not happen.
    */
    TOBII_RESEARCH_STATUS_FATAL_ERROR,

    /**
    Failed to initialize the API. This is a fatal error.
    */
    TOBII_RESEARCH_STATUS_INITIALIZE_FAILED,

    /**
    Failed to terminate the API.
    */
    TOBII_RESEARCH_STATUS_TERMINATE_FAILED,

    /**
    Failed to create browser for finding local devices.
    */
    TOBII_RESEARCH_STATUS_LOCALBROWSER_CREATE_FAILED,

    /**
    Failed to poll local devices.
    */
    TOBII_RESEARCH_STATUS_LOCALBROWSER_POLL_FAILED,

    /**
    Failed to create zero configuration browser.
    */
    TOBII_RESEARCH_STATUS_ZEROCONFBROWSER_CREATE_FAILED,

    /**
    Failed to poll devices from zero configuration browser.
    */
    TOBII_RESEARCH_STATUS_ZEROCONFBROWSER_POLL_FAILED,

    /**
    Failed to create browser that looks for devices in file.
    */
    TOBII_RESEARCH_STATUS_FILEBROWSER_CREATE_FAILED,

    /**
    Failed to poll devices from file browser.
    */
    TOBII_RESEARCH_STATUS_FILEBROWSER_POLL_FAILED,

    /**
    An invalid parameter was given to the method.
    */
    TOBII_RESEARCH_STATUS_INVALID_PARAMETER,

    /**
    The operation was invalid.
    */
    TOBII_RESEARCH_STATUS_INVALID_OPERATION,

    /**
    Internal core error code. Should never be returned by the SDK.
    */
    TOBII_RESEARCH_STATUS_UNINITIALIZED,

    /**
    A parameter is out of bounds.
    */
    TOBII_RESEARCH_STATUS_OUT_OF_BOUNDS,

    /**
    The display area is not valid. Please configure the eye tracker.
    */
    TOBII_RESEARCH_STATUS_DISPLAY_AREA_NOT_VALID,

    /**
    The buffer is too small.
    */
    TOBII_RESEARCH_STATUS_BUFFER_TOO_SMALL,

    /**
    tobii_research_initialize has not been called.
    */
    TOBII_RESEARCH_STATUS_NOT_INITIALIZED,

    /**
    tobii_research_initialize has already been called.
    */
    TOBII_RESEARCH_STATUS_ALREADY_INITIALIZED,

    /**
    The license saved on the device failed to apply when connecting. It has probably expired.
    */
    TOBII_RESEARCH_STATUS_SAVED_LICENSE_FAILED_TO_APPLY,

    /**
    Internal stream engine error.
    */
    TOBII_RESEARCH_STATUS_SE_INTERNAL = 200,

    /**
    The operation requires a higher license type.
    */
    TOBII_RESEARCH_STATUS_SE_INSUFFICIENT_LICENSE,

    /**
    The operations isn't supported in the current context.
    */
    TOBII_RESEARCH_STATUS_SE_NOT_SUPPORTED,

    /**
    The device is unavailable.
    */
    TOBII_RESEARCH_STATUS_SE_NOT_AVAILABLE,

    /**
    Connection to the device failed.
    */
    TOBII_RESEARCH_STATUS_SE_CONNECTION_FAILED,

    /**
    The operation timed out.
    */
    TOBII_RESEARCH_STATUS_SE_TIMED_OUT,

    /**
    Failed to allocate memory.
    */
    TOBII_RESEARCH_STATUS_SE_ALLOCATION_FAILED,

    /**
    The API is already initialized.
    */
    TOBII_RESEARCH_STATUS_SE_ALREADY_INITIALIZED,

    /**
    The API isn't initialized.
    */
    TOBII_RESEARCH_STATUS_SE_NOT_INITIALIZED,

    /**
    An invalid parameter was given to the method.
    */
    TOBII_RESEARCH_STATUS_SE_INVALID_PARAMETER,

    /**
    Calibration has already started.
    */
    TOBII_RESEARCH_STATUS_SE_CALIBRATION_ALREADY_STARTED,

    /**
    Calibration isn't started.
    */
    TOBII_RESEARCH_STATUS_SE_CALIBRATION_NOT_STARTED,

    /**
    Already subscribed.
    */
    TOBII_RESEARCH_STATUS_SE_ALREADY_SUBSCRIBED,

    /**
    Not subscribed.
    */
    TOBII_RESEARCH_STATUS_SE_NOT_SUBSCRIBED,

    /**
    Operation failed.
    */
    TOBII_RESEARCH_STATUS_SE_OPERATION_FAILED,

    /**
    Conflicting api instances.
    */
    TOBII_RESEARCH_STATUS_SE_CONFLICTING_API_INSTANCES,

    /**
    Calibration busy.
    */
    TOBII_RESEARCH_STATUS_SE_CALIBRATION_BUSY,

    /**
    Callback in progress.
    */
    TOBII_RESEARCH_STATUS_SE_CALLBACK_IN_PROGRESS,

    /**
    Too many users subscribed to a stream.
    */
    TOBII_RESEARCH_STATUS_SE_TOO_MANY_SUBSCRIBERS,

    /**
    The buffer is too small.
    */
    TOBII_RESEARCH_STATUS_SE_BUFFER_TOO_SMALL,

    /**
    No response from firmware.
    */
    TOBII_RESEARCH_STATUS_SE_FIRMWARE_NO_RESPONSE,

    /**
    Internal error.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_INTERNAL = 400,

    /**
    Firmware upgrade is not supported.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_NOT_SUPPORTED,

    /**
    Unknown firmware version.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_UNKNOWN_FIRMWARE_VERSION,

    /**
    Connection failed.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_CONNECTION_FAILED,

    /**
    Invalid parameter.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_INVALID_PARAMETER,

    /**
    Device mismatch. The firmware package is not meant for the device.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_PACKAGE_DEVICE_MISMATCH,

    /**
    Parse response.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_PARSE_RESPONSE,

    /**
    The firmware upgrade operation failed.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_OPERATION_FAILED,

    /**
    Memory allocation failed during firmware upgrade.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_ALLOCATION_FAILED,

    /**
    The firmware failed to respond during firmware upgrade.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_FIRMWARE_NO_RESPONSE,

    /**
    The firmware downgrade operation is not supported.
    */
    TOBII_RESEARCH_STATUS_FWUPGRADE_DOWNGRADE_NOT_SUPPORTED,

    /**
    Unknown error. This is a fatal error.
    */
    TOBII_RESEARCH_STATUS_UNKNOWN = 1000
} TobiiResearchStatus;

/**
Opaque representation of an eye tracker struct.
*/
typedef struct TobiiResearchEyeTracker TobiiResearchEyeTracker;

/**
Contains all eye trackers connected to the computer or the network.
*/
typedef struct {
    /**
    An array of pointers to eye trackers.
    */
    TobiiResearchEyeTracker** eyetrackers;
    /**
    Number of eye tracker pointers in the array.
    */
    size_t count;
} TobiiResearchEyeTrackers;

/**
Source of log message.
*/
typedef enum {
    /**
    The log message is from stream engine.
    */
    TOBII_RESEARCH_LOG_SOURCE_STREAM_ENGINE,

    /**
    The log message is from the SDK.
    */
    TOBII_RESEARCH_LOG_SOURCE_SDK,

    /**
    The log message is from the firmware upgrade module.
    */
    TOBII_RESEARCH_LOG_SOURCE_FIRMWARE_UPGRADE
} TobiiResearchLogSource;

/**
Log level.
*/
typedef enum {
    /**
    Error message.
    */
    TOBII_RESEARCH_LOG_LEVEL_ERROR,

    /**
    Warning message.
    */
    TOBII_RESEARCH_LOG_LEVEL_WARNING,

    /**
    Information message.
    */
    TOBII_RESEARCH_LOG_LEVEL_INFORMATION,

    /**
    Debug message.
    */
    TOBII_RESEARCH_LOG_LEVEL_DEBUG,

    /**
    Trace message.
    */
    TOBII_RESEARCH_LOG_LEVEL_TRACE
} TobiiResearchLogLevel;

/**
Represents a normalized x- and y-coordinate point in a two-dimensional plane.
 */
typedef struct {
    /**
    Position of the point in the X axis.
    */
    float x;
    /**
    Position of the point in the Y axis.
    */
    float y;
} TobiiResearchNormalizedPoint2D;


/**
Represents an x-, y- and z-coordinate point in a three-dimensional space.
 */
typedef struct {
    /**
    Position of the point in the X axis.
    */
    float x;
    /**
    Position of the point in the Y axis.
    */
    float y;
    /**
    Position of the point in the Z axis.
    */
    float z;
} TobiiResearchPoint3D;

/**
Represents a normalized x-, y- and z-coordinate point in a three-dimensional space.
 */
typedef TobiiResearchPoint3D TobiiResearchNormalizedPoint3D;

/**
@brief Log callback.

Implement this and send as a parameter to @ref tobii_research_logging_subscribe.
@param system_time_stamp: The time stamp according to the computer's internal clock.
@param source: Source of log message.
@param level: Log message level.
@param message: The log message.
*/
typedef void(*tobii_research_log_callback)(int64_t system_time_stamp,
                TobiiResearchLogSource source,
                TobiiResearchLogLevel level,
                const char* message);

/**
@brief Subscribes to logging.

@param callback: Callback that will receive log messages.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_logging_subscribe(
                        tobii_research_log_callback callback);

/**
@brief Unsubscribes from logging.

@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_logging_unsubscribe();

/**
@brief Finds eye trackers connected to the computer or the network.

The eye trackers can be used in any tobii_research function that requires an eye tracker.

\snippet find_all_eyetrackers.c FindAllEyetrackers

@param eyetrackers: Pointers to found eye trackers will be stored in this struct.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_find_all_eyetrackers(
                        TobiiResearchEyeTrackers** eyetrackers);

/**
@brief Free memory allocation for the result received via @ref tobii_research_find_all_eyetrackers.

@param eyetrackers: Eye trackers to free.
*/
TOBII_RESEARCH_API void TOBII_RESEARCH_CALL tobii_research_free_eyetrackers(
                        TobiiResearchEyeTrackers* eyetrackers);

/**
@brief Gets data for an eye tracker given an address.

\snippet create_eyetracker.c Example

@param address: Address of eye tracker to get data for.
@param eyetracker: Eye tracker object returned.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_eyetracker(
                        const char* address,
                        TobiiResearchEyeTracker** eyetracker);

/**
@brief Retrieves the time stamp from the system clock in microseconds.

\snippet get_system_time_stamp.c Example

@param time_stamp_us: The time stamp of the system in microseconds.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_system_time_stamp(int64_t* time_stamp_us);

/**
@brief Free memory allocation for a string allocated by the SDK.

@param str: String to free.
*/
TOBII_RESEARCH_API void TOBII_RESEARCH_CALL tobii_research_free_string(char* str);


/**
SDK Version.
*/
typedef struct {
    /**
    Major.
    */
    int major;
    /**
    Minor.
    */
    int minor;
    /**
    Revision.
    */
    int revision;
    /**
    Build.
    */
    int build;
} TobiiResearchSDKVersion;

/**
Gets the SDK version.

@param sdk_version: Version of the SDK.
@returns A TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_sdk_version(
                        TobiiResearchSDKVersion* sdk_version);

#ifdef __cplusplus
}
#endif
#endif /* TOBII_RESEARCH_H_ */
