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
:: run the Grammar Validator
::-----------------------------------------------------------
"%TOOLDIR%\GrammarValidator" -In %GRAMMAR% -RecoConfig %RECO_CONFIG%

PAUSE
