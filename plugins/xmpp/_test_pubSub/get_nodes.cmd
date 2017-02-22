REM (c)2015 Andreas Seiderer: get all nodes from server

echo off
title get nodes
cls

python pubsub_client.py -j test@andy -p 1234 pubsub.andy nodes

pause