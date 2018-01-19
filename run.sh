#!/bin/sh
export HOSTNAME=localhost
( sleep 1; cd Bglclnt; ./bgl-GUI_X11-Linux ) &
cd tierra; ./tierra si0
