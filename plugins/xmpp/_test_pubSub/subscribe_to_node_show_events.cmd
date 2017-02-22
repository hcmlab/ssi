REM (c)2015 Andreas Seiderer: subscribe to event and show content

echo off
title get nodes
cls

python pubsub_client.py -j logger@andy -p 123 pubsub.andy subscribe testnode2
python pubsub_events.py -j logger@andy -p 123

pause