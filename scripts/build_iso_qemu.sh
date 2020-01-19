#!/bin/bash
source $(dirname $0)/variables.sh

sh build.c
$SCRIPT_DIR/make_iso.sh
$SCRIPT_DIR/run_qemu_iso.sh
