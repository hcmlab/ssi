::-----------------------------------------------------------
:: path to the MS Grammar Developer Tools
::-----------------------------------------------------------

SET TOOLDIR=c:\Program Files\Microsoft SDKs\Speech\v11.0\Tools

::-----------------------------------------------------------
:: configuration for US English sample grammar
::-----------------------------------------------------------

SET GRAMMAR=..\..\pipes\SampleGrammar_en-US.xml

SET RECO_CONFIG=RecoConfig_en-US.xml

::-----------------------------------------------------------
:: time stamp
::-----------------------------------------------------------
SET HOUR=%TIME:~0,2%
if "%HOUR:~0,1%" == " " SET HOUR=0%HOUR:~1,1%
SET MIN=%TIME:~3,2%
if "%MIN:~0,1%" == " " SET MIN=0%MIN:~1,1%
SET SECS=%TIME:~6,2%
if "%SECS:~0,1%" == " " SET SECS=0%SECS:~1,1%
SET YEAR=%DATE:~-4%
SET MONTH=%DATE:~-7,2%
if "%MONTH:~0,1%" == " " SET MONTH=0%MONTH:~1,1%
SET DAY=%DATE:~-10,2%
if "%DAY:~0,1%" == " " SET DAY=0%DAY:~1,1%

SET TIMESTAMP=%YEAR%-%MONTH%-%DAY%_%HOUR%-%MIN%-%SECS%


::-----------------------------------------------------------
:: phrase to check
::-----------------------------------------------------------

SET PHRASE=go left

SET OUTFILE="logs\%TIMESTAMP%_%PHRASE%.xml"

::-----------------------------------------------------------
:: run the phrase checker
::-----------------------------------------------------------

"%TOOLDIR%\CheckPhrase" -In %GRAMMAR% -RecoConfig %RECO_CONFIG% -Phrase "%PHRASE%" -Out %OUTFILE%

PAUSE
