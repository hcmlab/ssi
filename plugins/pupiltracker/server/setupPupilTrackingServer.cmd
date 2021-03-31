@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
SET parent=%~dp0

IF NOT EXIST pupil_tracking-server (
    IF NOT Exist pupil_tracking-server.zip (
        ::Download server code. TODO: make reading public or upload to github!
        wget https://hcai.eu/git/alexanderh/pupil_tracking/-/archive/server/pupil_tracking-server.zip
    )

    ECHO Unzipping server
    powershell.exe -nologo -noprofile -command "& { $shell = New-Object -COM Shell.Application; $target = $shell.NameSpace('%parent%'); $zip = $shell.NameSpace('%parent%pupil_tracking-server.zip'); $target.CopyHere($zip.Items(), 16); }"
    ECHO Unzipped server to %parent%pupil_tracking-server
    DEL pupil_tracking-server.zip
)

cd pupil_tracking-server

SET imagetag=pupiltrackingserver
SET containerName=pupiltrackingserver
SET videoMountDirectory=C:\Users\admin\codingprojects\HCMLab\TestMount

setupDockerContainer.cmd

PAUSE
EXIT /B 0