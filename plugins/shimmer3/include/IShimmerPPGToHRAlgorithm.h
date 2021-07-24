#pragma once

#ifndef IShimmerPPGToHRAlgorithm_H
#define IShimmerPPGToHRAlgorithm_H

#include <memory>

/// <summary>
/// Interface definition for the wrapper type for ShimmerSensing's closed source PPG to HeartRate conversion algorithm
/// </summary>
class IShimmerPPGToHRAlgorithm {
public:
	/// <summary>
	/// Converts PPG values (calibrated in mV) from a Shimmer3 device into a Heart Rate estimate.
	/// This function is expected to be called with every sample received from the Shimmer.
	/// </summary>
	/// <param name="data">ppg value in mV smoothed by a bandpass filter from 0.5-5 Hz. Unsmoothed input values will not produce HR estimates!</param>
	/// <param name="timestamp">timestamp fo the value in mS since session start</param>
	/// <returns>The current estimate of the Heart Rate, based on all the values passed to this function during previous calls</returns>
	virtual double ppgToHrConversion(double data, double timestamp) = 0;
};

// custom destructor is needed to be able to construct unique_ptr over a raw pointer
struct PPGToHRAlgorithDeletor {
	void operator()(IShimmerPPGToHRAlgorithm* p) {
		delete p;
	}
};

typedef std::unique_ptr<IShimmerPPGToHRAlgorithm, PPGToHRAlgorithDeletor> PPGToHRAlgorithmUniquePtr; //typedef to hide custom destructor from users


#endif //IShimmerPPGToHRAlgorithm_H