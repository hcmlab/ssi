/**
@copyright COPYRIGHT 2019 - PROPERTY OF TOBII PRO AB
@copyright 2019 TOBII PRO AB - KARLSROVAGEN 2D, DANDERYD 182 53, SWEDEN - All Rights Reserved.

@copyright NOTICE:  All information contained herein is, and remains, the property of Tobii Pro AB and its suppliers, if any.  The intellectual and technical concepts contained herein are proprietary to Tobii Pro AB and its suppliers and may be covered by U.S.and Foreign Patents, patent applications, and are protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is strictly forbidden unless prior written permission is obtained from Tobii Pro AB.
*/

/**
* @file tobii_research_eyetracker.h
* @brief <b>Functionality for an eye tracker.</b>
*
*/

#ifndef TOBII_RESEARCH_EYETRACKER_H_
#define TOBII_RESEARCH_EYETRACKER_H_

#include "tobii_research.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Defines the capabilities.
*/
typedef enum {
    /**
    * No capabilities set.
    */
    TOBII_RESEARCH_CAPABILITIES_NONE,

    /**
    * Indicates that the device can have display areas set.
    */
    TOBII_RESEARCH_CAPABILITIES_CAN_SET_DISPLAY_AREA = 1 << 0,

    /**
    * Indicates that the device can deliver an external signal stream.
    */
    TOBII_RESEARCH_CAPABILITIES_HAS_EXTERNAL_SIGNAL = 1 << 1,

    /**
    * Indicates that the device can deliver an eye image stream.
    */
    TOBII_RESEARCH_CAPABILITIES_HAS_EYE_IMAGES = 1 << 2,

    /**
    * Indicates that the device can deliver a gaze data stream. Standard for all screen based eye trackers.
    */
    TOBII_RESEARCH_CAPABILITIES_HAS_GAZE_DATA  = 1 << 3,

    /**
    * Indicates that the device can deliver a HMD gaze data stream.
    */
    TOBII_RESEARCH_CAPABILITIES_HAS_HMD_GAZE_DATA = 1 << 4,

    /**
    * Indicates that screen based calibration can be performed on the device.
    */
    TOBII_RESEARCH_CAPABILITIES_CAN_DO_SCREEN_BASED_CALIBRATION = 1 << 5,

    /**
    * Indicates that HMD based calibration can be performed on the device.
    */
    TOBII_RESEARCH_CAPABILITIES_CAN_DO_HMD_BASED_CALIBRATION = 1 << 6,

    /**
    * Indicates that it's possible to get and set the HMD lens configuration on the device.
    */
    TOBII_RESEARCH_CAPABILITIES_HAS_HMD_LENS_CONFIG = 1 << 7,

    /**
    * Indicates that monocular calibration can be performed on the device.
    */
    TOBII_RESEARCH_CAPABILITIES_CAN_DO_MONOCULAR_CALIBRATION = 1 << 8,

    /**
    * Indicates that an eye openness stream is available on the device
    */
    TOBII_RESEARCH_CAPABILITIES_HAS_EYE_OPENNESS_DATA = 1 << 9,
} TobiiResearchCapabilities;

/**
Represents the eight corners in user coordinate system that together forms the track box.
*/
typedef struct {
    /**
    The back lower left corner of the track box.
    */
    TobiiResearchPoint3D back_lower_left;

    /**
    The back lower right corner of the track box.
    */
    TobiiResearchPoint3D back_lower_right;

    /**
    The back upper left corner of the track box.
    */
    TobiiResearchPoint3D back_upper_left;

    /**
    The back upper right corner of the track box.
    */
    TobiiResearchPoint3D back_upper_right;

    /**
    The front lower left corner of the track box.
    */
    TobiiResearchPoint3D front_lower_left;

    /**
    The front lower right corner of the track box.
    */
    TobiiResearchPoint3D front_lower_right;

    /**
    The front upper left corner of the track box.
    */
    TobiiResearchPoint3D front_upper_left;

    /**
    The front upper right corner of the track box.
    */
    TobiiResearchPoint3D front_upper_right;
} TobiiResearchTrackBox;

/**
Specifies license validation result.
*/
typedef enum TobiiResearchLicenseValidationResult {
    /**
    The license is ok.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_OK,

    /**
    The license is tampered.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_TAMPERED,

    /**
    The application signature is invalid.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_INVALID_APPLICATION_SIGNATURE,

    /**
    The application has not been signed.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_NONSIGNED_APPLICATION,

    /**
    The license has expired.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_EXPIRED,

    /**
    The license is not yet valid.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_PREMATURE,

    /**
    The process name does not match the license.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_INVALID_PROCESS_NAME,

    /**
    The serial number does not match the license.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_INVALID_SERIAL_NUMBER,

    /**
    The model does not match the license.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_INVALID_MODEL,

    /**
    The license validation returned an unexpected result.
    */
    TOBII_RESEARCH_LICENSE_VALIDATION_RESULT_UNKNOWN
} TobiiResearchLicenseValidationResult;

/**
Represents the corners of the active display area in the user coordinate system, and its size.
BottomRight, Height, and Width are calculated values.
*/
typedef struct {
    /**
    The bottom left corner of the active display area.
    */
    TobiiResearchPoint3D bottom_left;

    /**
    The bottom right corner of the active display area.
    */
    TobiiResearchPoint3D bottom_right;

    /**
    The height in millimeters of the active display area.
    */
    float height;

    /**
    The top left corner of the active display area.
    */
    TobiiResearchPoint3D top_left;

    /**
    The top right corner of the active display area.
    */
    TobiiResearchPoint3D top_right;

    /**
    The width in millimeters of the active display area.
    */
    float width;
} TobiiResearchDisplayArea;

/**
Represents the calibration data used by the eye tracker.
 */
typedef struct {
    /**
     The calibration data used by the eye tracker.
     */
    void* data;
    /**
     The size of the calibration data used by the eye tracker.
     */
    size_t size;
} TobiiResearchCalibrationData;

/**
Represents the gaze output frequencies supported by the eye tracker.
 */
typedef struct {
    /**
    An array of gaze sampling frequencies supported by the eye tracker.
    */
    float* frequencies;
    /**
    The number of gaze output frequencies.
    */
    size_t frequency_count;
} TobiiResearchGazeOutputFrequencies;

/**
Represents the eye tracking modes supported by the eye tracker.
 */
typedef struct {
    /**
    An array of strings containing eye tracking modes supported by the eye tracker.
    */
    char** modes;
    /**
    The number of eye tracking modes supported by the eye tracker.
    */
    size_t mode_count;
} TobiiResearchEyeTrackingModes;

/**
Represents the lens configuration of the HMD device.
 */
typedef struct {
    /**
    The point in HMD coordinate system that defines the position of the left lens (in millimeters).
     */
    TobiiResearchPoint3D left;
    /**
    The point in HMD coordinate system that defines the position of the right lens (in millimeters).
     */
    TobiiResearchPoint3D right;
} TobiiResearchHMDLensConfiguration;

/**
@brief Gets the address (URI) of the eye tracker device.

\snippet find_all_eyetrackers.c GetEyetrackerProps

@param eyetracker: Eye tracker object.
@param address: Address as string, should be freed when not in use by @ref tobii_research_free_string.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_address(
        TobiiResearchEyeTracker* eyetracker, char** address);
/**
@brief Gets the serial number of the eye tracker. All physical eye trackers have a unique serial number.

\snippet find_all_eyetrackers.c GetEyetrackerProps

@param eyetracker: Eye tracker object.
@param serial_number: Serial number as string, should be freed when not in use by @ref tobii_research_free_string.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_serial_number(
        TobiiResearchEyeTracker* eyetracker, char** serial_number);

/**
@brief Gets the name of the eye tracker.

\snippet find_all_eyetrackers.c GetEyetrackerProps

@param eyetracker: Eye tracker object.
@param device_name: Device name as string, should be freed when not in use by @ref tobii_research_free_string.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_device_name(
        TobiiResearchEyeTracker* eyetracker, char** device_name);

/**
@brief Gets the model of the eye tracker.

@param eyetracker: Eye tracker object.
@param model: Model as string, should be freed when not in use by @ref tobii_research_free_string.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_model(
        TobiiResearchEyeTracker* eyetracker, char** model);

/**
@brief Gets the firmware version of the eye tracker.

@param eyetracker: Eye tracker object.
@param fw_version: Firmware version as string, should be freed when not in use by @ref tobii_research_free_string.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_firmware_version(
        TobiiResearchEyeTracker* eyetracker, char** fw_version);

/**
@brief Gets the runtime build version of the eye tracker.

@param eyetracker: Eye tracker object.
@param runtime_version: Runtime build version as string, should be freed when not in use by @ref tobii_research_free_string.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_runtime_version(
        TobiiResearchEyeTracker* eyetracker, char** runtime_version);


/**
@brief Gets the capabilities of the device.

@param eyetracker: Eye tracker object.
@param capabilities: Bit array where each bit set indicates a supported capability, see @ref TobiiResearchCapabilities.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_capabilities(
        TobiiResearchEyeTracker* eyetracker, TobiiResearchCapabilities* capabilities);

/**
@brief Gets the calibration data used currently by the eye tracker.

This data can be saved to a file for later use. See @ref tobii_research_apply_calibration_data

\snippet calibration_data.c GetCalibrationData

@param eyetracker: Eye tracker object.
@param data: Calibration data.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_retrieve_calibration_data(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchCalibrationData** data);

/**
@brief Free memory allocation for the calibration data received via tobii_research_retrieve_calibration_data.

@param data: Calibration data to free.
*/
TOBII_RESEARCH_API void TOBII_RESEARCH_CALL tobii_research_free_calibration_data(
        TobiiResearchCalibrationData* data);

/**
@brief Sets the provided calibration data to the eye tracker, which means it will be active calibration.

This function should not be called during calibration. Also see @ref tobii_research_retrieve_calibration_data.

\snippet calibration_data.c ApplyCalibrationData

@param eyetracker: Eye tracker object.
@param data: Calibration data.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_apply_calibration_data(
        TobiiResearchEyeTracker* eyetracker,
        const TobiiResearchCalibrationData* data);

/**
@brief Gets an array of gaze output frequencies supported by the eye tracker.

\snippet gaze_output_frequencies.c GetOutputFrequencies

@param eyetracker: Eye tracker object.
@param frequencies: Gaze output frequencies.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_all_gaze_output_frequencies(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchGazeOutputFrequencies** frequencies);

/**
@brief Free memory allocation for the gaze output frequencies received via tobii_research_free_gaze_output_frequencies.

@param frequencies: Gaze output frequencies to free.
*/
TOBII_RESEARCH_API void TOBII_RESEARCH_CALL tobii_research_free_gaze_output_frequencies(
        TobiiResearchGazeOutputFrequencies* frequencies);

/**
@brief Gets the gaze output frequency of the eye tracker.

\snippet gaze_output_frequencies.c GetOutputFrequency

@param eyetracker: Eye tracker object.
@param gaze_output_frequency: The current gaze output frequency.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_gaze_output_frequency(
        TobiiResearchEyeTracker* eyetracker,
        float* gaze_output_frequency);

/**
@brief Sets the gaze output frequency of the eye tracker.

\snippet gaze_output_frequencies.c SetOutputFrequency

@param eyetracker: Eye tracker object.
@param gaze_output_frequency: The gaze output frequency.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_set_gaze_output_frequency(
        TobiiResearchEyeTracker* eyetracker,
        float gaze_output_frequency);

/**
@brief Gets a all eye tracking modes supported by the eye tracker.

\snippet eye_tracking_modes.c GetEyeTrackingModes

@param eyetracker: Eye tracker object.
@param modes: Eye tracking modes.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_all_eye_tracking_modes(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchEyeTrackingModes** modes);


/**
@brief Free memory allocation for the eye tracking modes received via tobii_research_get_all_eye_tracking_modes.

@param modes: Eye tracker modes to free.
*/
TOBII_RESEARCH_API void TOBII_RESEARCH_CALL tobii_research_free_eye_tracking_modes(
        TobiiResearchEyeTrackingModes* modes);

/**
@brief Gets the eye tracking mode of the eye tracker.

\snippet eye_tracking_modes.c GetEyeTrackingMode

@param eyetracker: Eye tracker object.
@param eye_tracking_mode: The current eye tracking mode.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_eye_tracking_mode(
        TobiiResearchEyeTracker* eyetracker,
        char** eye_tracking_mode);

/**
@brief Sets the eye tracking mode of the eye tracker.

\snippet eye_tracking_modes.c SetEyeTrackingMode

@param eyetracker: Eye tracker object.
@param eye_tracking_mode: The eye tracking mode.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_set_eye_tracking_mode(
        TobiiResearchEyeTracker* eyetracker,
        const char* eye_tracking_mode);

/**
@brief Gets the track box of the eye tracker.

\snippet get_track_box.c Example

@param eyetracker: Eye tracker object.
@param track_box: Track box coordinates.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_track_box(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchTrackBox* track_box);

/**
@brief Apply one or more licenses to unlock features of the eye tracker.

The validation_results array indicates whether all licenses were applied or not.

\snippet apply_licenses.c Example

@param eyetracker: Eye tracker object.
@param license_key: Licenses to apply.
@param license_keys_size: A list of license key sizes.
@param validation_results: Optional. Validation result for each license.
@param number_of_licenses: Number of licenses in license_key_ring.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_apply_licenses(
        TobiiResearchEyeTracker* eyetracker,
        const void** license_key,
        size_t* license_keys_size,
        TobiiResearchLicenseValidationResult* validation_results,
        size_t number_of_licenses);


/**
@brief Clears any previously applied licenses.

@param eyetracker: Eye tracker object.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_clear_applied_licenses(
        TobiiResearchEyeTracker* eyetracker);

/**
@brief Gets the size and corners of the display area.

\snippet get_and_set_display_area.c Example

@param eyetracker: Eye tracker object.
@param display_area: The eye tracker's display area.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_display_area(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchDisplayArea* display_area);

/**
@brief Sets the display area of the eye tracker. It is strongly recommended to use
Eye Tracker Manager to calculate the display area coordinates as the origin of the
User Coordinate System differs between eye tracker models.

\snippet get_and_set_display_area.c Example

@param eyetracker: Eye tracker object.
@param display_area: The eye tracker's desired display area.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_set_display_area(
        TobiiResearchEyeTracker* eyetracker,
        const TobiiResearchDisplayArea* display_area);

/**
@brief Changes the device name. This is not supported by all eye trackers.

\snippet set_device_name.c Example

@param eyetracker: Eye tracker object.
@param device_name: The eye tracker's desired name.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_set_device_name(
        TobiiResearchEyeTracker* eyetracker,
        const char* device_name);

/**
@brief Gets the current lens configuration of the HMD based eye tracker.
The lens configuration describes how the lenses of the HMD device are positioned.

\snippet get_hmd_lens_configuration.c Example

@param eyetracker: Eye tracker object.
@param lens_configuration: The eye tracker's lens_configuration.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_get_hmd_lens_configuration(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchHMDLensConfiguration* lens_configuration);

/**
@brief Sets the lens configuration of the HMD based eye tracker.
The lens configuration describes how the lenses of the HMD device are positioned.

\snippet set_hmd_lens_configuration.c Example

@param eyetracker: Eye tracker object.
@param lens_configuration: The eye tracker's desired lens_configuration.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_set_hmd_lens_configuration(
        TobiiResearchEyeTracker* eyetracker,
        const TobiiResearchHMDLensConfiguration* lens_configuration);

#ifdef __cplusplus
}
#endif
#endif /* TOBII_RESEARCH_EYETRACKER_H_ */
