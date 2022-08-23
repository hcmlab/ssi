/**
@copyright COPYRIGHT 2019 - PROPERTY OF TOBII PRO AB
@copyright 2019 TOBII PRO AB - KARLSROVAGEN 2D, DANDERYD 182 53, SWEDEN - All Rights Reserved.

@copyright NOTICE:  All information contained herein is, and remains, the property of Tobii Pro AB and its suppliers, if any.  The intellectual and technical concepts contained herein are proprietary to Tobii Pro AB and its suppliers and may be covered by U.S.and Foreign Patents, patent applications, and are protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is strictly forbidden unless prior written permission is obtained from Tobii Pro AB.
*/

/**
* @file tobii_research_calibration.h
* @brief <b>Functionality for implementing calibration.</b>
*
*/

#ifndef TOBII_RESEARCH_CALIBRATION_H_
#define TOBII_RESEARCH_CALIBRATION_H_

#include "tobii_research.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
"Defines the overall status of a calibration process.
*/
typedef enum {
    /**
    Indicates that the calibration process failed.
    */
    TOBII_RESEARCH_CALIBRATION_FAILURE = 0,
    /**
    Indicates that the calibration process succeeded for both eyes.
    */
    TOBII_RESEARCH_CALIBRATION_SUCCESS = 1,
    /**
    Indicates that the left eye calibration process succeeded.
    */
    TOBII_RESEARCH_CALIBRATION_SUCCESS_LEFT_EYE = 2,
    /**
    Indicates that the right eye calibration process succeeded.
    */
    TOBII_RESEARCH_CALIBRATION_SUCCESS_RIGHT_EYE = 3
} TobiiResearchCalibrationStatus;

/**
Defines the selected eye.
*/
typedef enum TobiiResearchSelectedEye {
    /**
    Left Eye
    */
    TOBII_RESEARCH_SELECTED_EYE_LEFT,

    /**
    Right Eye
    */
    TOBII_RESEARCH_SELECTED_EYE_RIGHT,

    /**
    Both Eyes
    */
    TOBII_RESEARCH_SELECTED_EYE_BOTH
} TobiiResearchSelectedEye;

/**
Defines the validity of calibration eye data.
*/
typedef enum {
    /**
    The eye tracking failed or the calibration eye data is invalid.
    */
    TOBII_RESEARCH_CALIBRATION_EYE_VALIDITY_INVALID_AND_NOT_USED = -1,

    /**
    Eye Tracking was successful, but the calibration eye data was not used in calibration e.g. gaze was too far away.
    */
    TOBII_RESEARCH_CALIBRATION_EYE_VALIDITY_VALID_BUT_NOT_USED = 0,

    /**
    The calibration eye data was valid and used in calibration.
    */
    TOBII_RESEARCH_CALIBRATION_EYE_VALIDITY_VALID_AND_USED = 1,

    /**
    The calibration eye data has an unexpected validity.
    */
    TOBII_RESEARCH_CALIBRATION_EYE_VALIDITY_UNKNOWN
} TobiiResearchCalibrationEyeValidity;

/**
Represents the calibration sample data collected for one eye.
 */
typedef struct {
    /**
    The eye sample position on the active display Area for the left eye.
    */
    TobiiResearchNormalizedPoint2D position_on_display_area;
    /**
    Information about if the sample was used or not in the calculation for the left eye.
    */
    TobiiResearchCalibrationEyeValidity validity;
} TobiiResearchCalibrationEyeData;

/**
Represents the data collected for a calibration sample.
 */
typedef struct {
    /**
    The calibration sample data for the left eye.
    */
    TobiiResearchCalibrationEyeData left_eye;
    /**
    The calibration sample data for the right eye.
    */
    TobiiResearchCalibrationEyeData right_eye;
} TobiiResearchCalibrationSample;

/**
Represents the Calibration Point and its collected calibration samples.
*/
typedef struct {
    /**
    The position of the calibration point in the Active Display Area.
    */
    TobiiResearchNormalizedPoint2D position_on_display_area;
    /**
    An array of calibration samples.
    */
    TobiiResearchCalibrationSample* calibration_samples;
    /**
    Number of calibration calibration points in the array.
    */
    size_t calibration_sample_count;
} TobiiResearchCalibrationPoint;

/**
Represents the result of the calculated calibration.
*/
typedef struct {
    /**
    Array of calibration points.
    */
    TobiiResearchCalibrationPoint* calibration_points;
    /**
    Number of calibration calibration points in the array.
    */
    size_t calibration_point_count;
    /**
    Gets the status of the calculation.
    */
    TobiiResearchCalibrationStatus status;
} TobiiResearchCalibrationResult;


/**
Represents the result of the calculated HMD based calibration.
*/
typedef struct {
    /**
    Gets the status of the calculation.
    */
    TobiiResearchCalibrationStatus status;
} TobiiResearchHMDCalibrationResult;

/**
@brief Enters the screen based calibration mode and the eye tracker is made ready for collecting data and calculating new calibrations.

\snippet calibration.c CalibrationEnterExample

@param eyetracker: Eye tracker object.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL
                       tobii_research_screen_based_calibration_enter_calibration_mode(
                               TobiiResearchEyeTracker* eyetracker);

/**
@brief Leaves the screen based calibration mode.

\snippet calibration.c CalibrationLeftExample

@param eyetracker: Eye tracker object.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL
                        tobii_research_screen_based_calibration_leave_calibration_mode(
                                TobiiResearchEyeTracker* eyetracker);

/**
@brief Starts collecting data for a calibration point.

The argument used is the point the calibration user is assumed to
be looking at and is given in the active display area coordinate system.

You must call tobii_research_calibration_enter_calibration_mode before calling this function.
This function is blocking while collecting data and may take up to 10 seconds.

\snippet calibration.c CalibrationExample

@param eyetracker: Eye tracker object.
@param x: Normalized x coordinate on active display area where the user is looking.
@param y: Normalized y coordinate on active display area where the user is looking.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_screen_based_calibration_collect_data(
        TobiiResearchEyeTracker* eyetracker,
        float x,
        float y);

/**
@brief Removes the collected data associated with a specific calibration point.

\snippet calibration.c ReCalibrationExample

@param eyetracker: Eye tracker object.
@param x: Normalized x coordinate of point to discard.
@param y: Normalized y coordinate of point to discard.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_screen_based_calibration_discard_data(
        TobiiResearchEyeTracker* eyetracker,
        float x,
        float y);

/**
@brief Uses the collected data and tries to compute calibration parameters.

If the calculation is successful, the result is applied to the eye tracker.
If there is insufficient data to compute a new calibration or if the collected
data is not good enough then calibration is failed and will not be applied.

\snippet calibration.c CalibrationExample

@param eyetracker: Eye tracker object.
@param result: Represents the result of the calculated calibration.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_screen_based_calibration_compute_and_apply(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchCalibrationResult** result);

/**
@brief Free memory allocation for the calibration result received via @ref tobii_research_screen_based_calibration_compute_and_apply.

@param result: Calibration result to free.
*/
TOBII_RESEARCH_API void TOBII_RESEARCH_CALL tobii_research_free_screen_based_calibration_result(
        TobiiResearchCalibrationResult* result);

/**
@brief Starts collecting data for a calibration point.

The argument used is the point the calibration user is assumed to
be looking at and is given in the active display area coordinate system.

You must call tobii_research_calibration_enter_calibration_mode before calling this function.
This function is blocking while collecting data and may take up to 10 seconds.

@param eyetracker: Eye tracker object.
@param x: Normalized x coordinate on active display area where the user is looking.
@param y: Normalized y coordinate on active display area where the user is looking.
@param eye_to_calibrate: TobiiResearchSelectedEye instance that selects for which eye to collect data for the monocular calibration.
@param collected_eyes: TobiiResearchSelectedEye instance that indicates for which eyes data was collected for the monocular calibration.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_monocular_calibration_collect_data(TobiiResearchEyeTracker* eyetracker,
                float x, float y, TobiiResearchSelectedEye eye_to_calibrate, TobiiResearchSelectedEye* collected_eyes);

/**
@brief Removes the collected data associated with a specific calibration point.

@param eyetracker: Eye tracker object.
@param x: Normalized x coordinate of point to discard.
@param y: Normalized y coordinate of point to discard.
@param eye_to_calibrate:  TobiiResearchSelectedEye instance that selects for which eye to discard data for the monocular calibration.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_monocular_calibration_discard_data(TobiiResearchEyeTracker* eyetracker,
                float x, float y, TobiiResearchSelectedEye eye_to_calibrate);
/**
@brief Uses the collected data and tries to compute calibration parameters.

If the calculation is successful, the result is
applied to the eye tracker. If there is insufficient data to compute a new calibration or if the collected data is
not good enough then calibration is failed and will not be applied.

@param eyetracker: Eye tracker object.
@param result: @ref TobiiResearchCalibrationResult instance the calibration result.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL
    tobii_research_screen_based_monocular_calibration_compute_and_apply(TobiiResearchEyeTracker* eyetracker,
        TobiiResearchCalibrationResult** result);

/**
@brief Enters the hmd based calibration mode and the eye tracker is made ready for collecting data and calculating new calibrations.

\snippet hmd_calibration.c HMDCalibrationEnterExample

@param eyetracker: Eye tracker object.
@returns A @ref TobiiResearchStatus code.
 */
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL
                       tobii_research_hmd_based_calibration_enter_calibration_mode(
                               TobiiResearchEyeTracker* eyetracker);

/**
@brief Leaves the hmd based calibration mode.

\snippet hmd_calibration.c HMDCalibrationLeftExample

@param eyetracker: Eye tracker object.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL
                        tobii_research_hmd_based_calibration_leave_calibration_mode(
                                TobiiResearchEyeTracker* eyetracker);

/**
@brief Starts collecting data for a calibration point.

The argument used is the point the calibration user is assumed to be looking at and is given in the HMD coordinate system.

You must call tobii_research_screen_based_calibration_enter_calibration_mode before calling this function.
This function is blocking while collecting data and may take up to 10 seconds.

@param eyetracker: Eye tracker object.
@param x: x coordinate in the HMD coordinate system where the user is looking.
@param y: y coordinate in the HMD coordinate system where the user is looking.
@param z: z coordinate in the HMD coordinate system where the user is looking.

@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_hmd_based_calibration_collect_data(
    TobiiResearchEyeTracker* eyetracker, float x, float y, float z);


/**
@brief Uses the collected data and tries to compute calibration parameters.

If the calculation is successful, the result is applied to the eye tracker.
If there is insufficient data to compute a new calibration or if the collected
data is not good enough then calibration is failed and will not be applied.

\snippet hmd_calibration.c HMDCalibrationExample

@param eyetracker: Eye tracker object.
@param result: Represents the result of the calculated HMD calibration.
@returns A @ref TobiiResearchStatus code.
*/
TOBII_RESEARCH_API TobiiResearchStatus TOBII_RESEARCH_CALL tobii_research_hmd_based_calibration_compute_and_apply(
        TobiiResearchEyeTracker* eyetracker,
        TobiiResearchHMDCalibrationResult* result);

#ifdef __cplusplus
}
#endif
#endif /* TOBII_RESEARCH_CALIBRATION_H_ */
