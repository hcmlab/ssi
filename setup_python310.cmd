@echo off



set PYTHON_VER_FULL=3.10.6
set PYTHON_VER=310

set DST=.\bin\x64\vc140\
set LIB=.\libs\build\


%DST%wget.exe https://www.python.org/ftp/python/%PYTHON_VER_FULL%/python-%PYTHON_VER_FULL%-embed-amd64.zip -O %DST%python-%PYTHON_VER_FULL%-embed-amd64.zip
%DST%wget.exe https://github.com/python/cpython/archive/refs/tags/v%PYTHON_VER_FULL%.zip -O %LIB%python-%PYTHON_VER_FULL%.zip
%DST%7za.exe x %DST%python-%PYTHON_VER_FULL%-embed-amd64.zip -aoa -o%DST%
%DST%7za.exe x %DST%python%PYTHON_VER%.zip -aoa -o%DST%python%PYTHON_VER%
%DST%7za.exe x %LIB%python-%PYTHON_VER_FULL%.zip -aoa -o%LIB%
move %LIB%\cpython-%PYTHON_VER_FULL% %LIB%\Python-%PYTHON_VER_FULL% 

(echo . & echo .\ & echo .\DLLs & echo .\lib & echo .\lib\plat-win & echo .\lib\site-packages & echo .\python%PYTHON_VER%) > %DST%python%PYTHON_VER%._pth
%DST%wget.exe -q https://bootstrap.pypa.io/get-pip.py -O %DST%get-pip.py
%DST%python %DST%get-pip.py
%DST%python -m pip install numpy
%DST%python -m pip install scipy

del %LIB%python-%PYTHON_VER_FULL%.zip
del %DST%python-%PYTHON_VER_FULL%-embed-amd64.zip
del %DST%python%PYTHON_VER%.zip

PAUSE
