#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
CHINO_ROOT_DIR=$SCRIPT_DIR/../../../..
WCH_COMMON_DIR=$CHINO_ROOT_DIR/src/os/hal/chips/wch/common
$WCH_COMMON_DIR/prepare.sh
