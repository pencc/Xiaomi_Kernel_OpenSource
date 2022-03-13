#!/bin/bash
export OUT=${PWD}/out
export ARCH=arm64
export SUBARCH=arm64
#export DTC_EXT=dtc
export DTC_EXT=${PWD}/dtc-1.4.6/dtc
export CROSS_COMPILE=${PWD}/toolchains/aarch64-linux-android-4.9/bin/aarch64-linux-android-
export KERNEL_DEFCONFIG=lmi_user_defconfig
export PATH="/home/linx/Project/srcCode/camellia-r-oss/toolchains/clang-r383902/bin:$PATH"
export LD_LIBRARY_PATH="/home/linx/Project/srcCode/camellia-r-oss/toolchains/clang-r383902/lib:$LD_LIBRARY_PATH"
#set CONFIG_BUILD_ARM64_DT_OVERLAY=y
#set BUILD_CONFIG=build.config.gki.aarch64

ARCH=arm64 make CC=clang HOSTCC=gcc AR=aarch64-linux-android-ar NM=llvm-nm OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip O=out CLANG_TRIPLE=aarch64-linux-gnu-  LD=ld.lld $KERNEL_DEFCONFIG
ARCH=arm64 make -j8 CC=clang HOSTCC=gcc AR=aarch64-linux-android-ar NM=llvm-nm OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip O=out CLANG_TRIPLE=aarch64-linux-gnu- LD=ld.lld

#O=$OUT REAL_CC=${PWD}/toolchains/llvm-Snapdragon_LLVM_for_Android_8.0/prebuilt/linux-x86_64/bin/clang CLANG_TRIPLE=aarch64-linux-gnu- ${PWD}/scripts/gki/generate_defconfig.sh $KERNEL_DEFCONFIG
#make O=$OUT REAL_CC=${PWD}/toolchains/llvm-Snapdragon_LLVM_for_Android_8.0/prebuilt/linux-x86_64/bin/clang CLANG_TRIPLE=aarch64-linux-gnu- $KERNEL_DEFCONFIG
#cp kernel.release out/include/config/kernel.release
#cp compile.h out/include/generated/compile.h
#make  O=$OUT REAL_CC=${PWD}/toolchains/llvm-Snapdragon_LLVM_for_Android_8.0/prebuilt/linux-x86_64/bin/clang CLANG_TRIPLE=aarch64-linux-gnu- 
#make -j$(nproc) O=$OUT REAL_CC=${PWD}/toolchains/llvm-Snapdragon_LLVM_for_Android_8.0/prebuilt/linux-x86_64/bin/clang CLANG_TRIPLE=aarch64-linux-gnu- 
