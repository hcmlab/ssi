@echo off
set CONDA_BASE=C:\Users\shogo\anaconda3
set ENV_NAME=ssi310
set SSI_PATH=C:\ssi\bin\x64\vc140

call %CONDA_BASE%\Scripts\activate.bat %CONDA_BASE%\envs\%ENV_NAME%
set PATH=%SSI_PATH%;%CONDA_PREFIX%;%CONDA_PREFIX%\Library\bin;%CONDA_PREFIX%\Scripts;%PATH%

set PYTHONHOME=%CONDA_PREFIX%
set PYTHONPATH=C:\ssi\lonebula;%CONDA_PREFIX%\Lib;%CONDA_PREFIX%\DLLs

echo --------------------------------------------------
echo [TEST] Starting GoPro Minimun Pipeline
echo --------------------------------------------------

cd /d C:\ssi\lonebula

:: 念のためゾンビ状態のストリームを停止
echo [GoPro] Resetting stream state...
python gopro_action.py stream_stop
timeout /t 2 /nobreak >nul

:: バックグラウンドで遅延スタートを仕掛ける (5秒後に起動)
echo [GoPro] Initiating delayed stream start (5 sec)...
start /b cmd /c "timeout /t 5 /nobreak >nul & python gopro_action.py stream_start"

:: SSI起動
C:\ssi\bin\x64\vc140\xmlpipe.exe C:\ssi\lonebula\gopro.pipeline