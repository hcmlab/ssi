/**
@copyright COPYRIGHT 2019 - PROPERTY OF TOBII PRO AB
@copyright 2019 TOBII PRO AB - KARLSROVAGEN 2D, DANDERYD 182 53, SWEDEN - All Rights Reserved.

@copyright NOTICE:  All information contained herein is, and remains, the property of Tobii Pro AB and its suppliers, if any.  The intellectual and technical concepts contained herein are proprietary to Tobii Pro AB and its suppliers and may be covered by U.S.and Foreign Patents, patent applications, and are protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is strictly forbidden unless prior written permission is obtained from Tobii Pro AB.
*/

/**
* @file tobii_research_streams.h
* @brief <b>Functionality for eye tracker streams.</b>
*
*/

#ifndef TOBII_RESEARCH_STREAMS_H_
#define TOBII_RESEARCH_STREAMS_H_

#include "tobii_research.h"
#include "tobii_research_eyetracker.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Specifies the validity.
*/
typedef enum {
    /**
    Indicates invalid.
    */
    TOBII_RESEARCH_VALIDITY_INVALID,

    /**
    Indicates valid.
    */
    TOBII_RESEARCH_VALIDITY_VALID
} TobiiResearchValidity;

/**
Provides properties for the gaze origin.
*/
typedef struct {
    /**
    The gaze origin position in 3D in the user coordinate system.
    */
    TobiiResearchPoint3D position_in_user_coordinates;

    /**
    The normalized gaze origin position in 3D in the track box coordinate system.
    */
    TobiiResearchNormalizedPoint3D position_in_track_box_coordinates;

    /**
     The validity of the gaze origin data.
    */
    TobiiResearchValidity validity;
} TobiiResearchGazeOrigin;

/**
Provides properties for the pupil data.
*/
typedef struct {
    /**
    The diameter of the pupil in millimeters.
    */
    float diameter;

    /**
    The validity of the pupil data.
    */
    TobiiResearchValidity validity;
} TobiiResearchPupilData;

/**
Provides properties for the gaze point.
*/
typedef struct {
    /**
    The gaze point position in 2D on the active display area.
    */
    TobiiResearchNormalizedPoint2D position_on_display_area;

    /**
    The gaze point position in 3D in the user coordinate system.
    */
    TobiiResearchPoint3D position_in_user_coordinates;

    /**
    The validity of the gaze point data.
    */
    TobiiResearchValidity validity;
} TobiiResearchGazePoint;

/**
Provides properties for the eye data.
*/
typedef struct {
    /**
    The gaze point data.
    */
    TobiiResearchGazePoint gaze_point;

    /**
    The pupil data.
    */
    TobiiResearchPupilData pupil_data;

    /**
    The gaze origin data.
    */
    TobiiResearchGazeOrigin gaze_origin;
} TobiiResearchEyeData;

/**
Provides data for the @ref tobii_research_gaze_data_callback callback.
*/
typedef struct {
    /**
    The gaze data for the left eye.
    */
    TobiiResearchEyeData left_eye;

    /**
    The gaze data for the right eye.
    */
    TobiiResearchEyeData right_eye;

    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t device_time_stamp;

    /**
    The time stamp according to the computer's internal clock.
    */
    int64_t system_time_stamp;
} TobiiResearchGazeData;


/**
Provides properties eye user position guide.
*/
typedef struct {
    /**
    The (normalized) 3D coordinates that describes the user's position, (0.5, 0.5, 0.5) is the ideal position".
    */
    TobiiResearchNormalizedPoint3D user_position;

    /**
    The validity of the user postion guide.
    */
    TobiiResearchValidity validity;
} TobiiResearchEyeUserPositionGuide;


/**
Provides data for the @ref tobii_research_user_position_guide_callback callback.
*/
typedef struct {
    /**
    The user position guide for the left eye.
    */
    TobiiResearchEyeUserPositionGuide left_eye;

    /**
    The user position guide for the right eye.
    */
    TobiiResearchEyeUserPositionGuide right_eye;
} TobiiResearchUserPositionGuide;


/**
Provides properties for the HMD pupil position.
*/
typedef struct {
    /**
    The (normalized) 2D coordinates that describes the pupil's position in the HMD's tracking area.
    */
    TobiiResearchNormalizedPoint2D position_in_tracking_area;

    /**
    The validity of the pupil position data.
    */
    TobiiResearchValidity validity;
} TobiiResearchHMDPupilPosition;

/**
Provides properties for the HMD gaze origin.
*/
typedef struct {
    /**
    The 3D coordinates that describes the gaze origin in (in mm).
    */
    TobiiResearchPoint3D position_in_hmd_coordinates;

    /**
    The validity of the gaze origin data.
    */
    TobiiResearchValidity validity;
} TobiiResearchHMDGazeOrigin;

/**
Provides properties for the HMD gaze direction.
*/
typedef struct {
    /**
    The 3D unit vector that describes the gaze direction.
    */
    TobiiResearchNormalizedPoint3D unit_vector;

    /**
    The validity of the gaze direction data.
    */
    TobiiResearchValidity validity;
} TobiiResearchHMDGazeDirection;

/**
Provides properties for the eye data when gotten from an HMD based device.
*/
typedef struct {
    /**
    The gaze direction data.
    */
    TobiiResearchHMDGazeDirection gaze_direction;

    /**
    The pupil data.
    */
    TobiiResearchPupilData pupil_data;

    /**
    The gaze origin data.
    */
    TobiiResearchHMDGazeOrigin gaze_origin;

    /**
    The pupil position in HMD track box.
    */
    TobiiResearchHMDPupilPosition pupil_position;
} TobiiResearchHMDEyeData;

/**
Provides data for the @ref tobii_research_hmd_gaze_data_callback callback.
*/
typedef struct {
    /**
    The gaze data for the left eye.
    */
    TobiiResearchHMDEyeData left_eye;

    /**
    The gaze data for the right eye.
    */
    TobiiResearchHMDEyeData right_eye;

    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t device_time_stamp;

    /**
    The time stamp according to the computer's internal clock.
    */
    int64_t system_time_stamp;
} TobiiResearchHMDGazeData;

/**
Provides data for the @ref tobii_research_time_synchronization_data_callback callback.
*/
typedef struct {
    /**
    The time stamp when the computer sent the request to the eye tracker.
    */
    int64_t system_request_time_stamp;

    /**
    The time stamp when the eye tracker received the request, according to the eye tracker's clock.
    */
    int64_t device_time_stamp;

    /**
    The time stamp when the computer received the response from the eye tracker.
    */
    int64_t system_response_time_stamp;
} TobiiResearchTimeSynchronizationData;

/**
Defines external signal change type.
*/
typedef enum {
    /**
    Indicates that the value sent to the eye tracker has changed.
    */
    TOBII_RESEARCH_EXTERNAL_SIGNAL_VALUE_CHANGED,

    /**
    Indicates that the value is the initial value, and is received when starting a subscription.
    */
    TOBII_RESEARCH_EXTERNAL_SIGNAL_INITIAL_VALUE,

    /**
    Indicates that there has been a connection lost and now it is restored and the value is the current value.
    */
    TOBII_RESEARCH_EXTERNAL_SIGNAL_CONNECTION_RESTORED
} TobiiResearchExternalSignalChangeType;

/**
Provides data for the @ref tobii_research_external_signal_data_callback callback.
*/
typedef struct {
    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t device_time_stamp;

    /**
    The time stamp according to the computer's internal clock.
    */
    int64_t system_time_stamp;

    /**
    The value of the external signal port on the eye tracker.
    */
    uint32_t value;

    TobiiResearchExternalSignalChangeType change_type;
} TobiiResearchExternalSignalData;

/**
Defines error types
*/
typedef enum {
    /**
    Indicates that the connection to the eye tracker was lost.
    */
    TOBII_RESEARCH_STREAM_ERROR_CONNECTION_LOST,

    /**
    Indicates that the license is insufficient for subscribing to the stream.
    */
    TOBII_RESEARCH_STREAM_ERROR_INSUFFICIENT_LICENSE,

    /**
    Indicates that the stream isn't supported by the eye tracker.
    */
    TOBII_RESEARCH_STREAM_ERROR_NOT_SUPPORTED,

    /**
    Indicates that number of subscriptions to the stream has reached its limit.
    */
    TOBII_RESEARCH_STREAM_ERROR_TOO_MANY_SUBSCRIBERS,

    /**
    Indicates that an internal error occurred.
    */
    TOBII_RESEARCH_STREAM_ERROR_INTERNAL_ERROR,

    /**
    Indicates that the user threw an exception in the callback.
    */
    TOBII_RESEARCH_STREAM_ERROR_USER_ERROR
} TobiiResearchStreamError;

/**
Defines error sources
*/
typedef enum {
    /**
    User callback failed.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_USER,

    /**
    Error when pumping event.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_STREAM_PUMP,

    /**
    Error when subscribing to event for gaze data.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_SUBSCRIPTION_GAZE_DATA,

    /**
    Error when subscribing to event for external signal.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_SUBSCRIPTION_EXTERNAL_SIGNAL,

    /**
    Error when subscribing to event for time synchronization data.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_SUBSCRIPTION_TIME_SYNCHRONIZATION_DATA,

    /**
    Error when subscribing to event for eye images.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_SUBSCRIPTION_EYE_IMAGE,

    /**
    Error when subscribing to notification event.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_SUBSCRIPTION_NOTIFICATION,

    /**
    Error when subscribing to event for hmd gaze data.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_SUBSCRIPTION_HMD_GAZE_DATA,

    /**
    Error when subscribing to event for user position guide.
    */
    TOBII_RESEARCH_STREAM_ERROR_SOURCE_SUBSCRIPTION_USER_POSITION_GUIDE
} TobiiResearchStreamErrorSource;

/**
Defines notification types
*/
typedef enum {
    /**
    Indicates that the connection to the eye tracker is lost.
    */
    TOBII_RESEARCH_NOTIFICATION_CONNECTION_LOST,

    /**
    Indicates that the connection to the eye tracker is restored.
    */
    TOBII_RESEARCH_NOTIFICATION_CONNECTION_RESTORED,

    /**
    Indicates that the calibration mode is entered.
    */
    TOBII_RESEARCH_NOTIFICATION_CALIBRATION_MODE_ENTERED,

    /**
    Indicates that the calibration mode is left.
    */
    TOBII_RESEARCH_NOTIFICATION_CALIBRATION_MODE_LEFT,

    /**
    Indicates that the calibration is changed.
    */
    TOBII_RESEARCH_NOTIFICATION_CALIBRATION_CHANGED,

    /**
    Indicates that the track box is changed.
    */
    TOBII_RESEARCH_NOTIFICATION_TRACK_BOX_CHANGED,

    /**
    Indicates that the display area is changed.
    */
    TOBII_RESEARCH_NOTIFICATION_DISPLAY_AREA_CHANGED,

    /**
    Indicates that the gaze output frequency is changed.
    */
    TOBII_RESEARCH_NOTIFICATION_GAZE_OUTPUT_FREQUENCY_CHANGED,

    /**
    Indicates that the eye tracking mode is changed.
    */
    TOBII_RESEARCH_NOTIFICATION_EYE_TRACKING_MODE_CHANGED,

    /**
    Indicates that the device has reported new faults.
    */
    TOBII_RESEARCH_NOTIFICATION_DEVICE_FAULTS,

    /**
    Indicates that the device has reported new warnings.
    */
    TOBII_RESEARCH_NOTIFICATION_DEVICE_WARNINGS,

    /**
    */
    TOBII_RESEARCH_NOTIFICATION_UNKNOWN
} TobiiResearchNotificationType;


typedef char TobiiResearchNotificationString[512];

/**
Provides data for the @ref tobii_research_notification_callback callback.
*/
typedef struct {
    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t system_time_stamp;
    /**
    The notification type.
    */
    TobiiResearchNotificationType notification_type;
    union {
        /**
        The new output frequency if notification of type @ref TOBII_RESEARCH_NOTIFICATION_GAZE_OUTPUT_FREQUENCY_CHANGED
        */
        float output_frequency;

        /**
        The new display area if notification of type @ref TOBII_RESEARCH_NOTIFICATION_DISPLAY_AREA_CHANGED
        */
        TobiiResearchDisplayArea display_area;

        /** The new faults if notification of type @ref TOBII_RESEARCH_NOTIFICATION_DEVICE_FAULTS.
        The new warnings if notification of type @ref TOBII_RESEARCH_NOTIFICATION_DEVICE_WARNINGS.
        Contains a comma separated list of warnings or faults.
        */
        TobiiResearchNotificationString text;
    } value;
} TobiiResearchNotification;

/**
@brief Gaze data callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_gaze_data.
@param gaze_data: Gaze data received from the eye tracker.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_gaze_data.
*/
typedef void(*tobii_research_gaze_data_callback)(TobiiResearchGazeData* gaze_data, void* user_data);

/**
@brief Subscribes to gaze data for the eye tracker.

You will get a callback when time synchronized gaze is received.Time synchronized gaze is not supported on all eye trackers,
other eye trackers need additional license to activate this support.

\snippet gaze_data.c Example

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the gaze data.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_gaze_data(
                        TobiiResearchEyeTracker* eyetracker,
                        tobii_research_gaze_data_callback callback, void* user_data);

/**
@brief Unsubscribes from gaze data for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_gaze_data.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_gaze_data(
                        TobiiResearchEyeTracker* eyetracker,
                        tobii_research_gaze_data_callback callback);


/**
@brief User position guide data callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_user_position_guide.
@param gaze_data: Gaze data received from the eye tracker.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_user_position_guide.
*/
typedef void(*tobii_research_user_position_guide_callback)(TobiiResearchUserPositionGuide* user_position_guide,
    void* user_data);


/**
@brief Subscribes to user position guide for the eye tracker.

\snippet user_position_guide.c Example

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the user position guide.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_user_position_guide(
                        TobiiResearchEyeTracker* eyetracker,
                        tobii_research_user_position_guide_callback callback, void* user_data);

/**
@brief Unsubscribes from user position guide for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_user_position_guide.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_user_position_guide(
                        TobiiResearchEyeTracker* eyetracker,
                        tobii_research_user_position_guide_callback callback);

/**
@brief HMD gaze data callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_hmd_gaze_data.
@param hmd_gaze_data: HMD gaze data received from the eye tracker.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_hmd_gaze_data.
*/
typedef void(*tobii_research_hmd_gaze_data_callback)(TobiiResearchHMDGazeData* hmd_gaze_data, void* user_data);

/**
@brief Subscribes to gaze data for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the gaze data.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_hmd_gaze_data(
        TobiiResearchEyeTracker* eyetracker, tobii_research_hmd_gaze_data_callback callback, void* user_data);

/**
@brief Unsubscribes from HMD gaze data for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_hmd_gaze_data.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_hmd_gaze_data(
        TobiiResearchEyeTracker* eyetracker, tobii_research_hmd_gaze_data_callback callback);

/**
@brief External signal callback.

Implement this and send as a parameter to #tobii_research_subscribe_to_external_signal_data.
@param external_signal_data: External signal data received from the eye tracker.
@param user_data: Caller specific data sent in with #tobii_research_subscribe_to_external_signal_data.
*/
typedef void(*tobii_research_external_signal_data_callback)(TobiiResearchExternalSignalData* external_signal_data,
        void* user_data);

/**
@brief Subscribes to external signal data for the eye tracker.

You will get a callback when the value of the external signal port (TTL input) on the eye tracker device changes. Not
all eye trackers have an output trigger port. The output feature could be used to synchronize the eye tracker data
with data from other devices. The output data contains a time reference that matches the time reference on the time
synchronized gaze data.

\snippet external_signal.c Example

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the external signal data.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_external_signal_data(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_external_signal_data_callback callback, void* user_data);

/**
@brief Unsubscribes from external signal data for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_external_signal_data
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_external_signal_data(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_external_signal_data_callback callback);

/**
@brief Time synchronization callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_time_synchronization_data.
@param gaze_data: Time synchronization data received from the eye tracker.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_time_synchronization_data.
*/
typedef void(*tobii_research_time_synchronization_data_callback)(
        TobiiResearchTimeSynchronizationData* time_synchronization_data,
        void* user_data);

/**
@brief Subscribes to time synchronization data for the eye tracker.

You will get a callback when the computer and the eye trackers clocks gets synchronized. To handle normal drifts
between clocks the clocks are checked on regular basis, and this results in that the time stamps are adjusted for the
drifts in the data streams. This drift handling is done in real time. The data received from this event could be used
for an even more accurate drift adjustment in the post processing.

\snippet time_synchronization_data.c Example

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the time synchronization data.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_time_synchronization_data(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_time_synchronization_data_callback callback,
        void* user_data);

/**
@brief Unsubscribes from time synchronization data for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_time_synchronization_data.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_time_synchronization_data(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_time_synchronization_data_callback callback);


/**
Provides properties for the stream error data.
*/
typedef struct {
    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t system_time_stamp;
    /**
    Type of error.
    */
    TobiiResearchStreamError error;
    /**
    Source of the error.
    */
    TobiiResearchStreamErrorSource source;
    /**
    The error message.
    */
    const char* message;
} TobiiResearchStreamErrorData;

/**
@brief Stream error callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_stream_errors.
@param stream_error: @ref TobiiResearchStreamErrorData object.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_stream_errors.
*/
typedef void(*tobii_research_stream_error_callback)(
    TobiiResearchStreamErrorData* stream_error,
    void* user_data);

/**
@brief Subscribes to stream errors for the eye tracker.

You will get a callback when an error occurs on other streams. You can get errors when subscribing, when something
happened to the connection in the stream pump or when an error was raised in a callback.

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the stream errors.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_stream_errors(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_stream_error_callback callback,
        void* user_data);

/**
@brief Unsubscribes from stream errors for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_stream_errors.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_stream_errors(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_stream_error_callback callback);

/**
@brief Notification callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_notifications.
@param notification: Notification received from the eye tracker.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_notifications.
*/
typedef void(*tobii_research_notification_callback)(
        TobiiResearchNotification* notification,
        void* user_data);

/**
@brief Subscribes to notifications for the eye tracker.

You will get a callback when notification is received.

\snippet notifications.c Example

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the notifications.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_notifications(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_notification_callback callback,
        void* user_data);

/**
@brief Unsubscribes from notifications for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_notifications.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_notifications(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_notification_callback callback);

/**
Defines eye image type.
*/
typedef enum {
    /**
    Indicates that the eye tracker could not identify the eyes, and the image is the full image.
    */
    TOBII_RESEARCH_EYE_IMAGE_TYPE_FULL,
    /**
    Indicates that the image is cropped and shows the eyes.
    */
    TOBII_RESEARCH_EYE_IMAGE_TYPE_CROPPED,
    /**
    The eye image is part of a group of regions of interest.
    */
    TOBII_RESEARCH_EYE_IMAGE_TYPE_MULTI_ROI,
    /**
    The eye image has an unexpected type.
    */
    TOBII_RESEARCH_EYE_IMAGE_TYPE_UNKNOWN
} TobiiResearchEyeImageType;

/**
Provides data for the @ref tobii_research_eye_image_callback callback.
*/
typedef struct {
    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t device_time_stamp;
    /**
    The time stamp according to the computer's internal clock.
    */
    int64_t system_time_stamp;
    /**
    The bits per pixel for the eye image.
    */
    int bits_per_pixel;
    /**
    The padding bits per pixel for the eye image.
    */
    int padding_per_pixel;
    /**
    The width in pixel for the eye image.
    */
    int width;
    /**
    The height in pixels for the eye image.
    */
    int height;
    /**
    The type of eye image.
    */
    TobiiResearchEyeImageType type;
    /**
    The source/which camera that generated the image.
    */
    int camera_id;
    /**
    Size in bytes of the data blob.
    */
    size_t data_size;
    /**
    The data blob sent by the eye tracker.
    */
    void* data;
    /**
    The region id for the eye image.
    */
    int region_id;
    /**
    The top position in pixels for the eye image.
    */
    int top;
    /**
    The left position in pixels for the eye image.
    */
    int left;
} TobiiResearchEyeImage;

/**
Provides data for the @ref tobii_research_eye_image_as_gif_callback callback.
*/
typedef struct {
    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t device_time_stamp;
    /**
    The time stamp according to the computer's internal clock.
    */
    int64_t system_time_stamp;
    /**
    The type of eye image.
    */
    TobiiResearchEyeImageType type;
    /**
    The source/which camera that generated the image.
    */
    int camera_id;
    /**
    Size in bytes of the image data.
    */
    size_t image_size;
    /**
    The GIF image data.
    */
    void* image_data;
    /**
    The region id for the eye image.
    */
    int region_id;
    /**
    The top position in pixels for the eye image.
    */
    int top;
    /**
    The left position in pixels for the eye image.
    */
    int left;
} TobiiResearchEyeImageGif;

/**
@brief Eye image callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_eye_image.
@param frame: Eye image frame received from the eye tracker.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_eye_image.
*/
typedef void(*tobii_research_eye_image_callback)(TobiiResearchEyeImage* frame, void* user_data);

/**
@brief Eye image gif callback.

Implement this and send as a parameter to @ref tobii_research_subscribe_to_eye_image_as_gif.
@param frame: Eye image gif frame received from the eye tracker.
@param user_data: Caller specific data sent in with @ref tobii_research_subscribe_to_eye_image_as_gif.
*/
typedef void(*tobii_research_eye_image_as_gif_callback)(TobiiResearchEyeImageGif* frame, void* user_data);

/**
@brief  Subscribes to eye image for the eye tracker.

You will get a callback when a new eye image is received, and the occurrence depends on the eye tracker model. Not all
eye tracker models support this feature. If no one is listening to gaze data, the eye tracker will only deliver full
images, otherwise either cropped or full images will be delivered depending on whether or not the eye tracker has
detected eyes.

\snippet eye_images.c Example

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the frames.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_eye_image(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_eye_image_callback callback,
        void* user_data);

/**
@brief  Subscribes to eye image for the eye tracker with the image delivered in gif format.

You will get a callback when a new eye image is received, and the occurrence depends on the eye tracker model. Not all
eye tracker models support this feature. If no one is listening to gaze data, the eye tracker will only deliver full
images, otherwise either cropped or full images will be delivered depending on whether or not the eye tracker has
detected eyes.

\snippet eye_images_as_gif.c Example

@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the frames.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_eye_image_as_gif(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_eye_image_as_gif_callback callback,
        void* user_data);

/**
@brief Unsubscribes from eye image for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_eye_image.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_eye_image(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_eye_image_callback callback);

/**
@brief Unsubscribes from eye image for the eye tracker.

@param eyetracker: Eye tracker object.
@param callback: Callback sent to @ref tobii_research_subscribe_to_eye_image_as_gif.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_eye_image_as_gif(
        TobiiResearchEyeTracker* eyetracker,
        tobii_research_eye_image_as_gif_callback callback);

/**
Provides data for the @ref tobii_research_eye_openness_data_callback callback.
*/
typedef struct {
    /**
    The time stamp according to the eye tracker's internal clock.
    */
    int64_t device_time_stamp;

    /**
    The time stamp according to the computer's internal clock.
    */
    int64_t system_time_stamp;

    /**
    Gets the validity of the eye openness data for the left eye.
    */
    TobiiResearchValidity left_eye_validity;

    /**
    The value of the left absolute eye openness.
    */
    float left_eye_openness_value;

    /**
    Gets the validity of the eye openness data for the right eye.
    */
    TobiiResearchValidity right_eye_validity;

    /**
    The value of the right absolute eye openness.
    */
    float right_eye_openness_value;
} TobiiResearchEyeOpennessData;

typedef void(*tobii_research_eye_openness_data_callback)(
    TobiiResearchEyeOpennessData* openness_data, void* user_data);

/**
@brief Subscribes to eye openness for the eye tracker.
@param eyetracker: Eye tracker object.
@param callback: Callback that will receive the eye openness data.
@param user_data: Caller specific data that will be sent to the callback.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_subscribe_to_eye_openness(
    TobiiResearchEyeTracker* eyetracker, tobii_research_eye_openness_data_callback callback, void* user_data
);

/**
@brief Unsubscribes from eye openness for the eye tracker.
@param eyetracker: Eye tracker object.
@param callback: Callback to unsubscribe.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_unsubscribe_from_eye_openness(
    TobiiResearchEyeTracker* eyetracker, tobii_research_eye_openness_data_callback callback
);

#ifdef __cplusplus
}
#endif
#endif /* TOBII_RESEARCH_STREAMS_H_ */
