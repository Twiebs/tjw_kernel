#!/bin/bash
SCRIPT_DIR=$(dirname $0)

#Source code directories
CODE_DIRECTORY=$SCRIPT_DIR/../code
KERNEL_SOURCE_DIRECTORY=$CODE_DIRECTORY/kernel
TOOLS_SOURCE_DIRECTORY=$CODE_DIRECTORY/tools
COMMON_SOURCE_DIRECTORY=$CODE_DIRECTORY/common

#Intermediate directories
INTERMEDIATE_DIRECTORY=$SCRIPT_DIR/../intermediate
BINARY_OUTPUT_DIRECTORY=$INTERMEDIATE_DIRECTORY/binaries
IMAGE_OUTPUT_DIRECTORY=$INTERMEDIATE_DIRECTORY/images
LOG_OUTPUT_DIRECTORY=$INTERMEDIATE_DIRECTORY/log

#Make sure the intermediate directories exist
mkdir -p $BINARY_OUTPUT_DIRECTORY
mkdir -p $IMAGE_OUTPUT_DIRECTORY
mkdir -p $LOG_OUTPUT_DIRECTORY

#Miscellaneous configuration
MISCELLANEOUS_DIRECTORY=$SCRIPT_DIR/../miscellaneous
GRUB_CONFIG_FILE=$MISCELLANEOUS_DIRECTORY/grub.cfg

#QEMU configuration
QEMU_EXECUTABLE=qemu-system-x86_64.exe
QEMU_MEMORY_IN_MB=1024
QEMU_CPU_CORE_COUNT=1