@echo off

SET SSI_BIN_DIR=D:\openssi\bin\x64\vc140

for /f %%f in ('dir /b bin') do (
	xcopy /Y %SSI_BIN_DIR%\%%f bin\%%f	
)
