#!/bin/bash
ARCH=$1
TOOLCHAIN=/opt/chino
cmake .. -DCMAKE_SYSTEM_NAME=Generic -DCMAKE_C_COMPILER=$TOOLCHAIN/bin/$ARCH-unknown-chino-gcc -DCMAKE_CXX_COMPILER=$TOOLCHAIN/bin/$ARCH-unknown-chino-g++ \
 -DCMAKE_LINKER=$TOOLCHAIN/bin/$ARCH-unknown-chino-ld -DCMAKE_NM=$TOOLCHAIN/bin/$ARCH-unknown-chino-nm -DCMAKE_OBJCOPY=/usr/bin/objcopy \
 -DCMAKE_OBJDUMP=$TOOLCHAIN/bin/$ARCH-unknown-chino-objdump -DCMAKE_RANLIB=$TOOLCHAIN/bin/$ARCH-unknown-chino-ranlib -DCMAKE_STRIP=$TOOLCHAIN/bin/$ARCH-unknown-chino-strip \
 -DCMAKE_AR=$TOOLCHAIN/bin/$ARCH-unknown-chino-ar \
 -DCMAKE_C_FLAGS="--specs=nosys.specs" \
 -DCMAKE_CXX_FLAGS="--specs=nosys.specs" \
 -DARCH=$ARCH -DGNU_EFI_LIB=/usr/local/lib -DGNU_EFI_INC=/usr/local/include/efi
