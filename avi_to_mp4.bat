@echo off
REM AVI to MP4 converter using FFmpeg
REM Usage: avi_to_mp4.bat input.avi [output.mp4]
REM If output not specified, uses input name with .mp4 extension

if "%~1"=="" (
    echo Usage: %0 input.avi [output.mp4]
    exit /b 1
)

set INPUT=%~1
set OUTPUT=%~2
if "%OUTPUT%"=="" set OUTPUT=%~dpn1.mp4

echo Converting %INPUT% to %OUTPUT%...
ffmpeg -i "%INPUT%" -c:v libx264 -preset fast -crf 23 -c:a aac -b:a 128k "%OUTPUT%"

if %ERRORLEVEL%==0 (
    echo Conversion successful!
) else (
    echo Conversion failed!
)
pause
