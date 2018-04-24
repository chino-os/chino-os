#!/bin/bash
mkdir build
cd build
cmake .. -DARCH=x86_64 -DGNU_EFI_LIB=/usr/local/lib -DGNU_EFI_INC=/usr/local/include/efi
