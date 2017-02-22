=================================
             FUBI 
(Full Body Interaction Framework)
=================================
Version 1.0.0 ALPHA

Copyright (C) 2010-2015 Felix Kistler

http://www.hcm-lab.de/fubi.html

The framework is licensed under the terms of the Eclipse Public License (EPL).

FUBI makes use of the following third-party open source software included in code form:
 - RapidXml (http://rapidxml.sourceforge.net, Copyright (C) 2006, 2009 Marcin Kalicinski)
 
FUBI can be used with the following third-party open source software not included in the FUBI developer download, but in the FUBI installers and the Unity integration in binary form:
 - OpenCV (http://opencv.org, Copyright (C) 2000-2008, Intel Corporation, all rights reserved. Copyright (C) 2008-2011, Willow Garage Inc., all rights reserved., BSD license)
 - OpenNI (Copyright (c) 2012 PrimeSense Ltd., Apache License 2.0)
 - Leap Motion SDK (Copyright (c) 2012-2014 Leap Motion, Leap Motion SDK Agreement available at https://developer.leapmotion.com/sdk_agreement)

FUBI can be used with the following third-party closed source software not included in any FUBI download, however, installers may copy already installed binaries to the FUBI installation folder:
 - Microsoft Kinect SDK 1.x or 2.x (http://www.microsoft.com/en-us/kinectforwindows/)
 - NiTE
 
A documentation with pictures and more detailed tutorials can be found here:
http://www.informatik.uni-augsburg.de/lehrstuehle/hcm/projects/tools/fubi/doc/


Installation of third-party components
======================================
You need to install the following third-party components:
1. Kinect SDK 2:
http://www.microsoft.com/en-us/kinectforwindows/develop/
--> Download and install the Kinect for Windows SDK 2.x for the second generation Kinect

2. Kinect SDK 1:
http://www.microsoft.com/en-us/download/details.aspx?id=40278
http://www.microsoft.com/en-us/download/details.aspx?id=40276
--> Download and install the Kinect for Windows SDK 1.8 and Kinect for Windows Developer Toolkit 1.7for the first generation Kinect

Alternatively or in addition you can install:
3. OpenNI/NiTE:
**Unfortunately OpenNI/NITE has been discontinued and there exists no official download anymore.**
However, you can still get them from Simple OpenNI:
http://code.google.com/p/simple-openni/downloads/list?can=1
Use SimpleOpenNI 1.96 for OpenNI/NiTe version 2.x 
or OpenNI_NITE_Installer-win32-0.27 for OpenNI/NiTE version 1.x
If you want to use the Kinect for Xbox/Windows with OpenNI/NiTE 2.x: additionally install the Kinect SDK 1.x as described in 2.
On Linux, you further have to define three environment variables in your Code::Blocks environment in Settings => Environment => Environment variables:
OPENNI2_INCLUDE (should point to the OpenNI2 header files)
OPENNI2_REDIST (should point to the folder that contains the libOpenNI2.so)
NITE2_REDIST (should point to the folder that contains the libNITE2.so)
 
Alternatively or in addition you can install:
4. Leap Motion SDK:
https://developer.leapmotion.com/downloads
--> After registering, download and install the latest Leap Motion SDK
Add the Leap lib folder to your path, that should be similar to: C:\LeapSDK\lib\x86
Set the include and lib paths for the Leap SDK: In Visual Studio, navigate to View => Property Manager => "Microsoft.Cpp.Win32.user"  under a random (yes, really!) project and config => Select VC++-directories => Add the include and lib path of your Leap installation to the corresponding properties. Alternatively you find the related config file “Microsoft.Cpp.Win32.user.props” in "%LocalAppData%\Microsoft\MSBuild\v4.0" that you can edit with a text editor and add the relevant paths. This file should then look something like this:
    <?xml version="1.0" encoding="utf-8"?>
    <Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <PropertyGroup>
        <IncludePath>C:\LeapSDK\include;$(IncludePath)</IncludePath>
      </PropertyGroup>
      <PropertyGroup>
        <LibraryPath>C:\LeapSDK\lib\x86;$(LibraryPath)</LibraryPath>
      </PropertyGroup>
    </Project>

5. We recommend to additionally install OpenCV, currently, the latest tested versions are 2.4.11 and 3.0.0:
The windows installer for version 2.4.11 already has all required pre-compiled binaries. For version 3.0.0, you need to build them yourself!
For Windows: http://sourceforge.net/projects/opencvlibrary/files/opencv-win/3.0.0/opencv-3.0.0.exe/download
If you do not want to use OpenCV, comment out the line "#define USE_OPENCV" in the FubiConfig.h
Add the OpenCV bin folder to your path, that should be similar to: C:\OpenCV3.0.0\build\x86\vc10\bin (for VS 2010) or C:\OpenCV3.0.0\build\x86\vc12\bin (for VS 2013)
Set the include and lib paths for OpenCV: In Visual Studio, navigate to View => Property Manager => "Microsoft.Cpp.Win32.user"  under a random (yes, really!) project and config => Select VC++-directories => Add the include and lib path of your OpenCV installation to the corresponding properties. Alternatively you find the related config file “Microsoft.Cpp.Win32.user.props” in "%LocalAppData%\Microsoft\MSBuild\v4.0" that you can edit with a text editor and add the relevant paths. This file should then look something like this (for VS 2013; for VS 2010 exchange the vc12 to vc10 for the lib path):
    <?xml version="1.0" encoding="utf-8"?>
    <Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <PropertyGroup>
        <IncludePath>C:\OpenCV3.0.0\build\include;$(IncludePath)</IncludePath>
      </PropertyGroup>
      <PropertyGroup>
        <LibraryPath>C:\OpenCV3.0.0\build\x86\vc12\lib;$(LibraryPath)</LibraryPath>
      </PropertyGroup>
    </Project>
Fubi automatically checks for the installed OpenCV version and links against it. However, if you have different versions installed and you can select one by uncommenting the line "#define FUBI_OPENCV_VERSION 300" and change the 300 to the version that you would like to use. Nevertheless, you need to make sure that the correct headers are used!

 
Running the basic FUBI sample
=============================
1. Open the FUBI.sln
2. Edit the "FubiConfig.h" dependent on the sensor you want to use to be found under the project "Fubi" --> header files.
2. Set "RecognizerTest" (in the sample folder) as startup project, compile, and run it

The application will start, showing the depth image of the sensor with additional debug info for the user tracking.
The application as default checks for all user defined basic recognizers and prints the recognition on the console.
If you press "space", it will instead print the current state of all user defined combination recognizers to the console.
If you press "space" again, it will print the current state of all predefined defined combinations.
And if you press "space" once again, it will print the current state of all predefined basic recognizers.

Available keys:
---------------
ESC 	: Shutdown the application
Space 	: Switch between output of different recognizers
p		: Save pictures of the Kinect every 30 frames (1-2 sec.): 1 rgb image, 1 depth image, 1 tracking image
f		: Activate the finger count detection for the closest standing user every 15 frames (0,5-1 sec.)
r/i		: Switch to RGB or IR image (needs proper configuration in the Fubi::init() call at the top of the main function)
t		: Switch between different rendering options.
s		: Switch between different sensors (e.g. Kinect SDK and OpenNI).
d		: Enable/Disable Finger sensor (e.g. the Leap motion)
TAB		: Reload recognizers from the "SampleRecognizers.xml"
ENTER	: Start/stop recording skeleton data (starts playback of the data after stopping, default file name: "trainingData/tempRecord.xml")
o		: Load and start playback of skeleton data (default file name: "trainingData/tempRecord.xml")

The basic sample uses the glut library included in the Fubi distribution, Copyright (c) Mark J. Kilgard, 1994, 1995, 1996, 1998.


Running the FUBI GUI
====================
The second example in the Fubi solution is the FUBI GUI, a WPF application that demonstrates how to use the FUBI CS wrapper.
It provides a very good way to test your recognizers, as it provides almost everything that you can adjust in FUBI and further visualizes a lot of useful information about the Fubi functionality.
It further includes the option to bind mouse or key events to certain gesture occurrences and supports to record tracking data as well as training the values of a recognizer with an actual gesture performance and generating corresponding Fubi XML.
You can compile and run it in the same way as the basic example and test its functionalities.
More information can again be found in the online docu: http://www.informatik.uni-augsburg.de/lehrstuehle/hcm/projects/tools/fubi/doc/

Both samples are tested with OpenNI 2.2, NITE 2.2, OpenCV 2.4.11, as well as the Kinect SDK 2.0 and the Leap Motion SDK 2.2


Defining a recognizer in XML
============================
The main way to define recognizers in Fubi is defining them in XML and loading them via Fubi::loadRecognizersFromXML().
You therefore don't need any programming skills and have a comfortable way to adjust their parameters.
We recommend to use an XML editor that is able to use *.xsd files for XML schemes (e.g. Visual Studio on Windows).
If you open one of the existing XML sample files (e.g. "SampleRecognizers.xml" in the bin folder), your editor should automatically load the FubiRecognizers.xsd to support you while editing it.
More information can be found in the online documentation (http://www.informatik.uni-augsburg.de/lehrstuehle/hcm/projects/tools/fubi/doc/).


Adding recognizers via the FUBI API
===================================
If you need to dynamically add recognizers during runtime, you can do so by using the FUBI API functions called add...Recognizer().
All of them return an index of the recognizer that can be used to call it later. Alternatively, you can also assign them names for later reference.
As the the different recognizers have quite a lot of different parameters, the add..Recognizer() functions have a relatively complex signature and are not as comfortable to use as the XML. However, you can adapt them during runtime and add/remove recognizers as needed.
As combination recognizers are too complex for this method, the addCombinationRecognizer() function expects a string that contains an XML definition for the combination.


Implementation of a recognizer in code inside of Fubi
=====================================================
We don't recommend it anymore (you get into trouble when updating to a new Fubi version, and you always need to recompile Fubi after changing a recognizer), but it is still possible to implement recognizers directly in the code.
For basic recognizers, you can take the example recognizer "LeftHandUpRecognizer" in the folder "Fubi\GestureRecognizer" as a template.
It implements the interface of "Fubi\GestureRecognizer\IGestureRecognizer.h"
In "Fubi\Fubi.h" you have to add a value for your recognizer in the enum "Postures" and a description string in "getPostureName(Postures postureID)".
"Fubi\FubiRecognizerFactory.cpp" serves as a factory, and you have to link your enum-value with your recognizer's constructor.

If you want to add a combination recognizer, you should implemented it in "Fubi\FubiRecognizerFactory.cpp" following the example "WAVE_RIGHT_HAND_OVER_SHOULDER".
A combination consists of several states. Each state consists of a set of recognizers that need to be successful for going into this state. Each state has a minimum duration that all gestures have to be performed before proceeding to the next state, and a maximum duration, after which the recognition is aborted, if there has not been a transition to the next state, yet. At last, each state has a time for transition that is the maximum duration between holding the postures of the current and the next state (where both states are not fulfilled). In "Fubi\Fubi.h" you have to add a value for your recognizer in the enum "Combinations" and a description string in "getCombinationName(Combinations postureID)".



CHANGELOG
=========
Check Changelog.txt for a history of changes