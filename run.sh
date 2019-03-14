#!/bin/bash

if [ -f "/proc/cpuinfo" ]; then
    CPU_NUM="$(cat /proc/cpuinfo | grep processor | wc -l)"
else
    CPU_NUM="8"
fi

python2 run_analysis.py "$@" --worker=${CPU_NUM}
