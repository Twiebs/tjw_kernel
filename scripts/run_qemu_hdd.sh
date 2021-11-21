#!/bin/bash
source $(dirname $0)/variables.sh

$QEMU_EXECUTABLE -boot d -hda $IMAGE_OUTPUT_DIRECTORY/hdd.img \
-smp $QEMU_CPU_CORE_COUNT -m $QEMU_MEMORY_IN_MB -serial file:$LOG_OUTPUT_DIRECTORY/serial.txt