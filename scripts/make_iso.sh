#!/bin/bash
#source variables.sh
# this script is mostly untested.
ISO_STAGING_DIRECTORY=../filesystem
ISO_OUTPUT_DIRECTORY=~/output
grub-mkrescue -o $ISO_OUTPUT_DIRECTORY/kernel.iso $ISO_STAGING_DIRECTORY