@echo off
set CONDA_BASE=C:\Users\shogo\anaconda3
set ENV_NAME=ssi310
set SSI_PATH=C:\ssi\bin\x64\vc140

call %CONDA_BASE%\Scripts\activate.bat %CONDA_BASE%\envs\%ENV_NAME%
set PATH=%SSI_PATH%;%CONDA_PREFIX%;%CONDA_PREFIX%\Library\bin;%CONDA_PREFIX%\Scripts;%PATH%

set PYTHONHOME=%CONDA_PREFIX%
set PYTHONPATH=C:\ssi\lonebula;%CONDA_PREFIX%\Lib;%CONDA_PREFIX%\DLLs

echo --------------------------------------------------
echo [SSI] Starting System
echo ENV: %CONDA_PREFIX%
echo --------------------------------------------------

cd /d C:\ssi\lonebula

:: --- GoProの確実な起動シーケンス ---
echo [GoPro] Resetting stream state...
python gopro_action.py stream_stop
timeout /t 2 /nobreak >nul

echo [GoPro] Initiating delayed stream start (5 sec)...
start /b cmd /c "timeout /t 5 /nobreak >nul & python gopro_action.py stream_start"
:: -----------------------------------

C:\ssi\bin\x64\vc140\xmlpipe.exe C:\ssi\lonebula\main.pipeline -config C:\ssi\lonebula\main.pipeline-config