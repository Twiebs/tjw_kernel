#!/bin/bash
source $(dirname $0)/variables.sh
gcc -I $COMMON_SOURCE_DIRECTORY $TOOLS_SOURCE_DIRECTORY/write_disk_image.c -o $BINARY_OUTPUT_DIRECTORY/write_disk_image