FOR /F "tokens=*" %%R IN ('where xmlpipe.exe') DO SET PIPE=%%R
copy %PIPE% .
xmlpipe.exe -init -export %1