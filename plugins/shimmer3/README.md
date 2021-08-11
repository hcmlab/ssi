```
------------------------------------------------------------
	Shimmer3 Plugin // Social Signal Interpretation (SSI)
------------------------------------------------------------

(c) University of Augsburg
	Lab for Human Centered Multimedia
	support@openssi.net
	http://openssi.net

	Published under GNU General Public License 
	see also LICENSE
	
------------------------------------------------------------
	Credits
------------------------------------------------------------

	Shimmer C# API: https://github.com/ShimmerEngineering/Shimmer-C-API
    Mixed Mode DLL loading: https://support.goldsim.com/hc/en-us/articles/217950017-DLL-using-NET-C-Framework?mobile_site=true
	
------------------------------------------------------------
	Bug report to support@openssi.net
------------------------------------------------------------
```

# Shimmer3 SSI plugin
This plugin enables using Shimmer3 boards as sensors in SSI pipelines.

For setup information regarding the Shimmer3 boards see [the subfolder `./ShimmerSensorSetup`](./ShimmerSensorSetup).

# Installation Requirements
Refer to [./INSTALL](./INSTALL)

# Notes on the plugin implementation

## Why not use Shimmer's prebuilt APIs?
ShimmerSensing publishes a number of APIs to easily work with their boards. They handle the bluetooth communication and decoding of Shimmer's custom protocol. Sadly, none of those APIs is provided in C++. Therefore the C# API (https://github.com/ShimmerEngineering/Shimmer-C-API) was used as a blueprint and reimplemented (in parts) in this plugin.

## Design of the reimplementation of the open source ShimmerAPI
Currently only the GSR+ board is in use at the Augsburg University, therefore only the feature-set specific for this board is implemented. However, the implementation was carefully designed to make extending the plugin to support other boards as easy as possible.

Shimmer boards, regardless of their extension (GSR/ECG/EXG/...) use the same basic firmware (LogAndStream). The protocol of that firmware is implemented in the class `ssi::shimmer3::LogAndStreamDevice`. This class is purposefully __NOT__ a SSI `ISensor`, because it can be used to communicate with any of the Shimmer3 boards using the LogAndStream firmware (version >= 0.6.0).

The SSI `ISensor` representing a specific board should be implemented separately and only internally use an instance of `LogAndStreamDevice` to communicate with the board. See `ssi::Shimmer3GSRPlus` for a working example of this paradigm (and notice how that class is completely unaware of the intricacies of ShimmerSensing's binary protocol).

> In short, `LogAndStreamDevice` can be used to retrieve data packets from the shimmer containing raw values from any of the possible sensors (which change depending on the concrete board). For each board an ISensor class should be created that takes the raw values provided by the `LogAndStreamDevice` and apply any conversion necessary to create meaningful SSI streams from the data.

## Using Shimmer's closed source algorithms (e.g. PPG to Heart Rate)
Shimmer Sensing has implemented a number of advanced algorithms to convert their raw data into more useful formats, e.g. continuous PPG streams into HeartRate estimates. However, the code for these conversions is not open source and Shimmer Sensing only published them in binary form (a C# dll): [ShimmerClosedLibraryRev0_5.dll](https://github.com/ShimmerEngineering/Shimmer-C-API/blob/master/ShimmerPPGHRGSRConsoleAppExample/ShimmerConsoleAppExample/libs/ShimmerClosedLibraryRev0_5.dll).

This dll was integrated into our pure C++ context through the use of a mixed-mode DLL built with C++/CLI. For more information look at the documentation in [`./build/dlls/ShimmerPPGtoHRMixedModeWrapper/dllmain.cpp`](./build/dlls/ShimmerPPGtoHRMixedModeWrapper/dllmain.cpp) and [`./include/ShimmerClosedLibraryAlgoFactory.h`](./include/ShimmerClosedLibraryAlgoFactory.h)