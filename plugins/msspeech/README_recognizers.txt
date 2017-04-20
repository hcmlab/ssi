#======================================================================#
| Important Notice: Speech Platform vs. SAPI                           |
#======================================================================#


This plugin will use *either* Microsoft Speech Platform *or* regular SAPI,
depending on which files were included at build time.

The problem is that the Speech Platform is derived from SAPI
and for some reason, the Microsoft developers
kept the "sapi*" names for the files in its SDK.

Consequently, the compiler will default to using SAPI
unless it finds the Speech Platform files first.


+----------------------------------------------------------------------+
| Key differences between both recognizers                             |
+----------------------------------------------------------------------+

-  SAPI:
   - recognition optimized for being trained for one specific user
   - restricted to language engine embedded in Windows itself

- Speech Platform:
  - recognition optimized for many different users without any training
  - runtime language engines can be downloaded and installed as needed


+----------------------------------------------------------------------+
| In order to use Microsoft Speech Platform 11, follow these steps:    |
+----------------------------------------------------------------------+

1) Download and install the Microsoft Speech Platform SDK 11.
   http://www.microsoft.com/en-us/download/confirmation.aspx?id=27226

2) Make sure that the project's include and library paths point to this SDK.

   - Open its property pages,
     then go to "Configuration Properties -> VC++ Directories".

   - Check that "Include Directories" and "Library Directories"
     list the SDK paths FIRST.

     For the default installation (32 bit), these are:
     - "C:\Program Files (x86)\Microsoft SDKs\Speech\v11.0\Include"
     - "C:\Program Files (x86)\Microsoft SDKs\Speech\v11.0\Lib"

	 For the default installation (64 bit), these are:
     - "C:\Program Files\Microsoft SDKs\Speech\v11.0\Include"
     - "C:\Program Files\Microsoft SDKs\Speech\v11.0\Lib"


3) Finally, install the runtime components before using the plugin.

   - Speech Platform Runtime:
     http://www.microsoft.com/en-us/download/details.aspx?id=27225
	 (or use the installer at <plugin directory>\build\bin\SpeechPlatformRuntime.msi)

   - Recognition engine(s):
     http://www.microsoft.com/en-us/download/details.aspx?id=27224

	 - Clicking on "Download" will open a list
	   with all available recognition and text-to-speech engines.

     - Select "MSSpeech_SR_<culture code>_TELE.msi" for all languages
       you want to recognize.
	   The culture code must match the language of your grammar,
       for example, "de-DE" for German and "en-US" for US English.


Note: The Microsoft Kinect also uses Speech Platform for speech recognition,
      so some of these components are already included in the Kinect SDK.
 

+----------------------------------------------------------------------+
| How to tell if the plugin was compiled with Speech Platfrom or SAPI: |
+----------------------------------------------------------------------+

a) Check the recognizer ID which is displayed when the plugin starts.

   - If it uses SAPI, the ID looks like
        "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\
         Recognizers\Tokens\MS-1031-80-DESK"

   - If it uses Speech Platform, the ID looks like
        "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech Server\
         v11.0\Recognizers\Tokens\SR_MS_de-DE_TELE_11.0"
     or "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech Server\
         v11.0\Recognizers\Tokens\SR_MS_en-US_Kinect_11.0"
   

b) The plugin crashes if it was compiled with SAPI and tries to use a language
   different from the language of your Windows.
   
   Although the Speech Platform recognition engine for this language
   was properly installed, the plugin will keep telling you 
   "Error: specified recognizier not available.Could not create speech recognizer.
    Please ensure that Microsoft Speech SDK and other sample requirements
	are installed."
