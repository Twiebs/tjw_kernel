#!/bin/bash
source $(dirname $0)/variables.sh

# this script is mostly untested.

ISO_STAGING_DIRECTORY=$INTERMEDIATE_DIRECTORY/iso_staging
ISO_OUTPUT_DIRECTORY=$IMAGE_OUTPUT_DIRECTORY

# Copy the grub configuration and the kernel into the iso staging directory
mkdir -p $ISO_STAGING_DIRECTORY/boot/grub
cp $GRUB_CONFIG_FILE $ISO_STAGING_DIRECTORY/boot/grub
cp $BINARY_OUTPUT_DIRECTORY/kernel $ISO_STAGING_DIRECTORY/boot

grub-mkrescue -o $ISO_OUTPUT_DIRECTORY/kernel.iso $ISO_STAGING_DIRECTORY