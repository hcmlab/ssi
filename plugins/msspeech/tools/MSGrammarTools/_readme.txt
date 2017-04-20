This folder contains batch files for running the Grammar Developer Tools 
which are included in the MS Speech Platform SDK.
More details can be found here:
https://msdn.microsoft.com/en-us/library/office/hh362855(v=office.14).aspx


0. general
---------------------------
	All batch files assume that you have installed
	the Speech Platform SDK 11 (64 Bit) in its default location.
	If that is not the case, adjust the variable "TOOLDIR"
	at the beginning of the batch file.
	
	The grammar and recognizer configuration are specified under the heading
	"configuration for <language> sample grammar".
	Make sure that "GRAMMAR" points to your grammar
	and that the recognizer uses the matching language.

	
1. Phrase Checker
---------------------------
	Checks whether a given phrase can be recognized by this grammar.
	Change the variable "PHRASE" to the one you want to check.
	
	After a few moments the Phrase Checker reports the result. 
	It also writes a detailed log file (rules, semantics, phonetics etc.)
	to the "logs" folder, using the name pattern "<time stamp>_<phrase>.xml".


2. Grammar Validator
---------------------------
	Checks your grammar file for errors on the XML level,
	such as missing closing tags or incorrect rule references.
	Run this tool if the SSI plugin fails to load your grammar.
	
	However, this tool can NOT detect errors in the semantic tag scripts.
	Those can lead to incomplete recognition outputs
	or make the SSI plugin crash whenever it detects a particular phrase.
	If that happens, use the Phrase Checker tool on the problematic phrase.

