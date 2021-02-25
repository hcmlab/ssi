:: make sure that a current version of the "pupiltracker.dll" (release build) is either in the ssi master branch on github or in the directory of this file when you run it
:: (also: ssi variables need to be in the PATH. Otherwise "xmlchain.exe" will not be found

:: replace "inputVideo.mp4" with the video you want to process
xmlchain.exe -step 1 pupiltracker.chain inputVideo.mp4 inputVideoPupilTrackingOutput