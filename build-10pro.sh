#!/bin/bash
export OUT=${PWD}/out
export ARCH=arm64
export SUBARCH=arm64
#export DTC_EXT=dtc
export DTC_EXT=/home/linx/Project/srcCode/gauguin-r-oss/dtc-1.4.6/dtc
export CROSS_COMPILE=/home/linx/Project/srcCode/gauguin-r-oss/toolchains/aarch64-linux-android-4.9/bin/aarch64-linux-android-
export KERNEL_DEFCONFIG=cmi_user_defconfig
#set CONFIG_BUILD_ARM64_DT_OVERLAY=y
#set BUILD_CONFIG=build.config.gki.aarch64

make O=$OUT REAL_CC=/home/linx/Project/srcCode/gauguin-r-oss/toolchains/llvm-Snapdragon_LLVM_for_Android_8.0/prebuilt/linux-x86_64/bin/clang CLANG_TRIPLE=aarch64-linux-gnu- cmi_user_defconfig
make -j$(nproc) O=$OUT REAL_CC=/home/linx/Project/srcCode/gauguin-r-oss/toolchains/llvm-Snapdragon_LLVM_for_Android_8.0/prebuilt/linux-x86_64/bin/clang CLANG_TRIPLE=aarch64-linux-gnu- 
