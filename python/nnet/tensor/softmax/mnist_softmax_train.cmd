@echo off

set PYTHONPATH=\\?\%~dp0

..\..\..\..\bin\x64\vc140\python.exe mnist_softmax_train.py