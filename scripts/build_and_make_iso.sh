#!/bin/bash
source $(dirname $0)/variables.sh

$SCRIPT_DIR/build_kernel.sh
$SCRIPT_DIR/make_iso.sh