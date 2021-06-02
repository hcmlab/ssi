@echo off



set PYTHON_VER_FULL=3.7.7
set PYTHON_VER=37

set SRC=https://github.com/hcmlab/ssi/raw/master/bin/x64/vc140/
set DST=.\bin\x64\vc140\
set LIB=.\libs\build\


%DST%wget.exe https://www.python.org/ftp/python/%PYTHON_VER_FULL%/python-%PYTHON_VER_FULL%-embed-amd64.zip -O %DST%python-%PYTHON_VER_FULL%-embed-amd64.zip
%DST%wget.exe https://github.com/python/cpython/archive/refs/tags/v%PYTHON_VER_FULL%.zip -O %LIB%python-%PYTHON_VER_FULL%.zip
%DST%7za.exe x %DST%python-%PYTHON_VER_FULL%-embed-amd64.zip -aoa -o%DST%
%DST%7za.exe x %DST%python%PYTHON_VER%.zip -aoa -o%DST%python%PYTHON_VER%
%DST%7za.exe x %LIB%python-%PYTHON_VER_FULL%.zip -aoa -o%LIB%
move %LIB%\cpython-%PYTHON_VER_FULL% %LIB%\Python-%PYTHON_VER_FULL% 

%DST%wget.exe -q https://bootstrap.pypa.io/get-pip.py -O %DST%get-pip.py
%DST%wget.exe -q %SRC%python%PYTHON_VER%._pth -O %DST%python%PYTHON_VER%._pth
%DST%python %DST%get-pip.py
%DST%python -m pip install numpy
%DST%%python -m pip install scipy

del %LIB%python-%PYTHON_VER_FULL%.zip
del %DST%python-%PYTHON_VER_FULL%-embed-amd64.zip
del %DST%python%PYTHON_VER%.zip

