#!/bin/bash
ARCH=$1
BOARD=$2
TOOLCHAIN=/opt/chino

case $ARCH in
x86_64)
	TUPLE0=x86_64
	;;
riscv64)
	TUPLE0=riscv64
	;;
cortex-m3)
	TUPLE0=arm
	;;
nios2)
	TUPLE0=nios2
	;;
*)
	echo "Unknown arch: $ARCH"
	exit 1
esac

cmake .. -DCMAKE_SYSTEM_NAME=Generic -DCMAKE_C_COMPILER=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-gcc -DCMAKE_CXX_COMPILER=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-g++ \
 -DCMAKE_LINKER=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-ld -DCMAKE_NM=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-nm -DCMAKE_OBJCOPY=/usr/bin/objcopy \
 -DCMAKE_ARCH_OBJCOPY=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-objcopy \
 -DCMAKE_OBJDUMP=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-objdump -DCMAKE_RANLIB=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-ranlib -DCMAKE_STRIP=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-strip \
 -DCMAKE_AR=$TOOLCHAIN/bin/$TUPLE0-unknown-chino-ar \
 -DARCH=$ARCH -DBOARD=$BOARD -DGNU_EFI_LIB=/usr/local/lib -DGNU_EFI_INC=/usr/local/include/efi
