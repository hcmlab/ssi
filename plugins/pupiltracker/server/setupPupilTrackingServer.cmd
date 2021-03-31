@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
SET parent=%~dp0

SET servercodedir=PupilSizeTracker-server

IF NOT EXIST %servercodedir% (
    IF NOT Exist server.zip (
        ::Download server code
        wget https://github.com/FabianWildgrube/PupilSizeTracker/archive/refs/heads/server.zip
    )

    ECHO Unzipping server
    powershell.exe -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $target = $shell.NameSpace('%parent%'); $zip = $shell.NameSpace('%parent%server.zip'); $target.CopyHere($zip.Items(), 16); }"
    ECHO Unzipped server to %parent%%servercodedir%
    DEL server.zip
)

cd %servercodedir%

SET imagetag=pupiltrackingserver
SET containerName=pupiltrackingserver
SET videoMountDirectory=C:\Users\admin\codingprojects\HCMLab\TestMount

setupDockerContainer.cmd

PAUSE
EXIT /B 0