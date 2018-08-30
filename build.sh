#!/bin/bash
ARCH=$1
BOARD=$2
TOOLCHAIN=/opt/chino

case $ARCH in
x86_64)
	TUPLE0=x86_64
	TUPLE1=unknown
	;;
riscv64)
	TUPLE0=riscv64
	TUPLE1=unknown
	;;
cortex-m3)
	TUPLE0=arm
	TUPLE1=unknown
	;;
nios2)
	TUPLE0=nios2
	TUPLE1=unknown
	;;
esp32)
	TUPLE0=xtensa
	TUPLE1=esp32
	;;
*)
	echo "Unknown arch: $ARCH"
	exit 1
esac

cmake .. -DCMAKE_SYSTEM_NAME=Generic -DCMAKE_C_COMPILER=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-gcc -DCMAKE_CXX_COMPILER=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-g++ \
 -DCMAKE_LINKER=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-ld -DCMAKE_NM=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-nm -DCMAKE_OBJCOPY=/usr/bin/objcopy \
 -DCMAKE_ARCH_OBJCOPY=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-objcopy \
 -DCMAKE_OBJDUMP=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-objdump -DCMAKE_RANLIB=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-ranlib -DCMAKE_STRIP=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-strip \
 -DCMAKE_AR=$TOOLCHAIN/bin/$TUPLE0-$TUPLE1-chino-ar \
 -DARCH=$ARCH -DBOARD=$BOARD -DGNU_EFI_LIB=/usr/local/lib -DGNU_EFI_INC=/usr/local/include/efi
