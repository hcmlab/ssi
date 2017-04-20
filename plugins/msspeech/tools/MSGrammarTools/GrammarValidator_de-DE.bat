::-----------------------------------------------------------
:: path to the MS Grammar Developer Tools
::-----------------------------------------------------------

SET TOOLDIR=c:\Program Files\Microsoft SDKs\Speech\v11.0\Tools

::-----------------------------------------------------------
:: configuration for German sample grammar
::-----------------------------------------------------------

SET GRAMMAR=..\..\pipes\SampleGrammar.xml

SET RECO_CONFIG=RecoConfig_de-DE.xml


::-----------------------------------------------------------
:: run the Grammar Validator
::-----------------------------------------------------------
"%TOOLDIR%\GrammarValidator" -In %GRAMMAR% -RecoConfig %RECO_CONFIG%

PAUSE
