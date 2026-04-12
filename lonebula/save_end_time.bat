@echo off
for /f %%i in ('powershell -Command "[int64](Get-Date -UFormat %%s)"') do set UNIX=%%i
echo %date%,%time%,%UNIX% > "%1\end_time.csv"
