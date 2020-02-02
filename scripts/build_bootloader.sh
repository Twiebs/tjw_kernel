#!/bin/bash
SCRIPT_DIR=$(dirname $0)
BOOTLOADER_SOURCE_DIRECTORY=$SCRIPT_DIR/../code/bootloader
BINARY_OUTPUT_DIRECTORY=$SCRIPT_DIR/../intermediate/binaries

nasm -f bin $BOOTLOADER_SOURCE_DIRECTORY/bootloader.asm -o $BINARY_OUTPUT_DIRECTORY/bootloader.bin