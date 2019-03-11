#!/usr/bin/env bash

WORKING_DIR="$(cd $(dirname "{BASH_SOURCE[0]") && pwd)"
cd ${WORKING_DIR}

python2 -c "from worker import huey; huey.flush()"

if [ -f "/proc/cpuinfo" ]; then
    CPU_NUM="$(cat /proc/cpuinfo | grep processor | wc -l)"    
    # 服务器上内存不足，将线程数量减少一半
    CPU_NUM="$(expr $CPU_NUM / 2)"
else
    CPU_NUM="8"
fi

huey_consumer.py worker.huey -k thread -w ${CPU_NUM} --logfile=$WORKING_DIR/worker_log.log -n
