#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
CHINO_ROOT_DIR=$SCRIPT_DIR/../../../../../..
DOWNLOAD_DIR=$CHINO_ROOT_DIR/download
OPENOCD_ARCHIVE=$DOWNLOAD_DIR/wch/openocd_osx-arm64.tar.zst
GCC_ARCHIVE=$DOWNLOAD_DIR/wch/xpack-riscv-none-elf-gcc-12.2.0-3_osx-arm64.tar.zst
mkdir $DOWNLOAD_DIR/wch &> /dev/null

# OpenOCD
if [ ! -f "$OPENOCD_ARCHIVE" ]; then
  wget https://dav.sunnycase.moe/d/ci/chino/$OPENOCD_ARCHIVE -O $OPENOCD_ARCHIVE
  tar --zstd -xf $OPENOCD_ARCHIVE -C $DOWNLOAD_DIR
fi

# GCC
if [ ! -f "$GCC_ARCHIVE" ]; then
  wget https://dav.sunnycase.moe/d/ci/chino/$GCC_ARCHIVE -O $GCC_ARCHIVE
  tar --zstd -xf $GCC_ARCHIVE -C $DOWNLOAD_DIR
fi
