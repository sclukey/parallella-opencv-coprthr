#!/bin/bash

set -e

ESDK=${EPIPHANY_HOME}
ELIBS=${ESDK}/tools/host/lib:${LD_LIBRARY_PATH}
EHDF=${EPIPHANY_HDF}

OPENCVLIBS=/usr/local/opencv/lib

setterm -blank 0 -powersave off -powerdown 0 -cursor off
clear
sudo -E LD_LIBRARY_PATH=${ELIBS}:${OPENCVLIBS} EPIPHANY_HDF=${EHDF} ./main
setterm -blank 0 -cursor on

